/**
 * @file abstract_astar_delta.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar delta-stepping algorithm class header file
 */
#pragma once

#include "abstract_delta_stepping.hpp"

namespace SP
{
class AbstractAStarDeltaAlgo : public AbstractDeltaSteppingAlgo
{
  public:
    AbstractAStarDeltaAlgo(std::string graphFileName)
        : AbstractDeltaSteppingAlgo(graphFileName)
    {
    }
    virtual ~AbstractAStarDeltaAlgo() {}

  protected:
    virtual void resetInternalData() override;
    virtual double estimateCost(int vertex) override;
    virtual ReturnCode buildResult() override;
    virtual double heuristic(int vertex) = 0;

    // h(v) cached per query. Materialized in resetInternalData (after the
    // destination is set), used to compute reduced edge weights and to
    // restore the original distance in buildResult.
    std::vector<double> cachedH;
};
} // namespace SP
