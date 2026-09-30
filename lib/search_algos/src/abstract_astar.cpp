/**
 * @file abstract_astar.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar algorithm class source file
 */

#include "abstract_astar.hpp"

namespace SP
{
void AbstractAStarAlgo::initInternalData()
{
    AbstractDijkstraSeqAlgo::initInternalData();

    heuristics.resize(graph.V);
}

void AbstractAStarAlgo::initQuery()
{
    heuristics.nextQuery();

    AbstractDijkstraSeqAlgo::initQuery();
}
} // namespace SP