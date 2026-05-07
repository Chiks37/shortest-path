/**
 * @file delta_stepping.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Delta-Stepping algorithm class header file
 */
#pragma once

#include "dijkstra.hpp"
#include <atomic>
#include <cstddef>

namespace SP
{
class DeltaSteppingAlgo : public DijkstraAlgo
{
  public:
    DeltaSteppingAlgo(std::string graphFileName) : DijkstraAlgo(graphFileName) {}

  protected:
    virtual void initInternalData() override;
    virtual void resetInternalData() override;
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode runSearch() override;

    double delta{1.0};
    std::vector<std::vector<edge>> lightEdges;
    std::vector<std::vector<edge>> heavyEdges;
    std::vector<std::vector<int>> buckets;
    std::vector<int> bucketInsertStamp;
    std::vector<std::size_t> bucketInsertBucket;
    int insertStamp{0};
    std::vector<std::atomic_flag> vertexLocks;
};
} // namespace SP