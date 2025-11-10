/**
 * @file dijkstra.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class source file
 */

#include "astar.hpp"

namespace SP
{
double AStarAlgo::estimateCost(int vertex)
{
    return distances[vertex] + heuristic(vertex);
}
double AStarAlgo::heuristic(int vertex)
{
    return 0; // @FIXIT
}

} // namespace SP