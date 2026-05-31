/**
 * @file dijkstra_seq_d_ary_heap.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm sequential version with boost::heap::d_ary_heap
 *        (arity=4, mutable) class header file
 */
#pragma once

#include "abstract_dijkstra_seq_mutable.hpp"
#include <boost/heap/d_ary_heap.hpp>

namespace SP
{
class DijkstraSeqDaryHeapAlgo final
    : public AbstractDijkstraSeqMutableAlgo<boost::heap::d_ary_heap<
          edge, boost::heap::compare<compareEdges>, boost::heap::arity<4>,
          boost::heap::mutable_<true>>>
{
  public:
    DijkstraSeqDaryHeapAlgo(std::string graphFileName)
        : AbstractDijkstraSeqMutableAlgo(graphFileName)
    {
    }
};
} // namespace SP
