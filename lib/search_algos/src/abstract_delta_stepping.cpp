/**
 * @file abstract_delta_stepping.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Delta-Stepping algorithm class source file
 */

#include "abstract_delta_stepping.hpp"
#include <algorithm>
#include <cmath>
#include <omp.h>
#include <utility>

namespace SP
{
namespace
{
constexpr std::size_t kNoBucket = std::numeric_limits<std::size_t>::max();
// A thread settles the vertices it put back into the current bucket itself
// while there are fewer of them than this, instead of sharing them in the next
// phase (bin fusion, as in the GAP benchmark suite)
constexpr std::size_t kFusionThreshold = 1000;

double loadLabel(std::vector<double> &labels, int vertex)
{
    return std::atomic_ref<double>(labels[vertex])
        .load(std::memory_order_relaxed);
}
} // namespace

void AbstractDeltaSteppingAlgo::initInternalData()
{
    AbstractDijkstraAlgo::initInternalData();

    vertexLocks = std::vector<std::atomic_flag>(graph.V);
    for (auto &vertexLock : vertexLocks)
    {
        vertexLock.clear(std::memory_order_release);
    }
    heavyStamps.assign(graph.V, 0);
    heavyStamp = 1;
    potentials.resize(graph.V);

    forwardDirection.graph = &graph;
    forwardDirection.lightEnd = &lightEnd;
    forwardDirection.labels = &distances;
    forwardDirection.parents = &parents;
    forwardDirection.potentialSign = 1.0;
}

void AbstractDeltaSteppingAlgo::initQuery()
{
    AbstractDijkstraAlgo::initQuery();

    potentials.nextQuery();
    destinationSettled.store(false);
    shortestPathLength.store(std::numeric_limits<double>::infinity());
    meetingVertex = -1;
    startDirection(forwardDirection, this->source);
}

void AbstractDeltaSteppingAlgo::resetInternalData()
{
    resetDirection(forwardDirection);

    AbstractDijkstraAlgo::resetInternalData();
}

void AbstractDeltaSteppingAlgo::computeDelta()
{
    if (requestedDelta > 0.0)
    {
        delta = requestedDelta;
        return;
    }

    if (graph.nz <= 0)
    {
        delta = 1.0;
        return;
    }

    double sum = 0.0;
    for (int i = 0; i < graph.nz; ++i)
    {
        sum += graph.Eweights[i];
    }

    delta = sum / static_cast<double>(graph.nz);
    if (!std::isfinite(delta) || delta <= 0.0)
    {
        delta = 1.0;
    }
}

void AbstractDeltaSteppingAlgo::orderEdges(crsGraph &searchGraph,
                                           std::vector<int> &lightEnds)
{
    lightEnds.resize(searchGraph.V);
    for (int vertex = 0; vertex < searchGraph.V; ++vertex)
    {
        int split = searchGraph.Xadj[vertex];
        for (int i = split; i < searchGraph.Xadj[vertex + 1]; ++i)
        {
            if (searchGraph.Eweights[i] < delta)
            {
                std::swap(searchGraph.Adjncy[i], searchGraph.Adjncy[split]);
                std::swap(searchGraph.Eweights[i], searchGraph.Eweights[split]);
                ++split;
            }
        }
        lightEnds[vertex] = split;
    }
}

ReturnCode AbstractDeltaSteppingAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractDijkstraAlgo::preProcessImpl();
    if (rc != ReturnCode::OK)
    {
        return rc;
    }

    computeDelta();
    orderEdges(graph, lightEnd);

    return rc;
}

void AbstractDeltaSteppingAlgo::startDirection(Direction &direction, int root)
{
    std::size_t threads = static_cast<std::size_t>(omp_get_max_threads());
    if (direction.threads.size() < threads)
    {
        direction.threads.resize(threads);
        for (auto &state : direction.threads)
        {
            state.offsets.resize(threads + 1);
        }
    }

    direction.threads[0].frontier[0].push_back(root);
    direction.threads[0].frontierSizes[0] = 1;
    direction.currentBucket = 0;
    direction.parity = 0;
}

