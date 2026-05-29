/**
 * @file abstract_dijkstra.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class header file
 */
#pragma once

#include "base.hpp"
#include "general.hpp"
#include <functional>
#include <queue>

namespace SP
{
class AbstractDijkstraAlgo : public BaseAlgo
{
  public:
    AbstractDijkstraAlgo(std::string graphFileName) : BaseAlgo(graphFileName) {}
    virtual ~AbstractDijkstraAlgo() {}
    virtual ReturnCode setSrcDest(int source, int destination) override;

  protected:
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode computeImpl() override;

    virtual void initInternalData();
    virtual void resetInternalData();

    virtual ReturnCode runSearch() = 0;
    virtual ReturnCode buildResult();
    virtual double estimateCost(int vertex) { return getDistance(vertex); }
    virtual double getDistance(int vertex);
    virtual std::vector<int> reconstructPath(int destination);
    virtual bool completeCondition(int currentVertex);

    // Shortest distances for each vertex
    std::vector<double> distances;
    // Parents for each vertex
    std::vector<int> parents;
};
} // namespace SP