/**
 * @file delta_stepping.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Delta-Stepping algorithm class source file
 */

#include "delta_stepping.hpp"
#include <cmath>
#include <limits>
#include <utility>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace SP
{

void DeltaSteppingAlgo::initInternalData()
{
    DijkstraAlgo::initInternalData();

    vertexLocks = std::vector<std::atomic_flag>(graph.V);
    for (auto &vertexLock : vertexLocks)
    {
        vertexLock.clear(std::memory_order_release);
    }

    buckets.clear();
    buckets.resize(1);
    bucketInsertStamp.assign(graph.V, -1);
    bucketInsertBucket.assign(graph.V, 0);
    insertStamp = 0;
}

void DeltaSteppingAlgo::resetInternalData()
{
    DijkstraAlgo::resetInternalData();

    for (auto &vertexLock : vertexLocks)
    {
        vertexLock.clear(std::memory_order_release);
    }

    if (buckets.empty())
    {
        buckets.resize(1);
    }
    buckets[0].clear();
    buckets[0].push_back(this->source);

    if (!bucketInsertStamp.empty() && this->source >= 0 &&
        this->source < graph.V)
    {
        bucketInsertStamp[this->source] = 0;
        bucketInsertBucket[this->source] = 0;
    }
}

void DeltaSteppingAlgo::computeDelta()
{
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

void DeltaSteppingAlgo::classifyEdges()
{
    lightEdges.assign(graph.V, {});
    heavyEdges.assign(graph.V, {});

    for (int vertex = 0; vertex < graph.V; ++vertex)
    {
        for (int i = graph.Xadj[vertex]; i < graph.Xadj[vertex + 1]; ++i)
        {
            int neighborVertex = graph.Adjncy[i];
            double edgeWeight = graph.Eweights[i];

            if (edgeWeight < delta)
            {
                lightEdges[vertex].push_back({neighborVertex, edgeWeight});
            }
            else
            {
                heavyEdges[vertex].push_back({neighborVertex, edgeWeight});
            }
        }
    }
}

ReturnCode DeltaSteppingAlgo::preProcessImpl()
{
    ReturnCode rc = DijkstraAlgo::preProcessImpl();
    if (rc != ReturnCode::OK)
    {
        return rc;
    }

    computeDelta();
    classifyEdges();

    return rc;
}

ReturnCode DeltaSteppingAlgo::runSearch()
{
    if (graph.V <= 0)
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    const double inf = std::numeric_limits<double>::infinity();

    auto relaxVertices = [&](const std::vector<int> &vertices,
                             const std::vector<std::vector<edge>> &adj)
    {
        if (vertices.empty())
        {
            return;
        }

        int maxThreads = 1;
#ifdef _OPENMP
        maxThreads = omp_get_max_threads();
#endif

        std::vector<std::vector<std::pair<int, std::size_t>>> localBuffers(
            static_cast<std::size_t>(maxThreads));

#pragma omp parallel
        {
            int tid = 0;
#ifdef _OPENMP
            tid = omp_get_thread_num();
#endif
            auto &buffer = localBuffers[static_cast<std::size_t>(tid)];
            buffer.clear();

#pragma omp for schedule(static)
            for (std::size_t idx = 0; idx < vertices.size(); ++idx)
            {
                int currentVertex = vertices[idx];

                double baseDist = 0.0;
                while (vertexLocks[currentVertex].test_and_set(
                    std::memory_order_acquire))
                {
                }
                baseDist = getDistance(currentVertex);
                vertexLocks[currentVertex].clear(std::memory_order_release);

                if (baseDist == inf)
                {
                    continue;
                }

                const auto &edges = adj[currentVertex];
                for (const auto &e : edges)
                {
                    int neighborVertex = e.vertex;
                    double newDist = baseDist + e.val;
                    bool updated = false;
                    double neigbourEstimatedCost = 0.0;

                    while (vertexLocks[neighborVertex].test_and_set(
                        std::memory_order_acquire))
                    {
                    }

                    if (newDist < getDistance(neighborVertex))
                    {
                        distances[neighborVertex] = newDist;
                        parents[neighborVertex] = currentVertex;
                        neigbourEstimatedCost = estimateCost(neighborVertex);
                        updated = true;
                    }

                    vertexLocks[neighborVertex].clear(
                        std::memory_order_release);

                    if (updated)
                    {
                        std::size_t bucketIndex = static_cast<std::size_t>(
                            neigbourEstimatedCost / delta);
                        buffer.push_back({neighborVertex, bucketIndex});
                    }
                }
            }
        }

        ++insertStamp;
        std::size_t requiredSize = buckets.size();
        for (const auto &buffer : localBuffers)
        {
            for (const auto &item : buffer)
            {
                std::size_t bucketIndex = item.second;
                if (bucketIndex + 1 > requiredSize)
                {
                    requiredSize = bucketIndex + 1;
                }
            }
        }

        if (requiredSize > buckets.size())
        {
            buckets.resize(requiredSize);
        }

        for (const auto &buffer : localBuffers)
        {
            for (const auto &item : buffer)
            {
                int vertex = item.first;
                std::size_t bucketIndex = item.second;

                if (bucketInsertStamp[vertex] == insertStamp &&
                    bucketInsertBucket[vertex] == bucketIndex)
                {
                    continue;
                }

                bucketInsertStamp[vertex] = insertStamp;
                bucketInsertBucket[vertex] = bucketIndex;
                buckets[bucketIndex].push_back(vertex);
            }
        }
    };

    std::size_t currentBucket = 0;
    while (currentBucket < buckets.size())
    {
        while (currentBucket < buckets.size() && buckets[currentBucket].empty())
        {
            ++currentBucket;
        }

        if (currentBucket >= buckets.size())
        {
            break;
        }

        std::vector<int> settled;
        settled.reserve(buckets[currentBucket].size());
        bool destSettled = false;

        while (!buckets[currentBucket].empty())
        {
            std::vector<int> request;
            request.swap(buckets[currentBucket]);
            if (request.empty())
            {
                break;
            }

            if (!destSettled)
            {
                for (int vertex : request)
                {
                    if (vertex == destination)
                    {
                        destSettled = true;
                        break;
                    }
                }
            }

            settled.insert(settled.end(), request.begin(), request.end());
            relaxVertices(request, lightEdges);
        }

        if (!settled.empty())
        {
            relaxVertices(settled, heavyEdges);
        }

        if (destSettled)
        {
            break;
        }

        ++currentBucket;
    }

    return ReturnCode::OK;
}

} // namespace SP
