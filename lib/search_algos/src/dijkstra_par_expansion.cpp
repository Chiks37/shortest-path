/**
 * @file dijkstra_par_expansion.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class with parallel expansion source file
 */

#include "dijkstra_par_expansion.hpp"
#include <cmath>

namespace SP
{
ReturnCode DijkstraParExpansionAlgo::runSearch()
{
#pragma omp parallel
    {
        std::vector<int> localTouched;
        while (!trackedPQ.is_drained())
        {
            edge currentEdge;
            if (!trackedPQ.try_pop(currentEdge))
            {
                std::this_thread::yield();
                continue;
            }

            int currentVertex = currentEdge.vertex;
            double currentPoppedCost = currentEdge.val;

            if (currentPoppedCost >=
                bestDestDistance.load(std::memory_order_relaxed))
            {
                trackedPQ.mark_as_done();
                continue;
            }

            if (currentPoppedCost > estimateCost(currentVertex))
            {
                trackedPQ.mark_as_done();
                continue;
            }

            for (int i = graph.Xadj[currentVertex];
                 i < graph.Xadj[currentVertex + 1]; i++)
            {
                int neighborVertex = graph.Adjncy[i];
                double neighborVertexWeight = graph.Eweights[i];

                double neighbVerNewDistance =
                    distances[currentVertex] + neighborVertexWeight;
                bool updated = false;

                // Spinlock for exact vertex
                while (vertexLocks[neighborVertex].test_and_set(
                    std::memory_order_acquire))
                {
                }

                if (distances[neighborVertex] > neighbVerNewDistance)
                {
                    if (std::isinf(distances[neighborVertex]))
                    {
                        localTouched.push_back(neighborVertex);
                    }
                    distances[neighborVertex] = neighbVerNewDistance;
                    parents[neighborVertex] = currentVertex;
                    updated = true;

                    if (neighborVertex == destination)
                    {
                        bestDestDistance.store(neighbVerNewDistance,
                                               std::memory_order_relaxed);
                    }
                }

                vertexLocks[neighborVertex].clear(std::memory_order_release);

                if (updated)
                {
                    double neigbourEstimatedCost = estimateCost(neighborVertex);
                    trackedPQ.push({neighborVertex, neigbourEstimatedCost});
                }
            }

            trackedPQ.mark_as_done();
        }

#pragma omp critical
        touched.insert(touched.end(), localTouched.begin(), localTouched.end());
    }

    return ReturnCode::OK;
}

void DijkstraParExpansionAlgo::initInternalData()
{
    AbstractDijkstraAlgo::initInternalData();

    trackedPQ.clear();
    vertexLocks = std::vector<std::atomic_flag>(graph.V);
    for (auto &vertexLock : vertexLocks)
    {
        vertexLock.clear(std::memory_order_release);
    }
}

void DijkstraParExpansionAlgo::initQuery()
{
    AbstractDijkstraAlgo::initQuery();

    bestDestDistance.store(distances[this->destination]);

    double sourceEstimatedCost = estimateCost(this->source);
    trackedPQ.push({this->source, sourceEstimatedCost});
}

void DijkstraParExpansionAlgo::resetInternalData()
{
    trackedPQ.clear();

    AbstractDijkstraAlgo::resetInternalData();
}

} // namespace SP
