/**
 * @file general.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Header file for general entities
 */
#pragma once

extern "C"
{
#include "graphio.h"
}
#include <queue>
#include <vector>

namespace SP
{
// Edge comparator for setting priority queues
struct compareEdges
{
    bool operator()(const edge &e1, const edge &e2) const
    {
        return e1.val > e2.val;
    }
};

// Binary heap of edges that keeps its storage when cleared, so a query does not
// grow it again after the previous one
class EdgeQueue
    : public std::priority_queue<edge, std::vector<edge>, compareEdges>
{
  public:
    void clear() { c.clear(); }
};
} // namespace SP
