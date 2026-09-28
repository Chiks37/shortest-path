/**
 * @file coordinates.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Vertex coordinates for geometric heuristic source file
 */

#include "coordinates.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

namespace SP
{

ReturnCode Coordinates::load(const std::string &nodesMappingFileName,
                             const crsGraph &graph)
{
    ReturnCode rc = load(nodesMappingFileName, graph.V);
    if (rc != ReturnCode::OK)
    {
        return rc;
    }

    for (int u = 0; u < graph.V; ++u)
    {
        for (int i = graph.Xadj[u]; i < graph.Xadj[u + 1]; ++i)
        {
            fitEdge(u, graph.Adjncy[i], graph.Eweights[i]);
        }
    }

    return ReturnCode::OK;
}

ReturnCode Coordinates::load(const std::string &nodesMappingFileName,
                             int vertexCount)
{
    std::ifstream file(nodesMappingFileName);
    if (!file.is_open())
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    coordinates.clear();
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string filler;
        double x, y;
        if (iss >> filler >> filler >> filler >> filler >> x >> y)
        {
            coordinates.emplace_back(x, y);
        }
    }

    file.close();

    // Coordinates are matched to vertices by line order
    if (static_cast<int>(coordinates.size()) != vertexCount)
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    maxRatio = 0.0;
    scale = 0.0;
    return ReturnCode::OK;
}

// Weights and coordinates may be in different units. Dividing the straight
// line by the largest line-to-weight ratio over all edges makes every edge at
// least as long as its scaled line, so the heuristic never overestimates. A
// zero-weight edge between distinct points makes the ratio infinite and turns
// the heuristic off.
void Coordinates::fitEdge(int from, int to, double weight)
{
    double length = straightLine(from, to);
    if (length > 0.0)
    {
        maxRatio = std::max(maxRatio, length / weight);
        scale = 1.0 / maxRatio;
    }
}

double Coordinates::lowerBound(int from, int to) const
{
    return scale * straightLine(from, to);
}

double Coordinates::straightLine(int from, int to) const
{
    double dx = coordinates[from].first - coordinates[to].first;
    double dy = coordinates[from].second - coordinates[to].second;
    return std::hypot(dx, dy);
}

} // namespace SP
