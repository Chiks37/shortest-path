/**
 * @file dijkstra.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class source file
 */

#include "dijkstra.hpp"
#include <algorithm>
#include <limits>

namespace SP
{
ReturnCode DijkstraAlgo::setSrcDest(int source, int destination)
{
    ReturnCode rc = BaseAlgo::setSrcDest(source, destination);
    if (rc != ReturnCode::OK)
    {
        return rc;
    }

    resetInternalData();

    return rc;
}

ReturnCode DijkstraAlgo::preProcessImpl()
{
    ReturnCode rc = BaseAlgo::preProcessImpl();
    if (ReturnCode::OK != rc)
    {
        return rc;
    }

    distances.resize(graph.V);
    parents.resize(graph.V);

    return rc;
}

ReturnCode DijkstraAlgo::computeImpl()
{
    ReturnCode rc = runSearch();
    rc = rc != ReturnCode::OK ? rc : buildResult();
    return rc;
}

void DijkstraAlgo::initInternalData()
{
    distances.assign(graph.V, std::numeric_limits<double>::infinity());
    distances[this->source] = 0.0;

    parents.assign(graph.V, -1);

    // Clear the pq
    pq = std::priority_queue<edge, std::vector<edge>, compareEdges>();
}

void DijkstraAlgo::resetInternalData()
{
    initInternalData();

    double sourceEstimatedCost = estimateCost(this->source);
    pq.push({this->source, sourceEstimatedCost});
}

ReturnCode DijkstraAlgo::runSearch()
{
    while (!pq.empty())
    {

        // Getting current vertex info from priority queue
        int currentVertex = pq.top().vertex;
        double curVerPoppedEstCost = pq.top().val;
        pq.pop();

        // Skip if there is already shorter path to current vertex than we
        // trying to calculate
        double curVerStoredEstCost = estimateCost(currentVertex);
        if (curVerPoppedEstCost > curVerStoredEstCost)
            continue;

        // Early exit condition: we already reached destination vertex
        if (completeCondition(currentVertex))
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
                distances[currentVertex] + neighborVertexWeight;
            if (distances[neighborVertex] > neighbVerNewDistance)
            {
                distances[neighborVertex] = neighbVerNewDistance;
                parents[neighborVertex] = currentVertex;
                double neigbourEstimatedCost = estimateCost(neighborVertex);
                pq.push({neighborVertex, neigbourEstimatedCost});
            }
        }
    }

    return ReturnCode::OK;
}

ReturnCode DijkstraAlgo::buildResult()
{
    ReturnCode rc = ReturnCode::OK;

    outData.shortestDistance = getDistance(this->destination);
    if (std::numeric_limits<double>::infinity() == outData.shortestDistance)
    {
        return rc;
    }

    outData.shortestPath = reconstructPath(this->destination);

    return rc;
}

double DijkstraAlgo::getDistance(int vertex)
{
    if (vertex < 0 || vertex >= graph.V)
    {
        return -1.0;
    }
    return distances[vertex];
}

std::vector<int> DijkstraAlgo::reconstructPath(int destination)
{
    int currentVertex = destination;
    std::vector<int> path;
    path.push_back(currentVertex);

    // Collecting optimal path
    while (currentVertex != this->source)
    {
        int parrentVertex = parents[currentVertex];
        path.push_back(parrentVertex);
        currentVertex = parrentVertex;
    }

    // Reversing path to get correct order
    std::reverse(path.begin(), path.end());

    return path;
}

bool DijkstraAlgo::completeCondition(int currentVertex)
{
    return currentVertex == this->destination;
}

} // namespace SP
