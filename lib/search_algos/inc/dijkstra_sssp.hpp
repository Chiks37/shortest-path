/**
 * @file dijkstra_sssp.hpp
 * @author tarakanov.2004@mail.ru
 * @brief SSSP Dijkstra algorithm class header file
 */
#pragma once

#include "abstract_dijkstra_seq.hpp"

namespace SP
{
class DijkstraSsspAlgo final : public AbstractDijkstraSeqAlgo
{
  public:
    DijkstraSsspAlgo(std::string graphFileName)
        : AbstractDijkstraSeqAlgo(graphFileName)
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