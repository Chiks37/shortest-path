/**
 * @file graph_transpose.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Graph transposition header file
 */
#pragma once

#include "base.hpp"
#include <vector>

namespace SP
{
// Fills the vectors with the transpose of "graph", where every edge u -> v
// becomes v -> u with the same weight, and points "transposed" at them
void transposeGraph(const crsGraph &graph, std::vector<int> &xadj,
                    std::vector<int> &adjncy, std::vector<double> &eweights,
                    crsGraph &transposed);
} // namespace SP
