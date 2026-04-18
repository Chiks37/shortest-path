/**
 * @file dijkstra_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra (bidirectional version) algorithm class source file
 */
#include "dijkstra_bidir.hpp"
#include <omp.h>

namespace SP
{
void dijkstraBiDirAlgo::initInternalData()
{
    DijkstraSeqAlgo::initInternalData();

    distancesBackward.assign(graph.V, std::numeric_limits<double>::infinity());
    distancesBackward[this->destination] = 0.0;

    parentsBackward.assign(graph.V, -1);

    // Clear the pq
    pqBackward = std::priority_queue<edge, std::vector<edge>, compareEdges>();

    meetingVertex = -1;
    shortestPathLength = std::numeric_limits<double>::infinity();
}

void dijkstraBiDirAlgo::resetInternalData(){
    DijkstraSeqAlgo::resetInternalData();

    double destEstimatedCost = estimateCostBackward(this->destination);
    pqBackward.push({this->destination, destEstimatedCost});
}
ReturnCode dijkstraBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = DijkstraSeqAlgo::preProcessImpl();
    if (ReturnCode::OK != rc)
    {
        return rc;
    }

    distancesBackward.resize(graph.V);
    parentsBackward.resize(graph.V);

    return rc;
}

ReturnCode dijkstraBiDirAlgo::computeImpl()
{
#pragma omp parallel
    {
#pragma omp single
        {
#pragma omp task shared(pq, distances, parents, distancesBackward)
            {
                runHalfSearch(pq, distances, parents, distancesBackward, &dijkstraBiDirAlgo::estimateCost);
            }

#pragma omp task shared(pqBackward, distancesBackward, parentsBackward,        \
                        distances)
            {
                runHalfSearch(pqBackward, distancesBackward, parentsBackward,
                              distances, &dijkstraBiDirAlgo::estimateCostBackward);
            }

#pragma omp taskwait
            if (meetingVertex != -1)
            {
#pragma omp task shared(outData)
                {
                    outData.shortestPath =
                        reconstructPath(source, meetingVertex, parents);
                }
#pragma omp task shared(shortestPathBackward)
                {
                    shortestPathBackward =
                        reconstructPath(destination, meetingVertex, parentsBackward);
                }
#pragma omp taskwait
            }
        }
    }

    buildResult();

    return ReturnCode::OK;
}

ReturnCode dijkstraBiDirAlgo::buildResult()
{
    if (meetingVertex != -1)
    {
        std::reverse(outData.shortestPath.begin(), outData.shortestPath.end());
        outData.shortestPath.insert(outData.shortestPath.end(),
                                    shortestPathBackward.begin() + 1,
                                    shortestPathBackward.end());
        outData.shortestDistance = shortestPathLength;
    }
    else
    {
        outData.shortestDistance = std::numeric_limits<double>::infinity();
    }

    return ReturnCode::OK;
}

ReturnCode dijkstraBiDirAlgo::runHalfSearch(
    std::priority_queue<edge, std::vector<edge>, compareEdges> &myPq,
    std::vector<double> &myDistances, std::vector<int> &myParents,
    const std::vector<double> &otherDistances,
    dijkstraBiDirAlgo::CostEstimator costEstimator)
{
    while (!myPq.empty())
    {

        // Getting current vertex info from priority queue
        int currentVertex = myPq.top().vertex;
        double curVerPoppedEstCost = myPq.top().val;
        myPq.pop();
        
        // Skip if there is already shorter path to current vertex than we
        // trying to calculate
        double curVerStoredEstCost = (this->*costEstimator)(currentVertex);
        if (curVerPoppedEstCost > curVerStoredEstCost)
            continue;

        // Early exit condition: we already reached destination vertex
        if (curVerPoppedEstCost >= shortestPathLength)
        {
            break;
        }

        // Researching neighbors
        for (int i = graph.Xadj[currentVertex];
             i < graph.Xadj[currentVertex + 1]; i++)
        {
            int neighborVertex = graph.Adjncy[i];
            double neighborVertexWeight = graph.Eweights[i];

            // Updating values
            double neighbVerNewDistance =
                myDistances[currentVertex] + neighborVertexWeight;
            if (myDistances[neighborVertex] > neighbVerNewDistance)
            {
                myDistances[neighborVertex] = neighbVerNewDistance;
                myParents[neighborVertex] = currentVertex;
                double neigbourEstimatedCost = (this->*costEstimator)(neighborVertex);
                myPq.push({neighborVertex, neigbourEstimatedCost});

                // Cross-check: check if the other search has reached this vertex
                if (otherDistances[neighborVertex] != std::numeric_limits<double>::infinity())
                {
                    double potentialPath = myDistances[neighborVertex] + otherDistances[neighborVertex];
#pragma omp critical
                    {
                        if (potentialPath < shortestPathLength)
                        {
                            shortestPathLength = potentialPath;
                            meetingVertex = neighborVertex;
                        }
                    }
                }
            }
        }
    }

    return ReturnCode::OK;
}

std::vector<int> dijkstraBiDirAlgo::reconstructPath(int source,
                                                        int destination, const std::vector<int> &myParents)
{
    int currentVertex = destination;
    std::vector<int> path;
    path.push_back(currentVertex);

    // Collecting optimal path
    while (currentVertex != source)
    {
        int parrentVertex = myParents[currentVertex];
        path.push_back(parrentVertex);
        currentVertex = parrentVertex;
    }

    return path;
}
}