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

    virtual void initQuery() override;
    virtual void resetInternalData() override;

    // Priority queue for storing of next vertexes to be considered
    EdgeQueue pq;
};
} // namespace SP