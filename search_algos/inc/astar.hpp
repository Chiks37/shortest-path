/**
 * @file dijkstra.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class header file
 */
#pragma once

#include "dijkstra.hpp"


namespace SP
{
class AStarAlgo : public DijkstraAlgo
{
  public:
    AStarAlgo(std::string graphFileName) : DijkstraAlgo(graphFileName) {}
    virtual ~AStarAlgo() {}

  protected:
    virtual double estimateCost(int vertex) override;
    virtual double heuristic(int vertex);
};
} // namespace SP