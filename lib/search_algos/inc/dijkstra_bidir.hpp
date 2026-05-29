/**
 * @file dijkstra_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra (bidirectional version) algorithm class header file
 */
#pragma once

#include "abstract_dijkstra_bidir.hpp"

namespace SP
{
class DijkstraBiDirAlgo final : public AbstractDijkstraBiDirAlgo
{
  public:
    DijkstraBiDirAlgo(std::string graphFileName)
        : AbstractDijkstraBiDirAlgo(graphFileName)
    {
    }
};
} // namespace SP
