/**
 * @file astar_delta.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar delta-stepping algorithm class header file
 */
#pragma once

#include "delta_stepping.hpp"

namespace SP
{
class AStarDeltaAlgo : public DeltaSteppingAlgo
{
  public:
    AStarDeltaAlgo(std::string graphFileName) : DeltaSteppingAlgo(graphFileName)
    {
    }
    virtual ~AStarDeltaAlgo() {}

  protected:
    virtual double estimateCost(int vertex) override
    {
        return distances[vertex] + heuristic(vertex);
    }
    virtual double heuristic(int vertex) = 0;
};
} // namespace SP
