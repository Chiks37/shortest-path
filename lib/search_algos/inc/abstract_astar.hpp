/**
 * @file abstract_astar.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar algorithm class header file
 */
#pragma once

#include "abstract_dijkstra_seq.hpp"

namespace SP
{
class AbstractAStarAlgo : public AbstractDijkstraSeqAlgo
{
  public:
    AbstractAStarAlgo(std::string graphFileName)
        : AbstractDijkstraSeqAlgo(graphFileName)
    {
    }
    virtual ~AbstractAStarAlgo() {}

  protected:
    virtual double estimateCost(int vertex) override
    {
        return distances[vertex] + heuristic(vertex);
    }
    virtual double heuristic(int vertex) = 0;
};
} // namespace SP