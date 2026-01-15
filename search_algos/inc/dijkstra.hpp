/**
 * @file dijkstra.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class header file
 */
#pragma once

#include "base.hpp"
#include <functional>
#include <queue>

namespace SP
{
class DijkstraAlgo : public BaseAlgo
{
  public:
    DijkstraAlgo(std::string graphFileName) : BaseAlgo(graphFileName) {}
    virtual ~DijkstraAlgo() {}
    virtual ReturnCode setSource(int source) override;

  protected:
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode computeImpl() override;

    void resetInternalData();

    virtual ReturnCode runSearch();
    virtual ReturnCode buildResult();
    virtual double estimateCost(int vertex);
    virtual double getDistance(int vertex);
    virtual std::vector<int> reconstructPath(int destination);
    virtual bool completeCondition(int currentVertex);

    // Shortest distances for each vertex
    std::vector<double> distances;
    // Parents for each vertex
    std::vector<int> parents;

    // Edge comparator for setting priority queue
    struct compareEdges
    {
        bool operator()(const edge &e1, const edge &e2)
        {
            return e1.val > e2.val;
        }
    };

    // Priority queue for storing of next vertexes to be considered
    std::priority_queue<edge, std::vector<edge>, compareEdges> pq;
};
} // namespace SP