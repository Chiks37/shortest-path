/**
 * @file abstract_dijkstra_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra (bidirectional version) algorithm class source file
 */
#include "abstract_dijkstra_bidir.hpp"
#include "graph_transpose.hpp"
#include <algorithm>
#include <cmath>

namespace SP
{
void AbstractDijkstraBiDirAlgo::initInternalData()
{
    AbstractDijkstraSeqAlgo::initInternalData();

    distancesBackward.assign(graph.V, std::numeric_limits<double>::infinity());
    parentsBackward.assign(graph.V, -1);
    touchedBackward.clear();
}

void AbstractDijkstraBiDirAlgo::initQuery()
{
    AbstractDijkstraSeqAlgo::initQuery();

    distancesBackward[this->destination] = 0.0;
    touchedBackward.push_back(this->destination);
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
}

void AbstractDijkstraBiDirAlgo::resetInternalData()
{
    AbstractDijkstraSeqAlgo::resetInternalData();

    for (int vertex : touchedBackward)
    {
        distancesBackward[vertex] = std::numeric_limits<double>::infinity();
        parentsBackward[vertex] = -1;
    }
    touchedBackward.clear();
    pqBackward.clear();
}

ReturnCode AbstractDijkstraBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractDijkstraSeqAlgo::preProcessImpl();
    if (ReturnCode::OK != rc)
    {
        return rc;
    }

    buildReverseGraph();

    return rc;
}

void AbstractDijkstraBiDirAlgo::buildReverseGraph()
{
    transposeGraph(graph, reverseXadj, reverseAdjncy, reverseEweights,
                   reverseGraph);
}

ReturnCode AbstractDijkstraBiDirAlgo::runSearch()
{
    // Every vertex a direction has not scanned yet costs at least the top of
    // its queue, so no path shorter than the sum of the two tops is left
    bool forward = true;
    while (!pq.empty() && !pqBackward.empty() &&
           pq.top().val + pqBackward.top().val < shortestPathLength)
    {
        scanNextVertex(forward);
        forward = !forward;
    }

    return ReturnCode::OK;
}

void AbstractDijkstraBiDirAlgo::scanNextVertex(bool forward)
{
    const crsGraph &searchGraph = forward ? graph : reverseGraph;
    auto &myPq = forward ? pq : pqBackward;
    auto &myDistances = forward ? distances : distancesBackward;
    auto &myParents = forward ? parents : parentsBackward;
    auto &myTouched = forward ? touched : touchedBackward;
    const auto &otherDistances = forward ? distancesBackward : distances;

    // Getting current vertex info from priority queue
    int currentVertex = myPq.top().vertex;
    double curVerPoppedEstCost = myPq.top().val;
    myPq.pop();

    // Skip if there is already shorter path to current vertex than we
    // trying to calculate
    double curVerStoredEstCost = forward ? estimateCost(currentVertex)
                                         : estimateCostBackward(currentVertex);
    if (curVerPoppedEstCost > curVerStoredEstCost)
    {
        return;
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
            if (std::isinf(myDistances[neighborVertex]))
            {
                myTouched.push_back(neighborVertex);
            }
            myDistances[neighborVertex] = neighbVerNewDistance;
            myParents[neighborVertex] = currentVertex;
            double neigbourEstimatedCost =
                forward ? estimateCost(neighborVertex)
                        : estimateCostBackward(neighborVertex);
            myPq.push({neighborVertex, neigbourEstimatedCost});

            double potentialPath =
                neighbVerNewDistance + otherDistances[neighborVertex];
            if (potentialPath < shortestPathLength)
            {
                shortestPathLength = potentialPath;
                meetingVertex = neighborVertex;
            }
        }
    }
}

ReturnCode AbstractDijkstraBiDirAlgo::buildResult()
{
    if (meetingVertex == -1)
    {
        outData.shortestDistance = std::numeric_limits<double>::infinity();
        return ReturnCode::OK;
    }

    outData.shortestPath = reconstructPath(source, meetingVertex, parents);
    std::reverse(outData.shortestPath.begin(), outData.shortestPath.end());
    std::vector<int> backwardPath =
        reconstructPath(destination, meetingVertex, parentsBackward);
    outData.shortestPath.insert(outData.shortestPath.end(),
                                backwardPath.begin() + 1, backwardPath.end());
    outData.shortestDistance = shortestPathLength;

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
