/**
 * @file abstract_dijkstra.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class header file
 */
#pragma once

#include "base.hpp"
#include "general.hpp"
#include <cmath>
#include <functional>
#include <queue>

namespace SP
{
class AbstractDijkstraAlgo : public BaseAlgo
{
  public:
    AbstractDijkstraAlgo(std::string graphFileName) : BaseAlgo(graphFileName) {}
    virtual ~AbstractDijkstraAlgo() {}

  protected:
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode computeImpl() override;

    // Per-vertex data is initialized once in preProcess. Everything a query
    // costs is paid inside compute: initQuery starts the search from the
    // source, resetInternalData returns only the vertices the query touched to
    // their initial state.
    virtual void initInternalData();
    virtual void initQuery();
    virtual void resetInternalData();

    virtual ReturnCode runSearch() = 0;
    virtual ReturnCode buildResult();
    virtual double estimateCost(int vertex) { return getDistance(vertex); }
    virtual double getDistance(int vertex);
    virtual std::vector<int> reconstructPath(int destination);
    virtual bool completeCondition(int currentVertex);

    void updateLabel(int vertex, double distance, int parent)
    {
        if (std::isinf(distances[vertex]))
        {
            touched.push_back(vertex);
        }
        distances[vertex] = distance;
        parents[vertex] = parent;
    }

    // Shortest distances for each vertex
    std::vector<double> distances;
    // Parents for each vertex
    std::vector<int> parents;
    // Vertices labeled by the current query
    std::vector<int> touched;
};
} // namespace SP