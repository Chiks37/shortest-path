/**
 * @file abstract_dijkstra_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra (bidirectional version) algorithm class source file
 */
#include "abstract_dijkstra_bidir.hpp"
#include "graph_transpose.hpp"
#include <omp.h>

namespace SP
{
void AbstractDijkstraBiDirAlgo::initInternalData()
{
    AbstractDijkstraSeqAlgo::initInternalData();

    distancesBackward.assign(graph.V, std::numeric_limits<double>::infinity());
    parentsBackward.assign(graph.V, -1);
}

void AbstractDijkstraBiDirAlgo::initQuery()
{
    AbstractDijkstraSeqAlgo::initQuery();

    distancesBackward[this->destination] = 0.0;
    double destEstimatedCost = estimateCostBackward(this->destination);
    pqBackward.push({this->destination, destEstimatedCost});

    meetingVertex = -1;
    shortestPathLength = std::numeric_limits<double>::infinity();

    // The meeting is detected on relaxation only, so a trivial query would
    // otherwise meet at a neighbor and report a non-zero distance.
    if (this->source == this->destination)
    {
        meetingVertex = this->source;
        shortestPathLength = 0.0;
    }

    forwardBound = estimateCost(this->source);
    backwardBound = destEstimatedCost;
}

void AbstractDijkstraBiDirAlgo::resetInternalData()
{
    // The half-searches do not record the vertices they label
    AbstractDijkstraSeqAlgo::resetInternalData();
    initInternalData();
    pqBackward = std::priority_queue<edge, std::vector<edge>, compareEdges>();
}

ReturnCode AbstractDijkstraBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractDijkstraSeqAlgo::preProcessImpl();
    if (ReturnCode::OK != rc)
    {
        return rc;
    }

    distancesBackward.resize(graph.V);
    parentsBackward.resize(graph.V);

    buildReverseGraph();

    return rc;
}

void AbstractDijkstraBiDirAlgo::buildReverseGraph()
{
    transposeGraph(graph, reverseXadj, reverseAdjncy, reverseEweights,
                   reverseGraph);
}

ReturnCode AbstractDijkstraBiDirAlgo::computeImpl()
{
    initQuery();

#pragma omp parallel
    {
#pragma omp single
        {
#pragma omp task shared(pq, distances, parents, distancesBackward,             \
                        forwardBound, backwardBound)
            {
                runHalfSearch(graph, pq, distances, parents, distancesBackward,
                              forwardBound, backwardBound,
                              &AbstractDijkstraBiDirAlgo::estimateCost);
            }

#pragma omp task shared(pqBackward, distancesBackward, parentsBackward,        \
                        distances, forwardBound, backwardBound)
            {
                runHalfSearch(reverseGraph, pqBackward, distancesBackward,
                              parentsBackward, distances, backwardBound,
                              forwardBound,
                              &AbstractDijkstraBiDirAlgo::estimateCostBackward);
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
    resetInternalData();

    return ReturnCode::OK;
}

ReturnCode AbstractDijkstraBiDirAlgo::buildResult()
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

ReturnCode AbstractDijkstraBiDirAlgo::runHalfSearch(
    const crsGraph &searchGraph,
    std::priority_queue<edge, std::vector<edge>, compareEdges> &myPq,
    std::vector<double> &myDistances, std::vector<int> &myParents,
    std::vector<double> &otherDistances, std::atomic<double> &myBound,
    const std::atomic<double> &otherBound,
    AbstractDijkstraBiDirAlgo::CostEstimator costEstimator)
{
    while (!myPq.empty())
    {

        // Getting current vertex info from priority queue
        int currentVertex = myPq.top().vertex;
        double curVerPoppedEstCost = myPq.top().val;
        myPq.pop();

        // Stop condition: every vertex this half has not scanned yet costs at
        // least the popped key, every vertex the other half has not scanned
        // yet costs at least its bound, so no shorter path is left.
        myBound = curVerPoppedEstCost;
        if (curVerPoppedEstCost + otherBound >= shortestPathLength)
        {
            break;
        }

        // Skip if there is already shorter path to current vertex than we
        // trying to calculate
        double curVerStoredEstCost = (this->*costEstimator)(currentVertex);
        if (curVerPoppedEstCost > curVerStoredEstCost)
            continue;

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
                // The other half reads this label concurrently. Both halves
                // store their own label before loading the other one with
                // sequentially consistent atomics, so at least one of them
                // sees the meeting at this vertex.
                std::atomic_ref<double>(myDistances[neighborVertex])
                    .store(neighbVerNewDistance);
                myParents[neighborVertex] = currentVertex;
                double neigbourEstimatedCost =
                    (this->*costEstimator)(neighborVertex);
                myPq.push({neighborVertex, neigbourEstimatedCost});

                double otherDistance =
                    std::atomic_ref<double>(otherDistances[neighborVertex])
                        .load();
                if (otherDistance != std::numeric_limits<double>::infinity())
                {
                    double potentialPath = neighbVerNewDistance + otherDistance;
                    if (potentialPath < shortestPathLength)
                    {
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
    }

    // This half can not improve the path anymore, so the other half must not
    // wait for its bound to grow
    myBound = std::numeric_limits<double>::infinity();

    return ReturnCode::OK;
}

std::vector<int>
AbstractDijkstraBiDirAlgo::reconstructPath(int source, int destination,
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