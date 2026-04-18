/**
 * @file astar.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar algorithm class header file
 */
#pragma once

#include "dijkstra_seq.hpp"

namespace SP
{
class AStarAlgo : public DijkstraSeqAlgo
{
  public:
    AStarAlgo(std::string graphFileName) : DijkstraSeqAlgo(graphFileName) {}
    virtual ~AStarAlgo() {}

  protected:
    virtual double estimateCost(int vertex) override
    {
        return distances[vertex] + heuristic(vertex);
    }
    virtual double heuristic(int vertex) = 0;
};
} // namespace SP