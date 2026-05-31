/**
 * @file dijkstra_seq_binomial_heap.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm sequential version with boost::heap::binomial_heap
 *        class header file
 */
#pragma once

#include "abstract_dijkstra_seq_mutable.hpp"
#include <boost/heap/binomial_heap.hpp>

namespace SP
{
class DijkstraSeqBinomialHeapAlgo final
    : public AbstractDijkstraSeqMutableAlgo<
          boost::heap::binomial_heap<edge, boost::heap::compare<compareEdges>>>
{
  public:
    DijkstraSeqBinomialHeapAlgo(std::string graphFileName)
        : AbstractDijkstraSeqMutableAlgo(graphFileName)
    {
    }
};
} // namespace SP
