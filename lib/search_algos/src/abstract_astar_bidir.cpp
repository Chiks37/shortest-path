/**
 * @file abstract_astar_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar algorithm class source file
 */

#include "abstract_astar_bidir.hpp"

namespace SP
{

void AbstractAStarBiDirAlgo::resetInternalData()
{
    cachedHsrc.assign(graph.V, 0.0);
    cachedHdst.assign(graph.V, 0.0);
    for (int v = 0; v < graph.V; ++v)
    {
        cachedHsrc[v] = heuristic(v, this->source);
        cachedHdst[v] = heuristic(v, this->destination);
    }

    AbstractDijkstraBiDirAlgo::resetInternalData();
}

double AbstractAStarBiDirAlgo::estimateCost(int vertex)
{
    return distances[vertex] + 0.5 * (cachedHdst[vertex] - cachedHsrc[vertex]);
}

double AbstractAStarBiDirAlgo::estimateCostBackward(int vertex)
{
    return distancesBackward[vertex] +
           0.5 * (cachedHsrc[vertex] - cachedHdst[vertex]);
}

} // namespace SP
