/**
 * @file abstract_astar_delta.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar delta-stepping algorithm class header file
 */
#pragma once

#include "abstract_delta_stepping.hpp"

namespace SP
{
// Delta-stepping on the reduced weights w(u, v) + h(v) - h(u), computed on
// the fly. With a consistent heuristic they are non-negative and preserve
// shortest paths, which makes the search goal-directed like A*.
class AbstractAStarDeltaAlgo : public AbstractDeltaSteppingAlgo
{
  public:
    AbstractAStarDeltaAlgo(std::string graphFileName)
        : AbstractDeltaSteppingAlgo(graphFileName)
    {
        usePotential = true;
    }
    virtual ~AbstractAStarDeltaAlgo() {}

  protected:
    virtual ReturnCode buildResult() override;
    virtual double computePotential(int vertex) override
    {
        return heuristic(vertex);
    }
    virtual double heuristic(int vertex) = 0;
};
} // namespace SP
