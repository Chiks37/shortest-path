/**
 * @file dijkstra_par_expansion.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class with parallel expansion source file
 */

#include "dijkstra_par_expansion.hpp"

namespace SP
{
ReturnCode DijkstraParExpansionAlgo::runSearch()
{
#pragma omp parallel
    {
        // Каждый поток крутится, пока алгоритм не завершен
        while (!trackedPQ.is_drained())
        {
            edge currentEdge;

            // Пытаемся взять задачу. Если пусто, но алгоритм не завершен - ждем
            if (!trackedPQ.try_pop(currentEdge))
            {
                std::this_thread::yield(); // Отдаем квант времени ОС, чтобы не
                                           // греть процессор
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

            // Отсечение устаревших путей (чтение без блокировки допустимо,
            // так как если прочитаем старое значение, просто сделаем лишнюю
            // проверку ниже)
            if (currentPoppedCost > estimateCost(currentVertex))
            {
                trackedPQ.mark_as_done(); // Обязательно отмечаем задачу как
                                          // выполненную!
                continue;
            }

            // Обход соседей (Expansion)
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
                    distances[neighborVertex] = neighbVerNewDistance;
                    parents[neighborVertex] = currentVertex;
                    updated = true;
                }

                vertexLocks[neighborVertex].clear(std::memory_order_release);

                // Если путь улучшен, добавляем соседа в очередь (вне
                // критической секции!)
                if (updated)
                {
                    double neigbourEstimatedCost = estimateCost(neighborVertex);
                    trackedPQ.push({neighborVertex, neigbourEstimatedCost});
                }
            }

            // Вершина полностью обработана, уменьшаем счетчик активных задач
            trackedPQ.mark_as_done();
        }
    }

    return ReturnCode::OK;
}

void DijkstraParExpansionAlgo::initInternalData()
{
    DijkstraAlgo::initInternalData();

    // Clear the pq
    trackedPQ.clear();

    bestDestDistance.store(std::numeric_limits<double>::infinity());
    vertexLocks = std::vector<std::atomic_flag>(graph.V);
}

void DijkstraParExpansionAlgo::resetInternalData()
{
    DijkstraAlgo::resetInternalData();

    double sourceEstimatedCost = estimateCost(this->source);
    trackedPQ.push({this->source, sourceEstimatedCost});

    for (auto &vertexLock : vertexLocks)
    {
        vertexLock.clear(std::memory_order_release);
    }
}

} // namespace SP
