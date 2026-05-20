/**
 * @file astar_delta_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar delta-stepping algorithm class header file
 */
#pragma once

#include "delta_stepping_bidir.hpp"

namespace SP
{
class AStarDeltaBiDirAlgo : public DeltaSteppingBiDirAlgo
{
  public:
    AStarDeltaBiDirAlgo(std::string graphFileName)
        : DeltaSteppingBiDirAlgo(graphFileName)
    {
    }
    virtual ~AStarDeltaBiDirAlgo() {}

  protected:
    virtual void resetInternalData() override;
    virtual ReturnCode buildResult() override;
    virtual double heuristic(int vertex, int target) = 0;

    // h(v, source) and h(v, destination) cached per query. The symmetric
    // potential pi_f(v) = (cachedHdst[v] - cachedHsrc[v]) / 2 drives the
    // Johnson reweighting used to feed both directions into a single
    // delta-stepping back-end (with pi_b = -pi_f). The cache also lets us
    // restore the real distance from the reduced one in buildResult.
    std::vector<double> cachedHsrc;
    std::vector<double> cachedHdst;
};
} // namespace SP
