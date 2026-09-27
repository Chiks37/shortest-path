/**
 * @file astarg_delta.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar with geometical heuristic delta-stepping algorithm class source
 * file
 */
#include "astarg_delta.hpp"

namespace SP
{

ReturnCode AStarGDeltaAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractAStarDeltaAlgo::preProcessImpl();
    return rc != ReturnCode::OK ? rc
                                : coordinates.load(nodesMappingFileName, graph);
}

double AStarGDeltaAlgo::heuristic(int vertex)
{
    return coordinates.lowerBound(vertex, destination);
}

} // namespace SP
