/**
 * @file dijkstra_seq.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm sequential version class header file
 */
#pragma once

#include "abstract_dijkstra_seq.hpp"

namespace SP
{
class DijkstraSeqAlgo final : public AbstractDijkstraSeqAlgo
{
  public:
    DijkstraSeqAlgo(std::string graphFileName)
        : AbstractDijkstraSeqAlgo(graphFileName)
    {
    }
};
} // namespace SP
