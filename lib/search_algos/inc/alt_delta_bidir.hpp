/**
 * @file alt_delta_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional ALT delta-stepping algorithm class header file
 */
#pragma once

#include "abstract_astar_delta_bidir.hpp"
#include "dijkstra_sssp.hpp"

namespace SP
{
class ALTDeltaBiDirAlgo final : public AbstractAStarDeltaBiDirAlgo
{
  public:
    ALTDeltaBiDirAlgo(std::string graphFileName)
        : AbstractAStarDeltaBiDirAlgo(graphFileName)
    {
    }
    virtual ~ALTDeltaBiDirAlgo() {}

  protected:
    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex, int target) override;
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
