/**
 * @file astarg.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar with geometical heuristic algorithm class source file
 */
#include "astarg.hpp"
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

namespace SP
{

ReturnCode AStarGAlgo::preProcessImpl()
{
    ReturnCode rc = AStarAlgo::preProcessImpl();
    if (rc != ReturnCode::OK)
    {
        return rc;
    }
    return loadNodeMapping();
}

double AStarGAlgo::heuristic(int vertex)
{
    double dx =
        vertexCoordinates[vertex].first - vertexCoordinates[destination].first;
    double dy = vertexCoordinates[vertex].second -
                vertexCoordinates[destination].second;
    return std::hypot(dx, dy);
}

ReturnCode AStarGAlgo::loadNodeMapping()
{
    std::ifstream file(nodesMappingFileName);
    if (!file.is_open())
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string filler;
        double x, y;
        if (iss >> filler >> filler >> filler >> filler >> x >> y)
        {
            vertexCoordinates.emplace_back(x, y);
        }
    }

    file.close();

    return ReturnCode::OK;
}

} // namespace SP
