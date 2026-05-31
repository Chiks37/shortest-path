/**
 * @file dijkstra_seq_pairing_heap.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm sequential version with boost::heap::pairing_heap
 *        class header file
 */
#pragma once

#include "abstract_dijkstra_seq_mutable.hpp"
#include <boost/heap/pairing_heap.hpp>

namespace SP
{
class DijkstraSeqPairingHeapAlgo final
    : public AbstractDijkstraSeqMutableAlgo<
          boost::heap::pairing_heap<edge, boost::heap::compare<compareEdges>>>
{
  public:
    DijkstraSeqPairingHeapAlgo(std::string graphFileName)
        : AbstractDijkstraSeqMutableAlgo(graphFileName)
    {
    }
};
} // namespace SP
