/**
 * @file abstract_astar_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar algorithm class header file
 */
#pragma once

#include "abstract_dijkstra_bidir.hpp"
#include "vertex_cache.hpp"

namespace SP
{
class AbstractAStarBiDirAlgo : public AbstractDijkstraBiDirAlgo
{
  public:
    AbstractAStarBiDirAlgo(std::string graphFileName)
        : AbstractDijkstraBiDirAlgo(graphFileName)
    {
    }
    virtual ~AbstractAStarBiDirAlgo() {}

  protected:
    virtual void initInternalData() override;
    virtual void initQuery() override;
    virtual double estimateCost(int vertex) override
    {
        return distances[vertex] + potential(vertex);
    }
    virtual double estimateCostBackward(int vertex) override
    {
        return distancesBackward[vertex] - potential(vertex);
    }
    // Lower bound on the distance from vertex to target (order matters for
    // directed graphs)
    virtual double heuristic(int vertex, int target) = 0;

    // The forward search uses the potential (h(v, destination) -
    // h(source, v)) / 2, the backward one its negation. The two sum up to zero,
    // so both searches work on the same reduced weights and the stopping
    // criterion of bidirectional Dijkstra stays valid.
    double potential(int vertex)
    {
        return potentials.get(vertex,
                              [&]
                              {
                                  return 0.5 *
                                         (heuristic(vertex, this->destination) -
                                          heuristic(this->source, vertex));
                              });
    }

    VertexCache potentials;
};
} // namespace SP
