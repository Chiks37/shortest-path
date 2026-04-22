/**
 * @file dijkstra_par_relaxation.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class with parallel relaxation header file
 */
#pragma once

#include "dijkstra_seq.hpp"
#include <atomic>

namespace SP
{

class DijkstraParRelaxationAlgo : public DijkstraSeqAlgo
{
  public:
    DijkstraParRelaxationAlgo(std::string graphFileName)
        : DijkstraSeqAlgo(graphFileName)
    {
    }

  protected:
    virtual ReturnCode runSearch() override;
    virtual void initInternalData() override;
    virtual void resetInternalData() override;
    virtual bool completeCondition(int currentVertex);

    std::vector<std::atomic_flag> vertexLocks;
    bool isQueueEmpty;
};
} // namespace SP