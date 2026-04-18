/**
 * @file dijkstra_sssp.hpp
 * @author tarakanov.2004@mail.ru
 * @brief SSSP Dijkstra algorithm class header file
 */
#pragma once

#include "dijkstra_seq.hpp"

namespace SP
{
class DijkstraSsspAlgo : public DijkstraSeqAlgo
{
  public:
    DijkstraSsspAlgo(std::string graphFileName) : DijkstraSeqAlgo(graphFileName)
    {
    }

    std::vector<double> getDistances() const { return std::move(distances); }
    ReturnCode setSource(int source)
    {
        return BaseAlgo::setSrcDest(source, source);
    }

  protected:
    virtual ReturnCode buildResult() override { return ReturnCode::OK; }
    virtual bool completeCondition(int currentVertex) { return false; }
};
} // namespace SP