void AbstractDeltaSteppingAlgo::resetDirection(Direction &direction)
{
    for (auto &state : direction.threads)
    {
        for (int vertex : state.touched)
        {
            (*direction.labels)[vertex] =
                std::numeric_limits<double>::infinity();
            (*direction.parents)[vertex] = -1;
        }
        state.touched.clear();

        for (auto &bin : state.bins)
        {
            bin.clear();
        }
        state.frontier[0].clear();
        state.frontier[1].clear();
        state.frontierSizes[0] = 0;
        state.frontierSizes[1] = 0;
        state.settled.clear();
    }
}

ReturnCode AbstractDeltaSteppingAlgo::runSearch()
{
#pragma omp parallel
    {
        while (processBucket(forwardDirection, nullptr, true))
        {
        }
    }

    return ReturnCode::OK;
}

bool AbstractDeltaSteppingAlgo::processBucket(Direction &direction,
                                              const Direction *other,
                                              bool stopAtDestination)
{
    int thread = omp_get_thread_num();
    int threads = omp_get_num_threads();
    std::size_t bucket = direction.currentBucket;
    int parity = direction.parity;
    ThreadState &state = direction.threads[thread];
    auto &offsets = state.offsets;

    // Light phases: a vertex whose label drops into this bucket comes back
    // into the frontier until the bucket stays empty
    while (true)
    {
        state.frontier[1 - parity].clear();
        offsets[0] = 0;
        for (int t = 0; t < threads; ++t)
        {
            offsets[t + 1] =
                offsets[t] + direction.threads[t].frontierSizes[parity];
        }
        std::size_t total = offsets[threads];
        if (total == 0)
        {
            break;
        }

        auto settle = [&](int vertex)
        {
            double label = loadLabel(*direction.labels, vertex);
            // The label dropped into an earlier bucket, which settled it
            if (bucketOf(label) != bucket)
            {
                return;
            }

            state.settled.push_back(vertex);
            if (stopAtDestination && vertex == this->destination)
            {
                destinationSettled.store(true, std::memory_order_relaxed);
            }
            relaxEdges<false>(direction, other, state, vertex, label,
                              1 - parity);
        };

        int owner = 0;
#pragma omp for schedule(dynamic, 64) nowait
        for (std::size_t i = 0; i < total; ++i)
        {
            while (i >= offsets[owner + 1])
            {
                ++owner;
            }
            settle(
                direction.threads[owner].frontier[parity][i - offsets[owner]]);
        }

        while (!state.frontier[1 - parity].empty() &&
               state.frontier[1 - parity].size() < kFusionThreshold)
        {
            state.fused.swap(state.frontier[1 - parity]);
            for (int vertex : state.fused)
            {
                settle(vertex);
            }
            state.fused.clear();
        }

        state.frontierSizes[1 - parity] = state.frontier[1 - parity].size();
#pragma omp barrier
        parity = 1 - parity;
    }

    // Every vertex of the bucket is settled here, the destination too
    if (stopAtDestination && destinationSettled.load(std::memory_order_relaxed))
    {
        return false;
    }

    for (int vertex : state.settled)
    {
        std::atomic_ref<unsigned> vertexStamp(heavyStamps[vertex]);
        if (vertexStamp.load(std::memory_order_relaxed) == heavyStamp)
        {
            continue;
        }
        vertexStamp.store(heavyStamp, std::memory_order_relaxed);
        relaxEdges<true>(direction, other, state, vertex,
                         loadLabel(*direction.labels, vertex), parity);
    }
    state.settled.clear();

    // A heavy edge leads out of the bucket, unless rounding brings its end
    // back into it; then the bucket is processed once more
    std::size_t next = kNoBucket;
    auto &bins = state.bins;
    if (!state.frontier[parity].empty())
    {
        next = bucket;
    }
    else
    {
        for (std::size_t b = bucket + 1; b < bins.size(); ++b)
        {
            if (!bins[b].empty())
            {
                next = b;
                break;
            }
        }
    }
    state.nextBucket = next;
#pragma omp barrier

    for (int t = 0; t < threads; ++t)
    {
        next = std::min(next, direction.threads[t].nextBucket);
    }
    if (next != kNoBucket && next != bucket && next < bins.size())
    {
        std::swap(bins[next], state.frontier[parity]);
    }
    state.frontierSizes[parity] = state.frontier[parity].size();

    // Every vertex closer than currentBucket * delta is settled in its
    // direction, so no path shorter than the sum of the two is left
    bool proceed = next != kNoBucket;
    if (proceed && other != nullptr)
    {
        proceed = static_cast<double>(next + other->currentBucket) * delta <
                  shortestPathLength.load();
    }

    if (thread == 0)
    {
        if (next != kNoBucket)
        {
            direction.currentBucket = next;
        }
        direction.parity = parity;
        if (++heavyStamp == 0)
        {
            std::fill(heavyStamps.begin(), heavyStamps.end(), 0);
            heavyStamp = 1;
        }
    }
#pragma omp barrier

    return proceed;
}

