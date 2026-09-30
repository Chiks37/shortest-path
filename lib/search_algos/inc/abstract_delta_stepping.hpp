/**
 * @file abstract_delta_stepping.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Delta-Stepping algorithm class header file
 */
#pragma once

#include "abstract_dijkstra.hpp"
#include "vertex_cache.hpp"
#include <atomic>
#include <cstddef>
#include <limits>
#include <vector>

namespace SP
{
class AbstractDeltaSteppingAlgo : public AbstractDijkstraAlgo
{
  public:
    // Bucket width, the mean edge weight when not set. Takes effect at the
    // next preProcess.
    void setDelta(double delta) { requestedDelta = delta; }

  protected:
    AbstractDeltaSteppingAlgo(std::string graphFileName)
        : AbstractDijkstraAlgo(graphFileName)
    {
    }

    // Part of a direction owned by one thread, alone in its cache lines
    struct alignas(64) ThreadState
    {
        // Buckets after the current one
        std::vector<std::vector<int>> bins;
        // Vertices of the current bucket, by phase parity
        std::vector<int> frontier[2];
        std::vector<int> fused;
        std::size_t frontierSizes[2]{0, 0};
        // Vertices settled in the current bucket
        std::vector<int> settled;
        // Vertices labeled by the query
        std::vector<int> touched;
        std::vector<std::size_t> offsets;
        std::size_t nextBucket{0};
    };

    // State of one search direction. Every thread keeps its own buckets, its
    // part of the frontier and the vertices it labeled, so threads meet only
    // at the barriers between phases.
    struct Direction
    {
        const crsGraph *graph{nullptr};
        // Edges of every vertex go light first, lightEnd[v] is the index of
        // its first heavy edge
        const std::vector<int> *lightEnd{nullptr};
        std::vector<double> *labels{nullptr};
        std::vector<int> *parents{nullptr};
        // The reduced weight of an edge (u, v) is w + sign * (p(v) - p(u))
        double potentialSign{1.0};

        std::vector<ThreadState> threads;
        std::size_t currentBucket{0};
        int parity{0};
    };

    virtual void initInternalData() override;
    virtual void initQuery() override;
    virtual void resetInternalData() override;
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode runSearch() override;

    void computeDelta();
    void orderEdges(crsGraph &searchGraph, std::vector<int> &lightEnds);
    void startDirection(Direction &direction, int root);
    void resetDirection(Direction &direction);
    // Settles the current bucket of the direction and moves to the next one.
    // Every thread of the team calls it and gets the same answer: whether the
    // search goes on.
    bool processBucket(Direction &direction, const Direction *other,
                       bool stopAtDestination);
    template <bool Heavy>
    void relaxEdges(Direction &direction, const Direction *other,
                    ThreadState &state, int vertex, double label,
                    int insertParity);
    std::size_t bucketOf(double label) const
    {
        return static_cast<std::size_t>(label / delta);
    }

    // Goal-directed variants run on reduced weights w(u, v) + p(v) - p(u)
    virtual double computePotential(int) { return 0.0; }
    double potential(int vertex)
    {
        return potentials.get(vertex, [&] { return computePotential(vertex); });
    }

    double delta{1.0};
    double requestedDelta{0.0};
    bool usePotential{false};
    VertexCache potentials;
    std::vector<int> lightEnd;
    std::vector<std::atomic_flag> vertexLocks;
    // A vertex relaxes its heavy edges once per bucket: the stamp of the last
    // heavy phase it took part in
    std::vector<unsigned> heavyStamps;
    unsigned heavyStamp{1};
    std::atomic<bool> destinationSettled{false};
    Direction forwardDirection;

    // Best path through a vertex labeled by both directions, used by the
    // bidirectional variants
    std::atomic<double> shortestPathLength{
        std::numeric_limits<double>::infinity()};
    int meetingVertex{-1};
};
} // namespace SP
