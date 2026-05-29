/**
 * @file abstract_dijkstra_seq.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm sequential version class header file
 */
#pragma once

#include "abstract_dijkstra.hpp"

namespace SP
{
class AbstractDijkstraSeqAlgo : public AbstractDijkstraAlgo
{
  protected:
    AbstractDijkstraSeqAlgo(std::string graphFileName)
        : AbstractDijkstraAlgo(graphFileName)
    {
    }

    virtual ReturnCode runSearch();

    virtual void initInternalData() override;
    virtual void resetInternalData() override;

    // Priority queue for storing of next vertexes to be considered
    std::priority_queue<edge, std::vector<edge>, compareEdges> pq;
};
} // namespace SP