/**
 * @file dijkstra.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class header file
 */
#pragma once

#include "base.hpp"
#include <queue>
#include <functional>

namespace SP
{
class DijkstraAlgo : public BaseAlgo {
 public:

    DijkstraAlgo (AlgoInData inData) : BaseAlgo(inData) {}
    virtual ~DijkstraAlgo() {}

 protected:
   virtual ReturnCode preProcess() override;
   virtual ReturnCode process() override;
   virtual ReturnCode postProcess() override;
   virtual double estimateCost(int vertex);

   // Shortest distances for each vertex
   std::vector<double> distances;
   // Parents for each vertex
   std::vector<int> parents;

   // Edge comparator for setting priority queue
   struct compareEdges
   {
      bool operator() (const edge& e1, const edge& e2)
      {
         return e1.val > e2.val;
      }
   };

   // Priority queue for storing of next vertexes to be considered
   std::priority_queue<edge, std::vector<edge>, compareEdges> pq;
};
} // namespace SP