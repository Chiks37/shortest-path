#include "dijkstra_seq.hpp"
/**
 * @file dijkstra_seq.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm sequential version class source file
 */

namespace SP
{
ReturnCode DijkstraSeqAlgo::runSearch()
{
    while (!pq.empty())
    {

        // Getting current vertex info from priority queue
        int currentVertex = pq.top().vertex;
        double curVerPoppedEstCost = pq.top().val;
        pq.pop();

        // Skip if there is already shorter path to current vertex than we
        // trying to calculate
        double curVerStoredEstCost = estimateCost(currentVertex);
        if (curVerPoppedEstCost > curVerStoredEstCost)
            continue;

        // Early exit condition: we already reached destination vertex
        if (completeCondition(currentVertex))
        {
            break;
        }

        // Researching neighbors
        for (int i = graph.Xadj[currentVertex];
             i < graph.Xadj[currentVertex + 1]; i++)
        {
            int neighborVertex = graph.Adjncy[i];
            double neighborVertexWeight = graph.Eweights[i];

            // Updating values
            double neighbVerNewDistance =
                distances[currentVertex] + neighborVertexWeight;
            if (distances[neighborVertex] > neighbVerNewDistance)
            {
                distances[neighborVertex] = neighbVerNewDistance;
                parents[neighborVertex] = currentVertex;
                double neigbourEstimatedCost = estimateCost(neighborVertex);
                pq.push({neighborVertex, neigbourEstimatedCost});
            }
        }
    }

    return ReturnCode::OK;
}

void DijkstraSeqAlgo::initInternalData()
{
    DijkstraAlgo::initInternalData();

    // Clear the pq
    pq = std::priority_queue<edge, std::vector<edge>, compareEdges>();
}

void DijkstraSeqAlgo::resetInternalData()
{
    DijkstraAlgo::resetInternalData();

    double sourceEstimatedCost = estimateCost(this->source);
    pq.push({this->source, sourceEstimatedCost});
}

} // namespace SP