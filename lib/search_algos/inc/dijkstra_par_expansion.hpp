/**
 * @file dijkstra_par_expansion.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class with parallel expansion header file
 */

#include "dijkstra.hpp"
#include "priority_queue.hpp"

namespace SP
{
class DijkstraParExpansionAlgo : public DijkstraAlgo
{
  public:
    DijkstraParExpansionAlgo(std::string graphFileName)
        : DijkstraAlgo(graphFileName)
    {
    }

  protected:
    virtual ReturnCode runSearch() override;

    virtual void initInternalData() override;
    virtual void resetInternalData() override;

    TrackedPriorityQueue trackedPQ;
    std::atomic<double> bestDestDistance;
    std::vector<std::atomic_flag> vertexLocks;
};
} // namespace SP