template <bool Heavy>
void AbstractDeltaSteppingAlgo::relaxEdges(Direction &direction,
                                           const Direction *other,
                                           ThreadState &state, int vertex,
                                           double label, int insertParity)
{
    const crsGraph &searchGraph = *direction.graph;
    int begin = searchGraph.Xadj[vertex];
    int end = searchGraph.Xadj[vertex + 1];
    double vertexPotential = 0.0;
    if (usePotential)
    {
        vertexPotential = direction.potentialSign * potential(vertex);
    }
    else if (Heavy)
    {
        begin = (*direction.lightEnd)[vertex];
    }
    else
    {
        end = (*direction.lightEnd)[vertex];
    }

    for (int i = begin; i < end; ++i)
    {
        int neighborVertex = searchGraph.Adjncy[i];
        double edgeWeight = searchGraph.Eweights[i];
        if (usePotential)
        {
            // A consistent heuristic keeps reduced weights non-negative up to
            // rounding
            edgeWeight = std::max(0.0, edgeWeight +
                                           direction.potentialSign *
                                               potential(neighborVertex) -
                                           vertexPotential);
            if ((edgeWeight >= delta) != Heavy)
            {
                continue;
            }
        }

        double newLabel = label + edgeWeight;
        std::atomic_ref<double> neighborLabel(
            (*direction.labels)[neighborVertex]);
        if (newLabel >= neighborLabel.load(std::memory_order_relaxed))
        {
            continue;
        }

        while (
            vertexLocks[neighborVertex].test_and_set(std::memory_order_acquire))
        {
        }
        double oldLabel = neighborLabel.load(std::memory_order_relaxed);
        bool updated = newLabel < oldLabel;
        if (updated)
        {
            neighborLabel.store(newLabel, std::memory_order_relaxed);
            (*direction.parents)[neighborVertex] = vertex;
        }
        vertexLocks[neighborVertex].clear(std::memory_order_release);

        if (!updated)
        {
            continue;
        }

        if (std::isinf(oldLabel))
        {
            state.touched.push_back(neighborVertex);
        }

        std::size_t neighborBucket = bucketOf(newLabel);
        if (neighborBucket == direction.currentBucket)
        {
            state.frontier[insertParity].push_back(neighborVertex);
        }
        else
        {
            auto &bins = state.bins;
            if (neighborBucket >= bins.size())
            {
                bins.resize(neighborBucket + 1);
            }
            bins[neighborBucket].push_back(neighborVertex);
        }

        if (other != nullptr)
        {
            double potentialPath =
                newLabel + loadLabel(*other->labels, neighborVertex);
            if (potentialPath < shortestPathLength.load())
            {
#pragma omp critical(deltaMeeting)
                {
                    if (potentialPath < shortestPathLength.load())
                    {
                        shortestPathLength.store(potentialPath);
                        meetingVertex = neighborVertex;
                    }
                }
            }
        }
    }
}

} // namespace SP
