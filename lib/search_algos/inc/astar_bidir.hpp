/**
 * @file astar_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar algorithm class header file
 */
#pragma once

#include "dijkstra_bidir.hpp"

namespace SP
{
class AStarBiDirAlgo : public DijkstraBiDirAlgo
{
  public:
    AStarBiDirAlgo(std::string graphFileName) : DijkstraBiDirAlgo(graphFileName)
    {
    }
    virtual ~AStarBiDirAlgo() {}

  protected:
    virtual void resetInternalData() override;
    virtual double estimateCost(int vertex) override;
    virtual double estimateCostBackward(int vertex) override;
    virtual double heuristic(int vertex, int target) = 0;

    // h(v, source) and h(v, destination) cached per query (filled in
    // resetInternalData once source/destination are known). The forward search
    // uses the symmetric potential (h_to_dest - h_to_src) / 2; the backward one
    // uses its negation. With h_f + h_b = 0 the two searches share a single
    // consistent meeting cost, which is what makes bidirectional A* sound.
    std::vector<double> cachedHsrc;
    std::vector<double> cachedHdst;
};
} // namespace SP
