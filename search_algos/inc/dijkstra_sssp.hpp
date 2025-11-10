/**
 * @file dijkstra_sssp.hpp
 * @author tarakanov.2004@mail.ru
 * @brief SSSP Dijkstra algorithm class header file
 */
#pragma once

#include "dijkstra.hpp"

namespace SP
{
class DijkstraSsspAlgo : public DijkstraAlgo
{
  public:
    DijkstraSsspAlgo(std::string graphFileName) : DijkstraAlgo(graphFileName) {}

    std::vector<double> getDistances() const { return distances; }

  protected:
    virtual ReturnCode buildResult() override { return ReturnCode::OK; }
    virtual bool completeCondition(int currentVertex) { return false; }
};
} // namespace SP