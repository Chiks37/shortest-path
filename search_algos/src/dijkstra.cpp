/**
 * @file dijkstra.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm class source file
 */

#include "dijkstra.hpp"
#include <limits>

namespace SP
{
    ReturnCode DijkstraAlgo::preProcess()
    {
        ReturnCode rc = BaseAlgo::preProcess();
        if (ReturnCode::OK != rc)
        {
            return rc;
        }

        // All vertex weights but source weight are endless untill they relaxed 
        distances.assign(graph.V, std::numeric_limits<double>::infinity());
        distances[data.source] = 0.0;

        // Parents are unknown for now
        parents.assign(graph.V, -1);

        // Source vertex is first to consider
        double sourceEstimatedCost = estimateCost(data.source);
        pq.push({data.source, sourceEstimatedCost});

        return rc;
    }

    ReturnCode DijkstraAlgo::process()
    {
        while (!pq.empty())
        {

            // Getting current vertex info from priority queue
            int currentVertex = pq.top().vertex;
            double curVerPoppedEstCost = pq.top().val;
            pq.pop();

            // Skip if there is already shorter path to current vertex than we trying to calculate
            double curVerStoredEstCost = estimateCost(currentVertex);
            if (curVerPoppedEstCost > curVerStoredEstCost) continue;

            // Early exit condition: we already reached destination vertex
            if (currentVertex == data.destination)
            {
                break;
            }

            // Researching neighbors
            for (int i = graph.Xadj[currentVertex]; i < graph.Xadj[currentVertex + 1]; i++) {
                int neighborVertex = graph.Adjncy[i];
                double neighborVertexWeight = graph.Eweights[i];

                // Updating values
                double neighbVerNewDistance = distances[currentVertex] + neighborVertexWeight; 
                if (distances[neighborVertex] > neighbVerNewDistance) {
                    distances[neighborVertex] = neighbVerNewDistance;
                    parents[neighborVertex] = currentVertex;
                    double neigbourEstimatedCost = estimateCost(neighborVertex);
                    pq.push({neighborVertex, neigbourEstimatedCost});
                }
            }
        }

        return ReturnCode::OK;
    }

    ReturnCode DijkstraAlgo::postProcess()
    {
        ReturnCode rc = ReturnCode::OK;

        data.shortestDistance = distances[data.destination];
        if (std::numeric_limits<double>::infinity() == data.shortestDistance)
        {
            return rc;
        }

        int currentVertex = data.destination;
        auto& path = data.shortestPath;
        path.clear();
        path.push_back(currentVertex);

        // Collecting optimal path
        while (currentVertex != data.source)
        {
            int parrentVertex = parents[currentVertex];
            path.push_back(parrentVertex);
            currentVertex = parrentVertex;
        }

        // Reversing path to get correct order
        std::reverse(path.begin(), path.end());

        rc = BaseAlgo::postProcess();

        return rc;
    }

    double DijkstraAlgo::estimateCost(int vertex)
    {
        return distances[vertex];
    }
} // namespace SP
