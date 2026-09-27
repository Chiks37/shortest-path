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

    const std::vector<double> &getDistances() const { return distances; }
    ReturnCode setSource(int source) { return setSrcDest(source, source); }

  protected:
    virtual ReturnCode buildResult() override { return ReturnCode::OK; }
    virtual bool completeCondition(int) override { return false; }
};
} // namespace SP