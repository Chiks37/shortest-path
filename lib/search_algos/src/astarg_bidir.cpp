/**
 * @file astarg_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar with geometrical heuristic algorithm class source
 * file
 */
#include "astarg_bidir.hpp"

namespace SP
{

ReturnCode AStarGBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractAStarBiDirAlgo::preProcessImpl();
    return rc != ReturnCode::OK ? rc
                                : coordinates.load(nodesMappingFileName, graph);
}

double AStarGBiDirAlgo::heuristic(int vertex, int target)
{
    return coordinates.lowerBound(vertex, target);
}

} // namespace SP
