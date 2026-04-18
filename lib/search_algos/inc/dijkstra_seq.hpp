/**
 * @file dijkstra_seq.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm sequential version class header file
 */
#pragma once

#include "dijkstra.hpp"

namespace SP
{
class DijkstraSeqAlgo : public DijkstraAlgo
{
  public:
    DijkstraSeqAlgo(std::string graphfilename) : DijkstraAlgo(graphFileName) {}
  protected:
    virtual ReturnCode runSearch();

    virtual void initInternalData() override;
    virtual void resetInternalData() override;

    // Priority queue for storing of next vertexes to be considered
    std::priority_queue<edge, std::vector<edge>, compareEdges> pq;
};
} // namespace SP