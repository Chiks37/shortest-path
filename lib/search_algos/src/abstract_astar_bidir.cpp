/**
 * @file abstract_astar_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar algorithm class source file
 */

#include "abstract_astar_bidir.hpp"

namespace SP
{

void AbstractAStarBiDirAlgo::initInternalData()
{
    AbstractDijkstraBiDirAlgo::initInternalData();

    potentials.resize(graph.V);
}

void AbstractAStarBiDirAlgo::initQuery()
{
    potentials.nextQuery();

    AbstractDijkstraBiDirAlgo::initQuery();
}

} // namespace SP
