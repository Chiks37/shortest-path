/**
 * @file dijkstra_par_relaxation.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class with parallel relaxation header file
 */

#include "dijkstra_par_relaxation.hpp"

namespace SP
{

ReturnCode DijkstraParRelaxationAlgo::runSearch()
{
#pragma omp parallel
    {
        std::vector<edge> localVertexBuffer;
        while (true)
        {
#pragma omp barrier

            int currentVertex = -1;
            double curVerPoppedEstCost = 0.0;

#pragma omp single copyprivate(currentVertex, curVerPoppedEstCost)
            {
                isQueueEmpty = pq.empty();
                if (!isQueueEmpty)
                {
                    currentVertex = pq.top().vertex;
                    curVerPoppedEstCost = pq.top().val;
                    pq.pop();
                }
            }

            if (completeCondition(currentVertex))
                break;
            if (curVerPoppedEstCost > estimateCost(currentVertex))
                continue;

            localVertexBuffer.clear();

#pragma omp for nowait
            for (int i = graph.Xadj[currentVertex];
                 i < graph.Xadj[currentVertex + 1]; i++)
            {
                int neighborVertex = graph.Adjncy[i];
                double neighbVerNewDistance =
                    distances[currentVertex] + graph.Eweights[i];

                if (distances[neighborVertex] > neighbVerNewDistance)
                {
                    while (vertexLocks[neighborVertex].test_and_set(
                        std::memory_order_acquire))
                    {
                    }

                    if (distances[neighborVertex] > neighbVerNewDistance)
                    {
                        distances[neighborVertex] = neighbVerNewDistance;
                        parents[neighborVertex] = currentVertex;

                        double neigbourEstimatedCost =
                            estimateCost(neighborVertex);
                        localVertexBuffer.push_back(
                            {neighborVertex, neigbourEstimatedCost});
                    }

                    vertexLocks[neighborVertex].clear(
                        std::memory_order_release);
                }
            }

#pragma omp critical
            {
                for (auto &e : localVertexBuffer)
                {
                    pq.push(e);
                }
            }
        }
    }
    return ReturnCode::OK;
}

void DijkstraParRelaxationAlgo::initInternalData()
{
    AbstractDijkstraSeqAlgo::initInternalData();

    vertexLocks = std::vector<std::atomic_flag>(graph.V);
    for (auto &vertexLock : vertexLocks)
    {
        vertexLock.clear(std::memory_order_release);
    }
}

void DijkstraParRelaxationAlgo::resetInternalData()
{
    AbstractDijkstraSeqAlgo::resetInternalData();

    for (auto &vertexLock : vertexLocks)
    {
        vertexLock.clear(std::memory_order_release);
    }

    bool isQueueEmpty = false;
}

bool DijkstraParRelaxationAlgo::completeCondition(int currentVertex)
{
    return isQueueEmpty ||
           AbstractDijkstraSeqAlgo::completeCondition(currentVertex);
}

} // namespace SP
