/**
 * @file abstract_astar_delta_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar delta-stepping algorithm class header file
 */
#pragma once

#include "abstract_delta_stepping_bidir.hpp"

namespace SP
{
// Bidirectional delta-stepping on reduced weights. The forward direction uses
// the potential p(v) = (h(v, destination) - h(source, v)) / 2 and the backward
// one its negation, so both work on the same reduced weights.
class AbstractAStarDeltaBiDirAlgo : public AbstractDeltaSteppingBiDirAlgo
{
  public:
    AbstractAStarDeltaBiDirAlgo(std::string graphFileName)
        : AbstractDeltaSteppingBiDirAlgo(graphFileName)
    {
        usePotential = true;
    }
    virtual ~AbstractAStarDeltaBiDirAlgo() {}

  protected:
    virtual ReturnCode buildResult() override;
    virtual double computePotential(int vertex) override
    {
        return 0.5 * (heuristic(vertex, this->destination) -
                      heuristic(this->source, vertex));
    }
    // Lower bound on the distance from vertex to target (order matters for
    // directed graphs)
    virtual double heuristic(int vertex, int target) = 0;
};
} // namespace SP
