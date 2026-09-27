/**
 * @file landmarks.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Landmarks distance tables for ALT heuristic header file
 */
#pragma once

#include "base.hpp"
#include <cstddef>
#include <vector>

namespace SP
{
// Distances between the landmarks and every vertex, shared by all ALT
// algorithms. The lower bound reads all landmarks of one vertex at once, so the
// tables are vertex-major. For a symmetric graph the distances to and from a
// landmark are equal and only one table is kept.
class Landmarks
{
  public:
    ReturnCode build(const crsGraph &graph, int landmarksCount = 16);

    // Lower bound on the distance from vertex "from" to vertex "to"
    double lowerBound(int from, int to) const;

  private:
    static void computeDistances(const crsGraph &graph, int source,
                                 std::vector<double> &distances);
    static int farthestVertex(const std::vector<double> &distances,
                              int fallback);
    const double *row(const std::vector<double> &table, int vertex) const
    {
        return &table[static_cast<std::size_t>(vertex) * stride];
    }

    std::vector<int> landmarks;
    std::size_t stride{0};
    bool symmetric{true};
    // distFromLandmarks[v * stride + i] = dist(landmarks[i], v)
    std::vector<double> distFromLandmarks;
    // distToLandmarks[v * stride + i] = dist(v, landmarks[i]), directed only
    std::vector<double> distToLandmarks;
};
} // namespace SP
