/**
 * @file astarg_delta_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar with geometrical heuristic delta-stepping
 * algorithm class source file
 */
#include "astarg_delta_bidir.hpp"

namespace SP
{

ReturnCode AStarGDeltaBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractAStarDeltaBiDirAlgo::preProcessImpl();
    return rc != ReturnCode::OK ? rc
                                : coordinates.load(nodesMappingFileName, graph);
}

double AStarGDeltaBiDirAlgo::heuristic(int vertex, int target)
{
    return coordinates.lowerBound(vertex, target);
}

} // namespace SP
