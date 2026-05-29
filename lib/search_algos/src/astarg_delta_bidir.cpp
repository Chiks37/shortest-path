/**
 * @file astarg_delta_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar with geometrical heuristic delta-stepping
 * algorithm class source file
 */
#include "astarg_delta_bidir.hpp"
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

namespace SP
{

ReturnCode AStarGDeltaBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractAStarDeltaBiDirAlgo::preProcessImpl();
    if (rc != ReturnCode::OK)
    {
        return rc;
    }
    return loadNodeMapping();
}

double AStarGDeltaBiDirAlgo::heuristic(int vertex, int target)
{
    double dx =
        vertexCoordinates[vertex].first - vertexCoordinates[target].first;
    double dy =
        vertexCoordinates[vertex].second - vertexCoordinates[target].second;
    return std::hypot(dx, dy);
}

ReturnCode AStarGDeltaBiDirAlgo::loadNodeMapping()
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
