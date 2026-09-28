/**
 * @file coordinates.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Vertex coordinates for geometric heuristic header file
 */
#pragma once

#include "base.hpp"
#include <string>
#include <utility>
#include <vector>

namespace SP
{
// Vertex coordinates shared by all A*G algorithms. The straight-line distance
// between two vertices is scaled to the units of the edge weights.
class Coordinates
{
  public:
    ReturnCode load(const std::string &nodesMappingFileName,
                    const crsGraph &graph);
    // For a graph stored in another structure: reads the coordinates only,
    // then every edge of the graph has to be passed to fitEdge
    ReturnCode load(const std::string &nodesMappingFileName, int vertexCount);
    void fitEdge(int from, int to, double weight);

    // Lower bound on the distance between vertices "from" and "to"
    double lowerBound(int from, int to) const;

  private:
    double straightLine(int from, int to) const;

    std::vector<std::pair<double, double>> coordinates;
    double maxRatio{0.0};
    double scale{1.0};
};
} // namespace SP
