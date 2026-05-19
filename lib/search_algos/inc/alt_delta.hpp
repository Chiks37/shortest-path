/**
 * @file alt_delta.hpp
 * @author tarakanov.2004@mail.ru
 * @brief ALT delta-stepping algorithm class header file
 */
#pragma once

#include "astar_delta.hpp"
#include "dijkstra_sssp.hpp"

namespace SP
{
class ALTDeltaAlgo : public AStarDeltaAlgo
{
  public:
    ALTDeltaAlgo(std::string graphFileName) : AStarDeltaAlgo(graphFileName) {}
    virtual ~ALTDeltaAlgo() {}

  protected:
    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex) override;
    ReturnCode runDijkstraSssp(DijkstraSsspAlgo &dijkstra, int source,
                               std::vector<double> &distances);

    ReturnCode initLandmarks();

    std::vector<std::vector<double>>
        distToLandmarks; // @TODO For WIP version we think thath grap is not
                         // oriented, so distToLandmarks is the same as
                         // distFromLandmarks
    std::vector<std::vector<double>> distFromLandmarks;
    std::vector<int> landmarks;
    int landmarksCount = 16; // Can be reduced if graph is small
};
} // namespace SP
