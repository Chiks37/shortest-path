/**
 * @file abstract_dijkstra_seq_mutable.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm sequential version with mutable priority queue
 *        (decrease_key) class header file
 */
#pragma once

#include "abstract_dijkstra.hpp"

namespace SP
{
template <typename PQ>
class AbstractDijkstraSeqMutableAlgo : public AbstractDijkstraAlgo
{
  protected:
    AbstractDijkstraSeqMutableAlgo(std::string graphFileName)
        : AbstractDijkstraAlgo(graphFileName)
    {
    }

    virtual ReturnCode runSearch();

    virtual void initInternalData() override;
    virtual void resetInternalData() override;

    PQ pq;
    std::vector<typename PQ::handle_type> handles;
    std::vector<bool> handleValid;
};
} // namespace SP
