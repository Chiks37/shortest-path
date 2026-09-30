/**
 * @file dijkstra_par_expansion.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class with parallel expansion header file
 */
#pragma once

#include "abstract_dijkstra.hpp"
#include "tracked_pq.hpp"

namespace SP
{
class DijkstraParExpansionAlgo final : public AbstractDijkstraAlgo
{
  public:
    DijkstraParExpansionAlgo(std::string graphFileName)
        : AbstractDijkstraAlgo(graphFileName)
    {
    }

  protected:
    virtual ReturnCode runSearch() override;

    virtual void initInternalData() override;
    virtual void initQuery() override;
    virtual void resetInternalData() override;

    TrackedPriorityQueue trackedPQ;
    std::atomic<double> bestDestDistance;
    std::vector<std::atomic_flag> vertexLocks;
};
} // namespace SP