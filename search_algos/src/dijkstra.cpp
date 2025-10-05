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

        if (ReturnCode::OK == rc)
        {
            // All vertex weights but source weight are endless untill they relaxed 
            distances.assign(graph.V, std::numeric_limits<double>::infinity());
            distances[data.source] = 0;

            // Parents are unknown for now
            parents.assign(graph.V, -1);

            // Source vertex is first to consider
            pq.push({data.source, 0.0});
        }

        return rc;
    }

    ReturnCode DijkstraAlgo::process()
    {
        while (!pq.empty())
        {

            // Getting current vertex info 
            int currentVertex = pq.top().vertex;
            double currentVertexDistance = pq.top().val;
            pq.pop();

            // Skip if not the shortest path to currentVertex
            if (currentVertexDistance > distances[currentVertex]) continue;

            // Researching neighbors
            for (int i = graph.Xadj[currentVertex]; i < graph.Xadj[currentVertex + 1]; i++) {
                int neighborVertex = graph.Adjncy[i];
                double neighborVertexWeight = graph.Eweights[i];

                // Updating values
                if (distances[neighborVertex] > distances[currentVertex] + neighborVertexWeight) {
                    distances[neighborVertex] = distances[currentVertex] + neighborVertexWeight;
                    parents[neighborVertex] = currentVertex;
                    pq.push({neighborVertex, distances[neighborVertex]});
                }
            }
        }

        return ReturnCode::OK;
    }

    ReturnCode DijkstraAlgo::postProcess()
    {

        data.shortestDistance = distances[data.destination];

        int currentVertex = data.destination;
        auto& path = data.shortestPath;
        path.push_back(currentVertex);

        // Collecting optimal path
        while (currentVertex != data.source)
        {
            int parrentVertex = parents[currentVertex];
            path.push_back(parrentVertex);
            currentVertex = parrentVertex;
        }

        // Reversing path to get correct order
        reverse(path.begin(), path.end());

        BaseAlgo::postProcess();

        return ReturnCode::OK;
    }

} // namespace SP
