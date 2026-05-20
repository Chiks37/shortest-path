/**
 * @file astar_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar algorithm class source file
 */

#include "astar_bidir.hpp"

namespace SP
{

void AStarBiDirAlgo::resetInternalData()
{
    cachedHsrc.assign(graph.V, 0.0);
    cachedHdst.assign(graph.V, 0.0);
    for (int v = 0; v < graph.V; ++v)
    {
        cachedHsrc[v] = heuristic(v, this->source);
        cachedHdst[v] = heuristic(v, this->destination);
    }

    DijkstraBiDirAlgo::resetInternalData();
}

double AStarBiDirAlgo::estimateCost(int vertex)
{
    return distances[vertex] + 0.5 * (cachedHdst[vertex] - cachedHsrc[vertex]);
}

double AStarBiDirAlgo::estimateCostBackward(int vertex)
{
    return distancesBackward[vertex] +
           0.5 * (cachedHsrc[vertex] - cachedHdst[vertex]);
}

} // namespace SP
