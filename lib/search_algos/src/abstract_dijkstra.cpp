/**
 * @file abstract_dijkstra.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class source file
 */

#include "abstract_dijkstra.hpp"
#include <algorithm>
#include <limits>

namespace SP
{
ReturnCode AbstractDijkstraAlgo::preProcessImpl()
{
    ReturnCode rc = BaseAlgo::preProcessImpl();
    if (ReturnCode::OK != rc)
    {
        return rc;
    }

    initInternalData();

    return rc;
}

ReturnCode AbstractDijkstraAlgo::computeImpl()
{
    initQuery();
    ReturnCode rc = runSearch();
    rc = rc != ReturnCode::OK ? rc : buildResult();
    resetInternalData();
    return rc;
}

void AbstractDijkstraAlgo::initInternalData()
{
    distances.assign(graph.V, std::numeric_limits<double>::infinity());
    parents.assign(graph.V, -1);
    touched.clear();
}

void AbstractDijkstraAlgo::initQuery() { updateLabel(this->source, 0.0, -1); }

void AbstractDijkstraAlgo::resetInternalData()
{
    for (int vertex : touched)
    {
        distances[vertex] = std::numeric_limits<double>::infinity();
        parents[vertex] = -1;
    }
    touched.clear();
}

ReturnCode AbstractDijkstraAlgo::buildResult()
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

double AbstractDijkstraAlgo::getDistance(int vertex)
{
    if (vertex < 0 || vertex >= graph.V)
    {
        return -1.0;
    }
    return distances[vertex];
}

std::vector<int> AbstractDijkstraAlgo::reconstructPath(int destination)
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

bool AbstractDijkstraAlgo::completeCondition(int currentVertex)
{
    return currentVertex == this->destination;
}

} // namespace SP
