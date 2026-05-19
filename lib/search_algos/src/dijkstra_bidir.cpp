/**
 * @file dijkstra_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra (bidirectional version) algorithm class source file
 */
#include "dijkstra_bidir.hpp"
#include <omp.h>

namespace SP
{
void DijkstraBiDirAlgo::initInternalData()
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

void DijkstraBiDirAlgo::resetInternalData()
{
    DijkstraSeqAlgo::resetInternalData();

    double destEstimatedCost = estimateCostBackward(this->destination);
    pqBackward.push({this->destination, destEstimatedCost});
}
ReturnCode DijkstraBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = DijkstraSeqAlgo::preProcessImpl();
    if (ReturnCode::OK != rc)
    {
        return rc;
    }

    distancesBackward.resize(graph.V);
    parentsBackward.resize(graph.V);

    buildReverseGraph();

    return rc;
}

void DijkstraBiDirAlgo::buildReverseGraph()
{
    int V = graph.V;
    int nz = graph.Xadj[V];

    reverseXadj.assign(V + 1, 0);
    reverseAdjncy.resize(nz);
    reverseEweights.resize(nz);

    for (int i = 0; i < nz; i++)
    {
        reverseXadj[graph.Adjncy[i] + 1]++;
    }
    for (int v = 0; v < V; v++)
    {
        reverseXadj[v + 1] += reverseXadj[v];
    }

    std::vector<int> cursor(reverseXadj.begin(), reverseXadj.end() - 1);
    for (int u = 0; u < V; u++)
    {
        for (int i = graph.Xadj[u]; i < graph.Xadj[u + 1]; i++)
        {
            int v = graph.Adjncy[i];
            int pos = cursor[v]++;
            reverseAdjncy[pos] = u;
            reverseEweights[pos] = graph.Eweights[i];
        }
    }

    reverseGraph.Xadj = reverseXadj.data();
    reverseGraph.Adjncy = reverseAdjncy.data();
    reverseGraph.Eweights = reverseEweights.data();
    reverseGraph.V = V;
    reverseGraph.nz = nz;
}

ReturnCode DijkstraBiDirAlgo::computeImpl()
{
#pragma omp parallel
    {
#pragma omp single
        {
#pragma omp task shared(pq, distances, parents, distancesBackward)
            {
                runHalfSearch(graph, pq, distances, parents, distancesBackward,
                              &DijkstraBiDirAlgo::estimateCost);
            }

#pragma omp task shared(pqBackward, distancesBackward, parentsBackward,        \
                        distances)
            {
                runHalfSearch(reverseGraph, pqBackward, distancesBackward,
                              parentsBackward, distances,
                              &DijkstraBiDirAlgo::estimateCostBackward);
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
                    shortestPathBackward = reconstructPath(
                        destination, meetingVertex, parentsBackward);
                }
#pragma omp taskwait
            }
        }
    }

    buildResult();

    return ReturnCode::OK;
}

ReturnCode DijkstraBiDirAlgo::buildResult()
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

ReturnCode DijkstraBiDirAlgo::runHalfSearch(
    const crsGraph &searchGraph,
    std::priority_queue<edge, std::vector<edge>, compareEdges> &myPq,
    std::vector<double> &myDistances, std::vector<int> &myParents,
    const std::vector<double> &otherDistances,
    DijkstraBiDirAlgo::CostEstimator costEstimator)
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

        // Early exit condition: we already reached destination vertex.
        // shortestPathLength is shared with the other half-search, so it
        // must be snapshotted under the same lock that guards its updates.
        double bestSoFar;
#pragma omp critical(bidirMeeting)
        {
            bestSoFar = shortestPathLength;
        }
        if (curVerPoppedEstCost >= bestSoFar)
        {
            break;
        }

        // Researching neighbors
        for (int i = searchGraph.Xadj[currentVertex];
             i < searchGraph.Xadj[currentVertex + 1]; i++)
        {
            int neighborVertex = searchGraph.Adjncy[i];
            double neighborVertexWeight = searchGraph.Eweights[i];

            // Updating values
            double neighbVerNewDistance =
                myDistances[currentVertex] + neighborVertexWeight;
            if (myDistances[neighborVertex] > neighbVerNewDistance)
            {
                myDistances[neighborVertex] = neighbVerNewDistance;
                myParents[neighborVertex] = currentVertex;
                double neigbourEstimatedCost =
                    (this->*costEstimator)(neighborVertex);
                myPq.push({neighborVertex, neigbourEstimatedCost});

                // Cross-check: check if the other search has reached this
                // vertex. Snapshot the value written by the other half-search
                // once so the guard and the sum stay consistent.
                double otherDistance = otherDistances[neighborVertex];
                if (otherDistance != std::numeric_limits<double>::infinity())
                {
                    double potentialPath =
                        myDistances[neighborVertex] + otherDistance;
#pragma omp critical(bidirMeeting)
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

std::vector<int>
DijkstraBiDirAlgo::reconstructPath(int source, int destination,
                                   const std::vector<int> &myParents)
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
} // namespace SP