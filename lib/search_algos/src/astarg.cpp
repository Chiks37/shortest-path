/**
 * @file astarg.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar with geometical heuristic algorithm class source file
 */
#include "astarg.hpp"

namespace SP
{

ReturnCode AStarGAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractAStarAlgo::preProcessImpl();
    return rc != ReturnCode::OK ? rc
                                : coordinates.load(nodesMappingFileName, graph);
}

double AStarGAlgo::heuristic(int vertex)
{
    return coordinates.lowerBound(vertex, destination);
}

} // namespace SP
