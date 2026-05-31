/**
 * @file abstract_dijkstra_seq_mutable.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra algorithm sequential version with mutable priority queue
 *        (decrease_key) class source file
 */

#include "abstract_dijkstra_seq_mutable.hpp"

#include <boost/heap/binomial_heap.hpp>
#include <boost/heap/d_ary_heap.hpp>
#include <boost/heap/pairing_heap.hpp>

namespace SP
{
template <typename PQ>
ReturnCode AbstractDijkstraSeqMutableAlgo<PQ>::runSearch()
{
    while (!pq.empty())
    {
        int currentVertex = pq.top().vertex;
        pq.pop();
        handleValid[currentVertex] = false;

        if (completeCondition(currentVertex))
        {
            break;
        }

        for (int i = graph.Xadj[currentVertex];
             i < graph.Xadj[currentVertex + 1]; i++)
        {
            int neighborVertex = graph.Adjncy[i];
            double neighborVertexWeight = graph.Eweights[i];

            double neighbVerNewDistance =
                distances[currentVertex] + neighborVertexWeight;
            if (distances[neighborVertex] > neighbVerNewDistance)
            {
                distances[neighborVertex] = neighbVerNewDistance;
                parents[neighborVertex] = currentVertex;
                double neighbourEstimatedCost = estimateCost(neighborVertex);
                edge newEdge = {neighborVertex, neighbourEstimatedCost};
                if (handleValid[neighborVertex])
                {
                    pq.increase(handles[neighborVertex], newEdge);
                }
                else
                {
                    handles[neighborVertex] = pq.push(newEdge);
                    handleValid[neighborVertex] = true;
                }
            }
        }
    }

    return ReturnCode::OK;
}

template <typename PQ>
void AbstractDijkstraSeqMutableAlgo<PQ>::initInternalData()
{
    AbstractDijkstraAlgo::initInternalData();

    pq.clear();
    handles.resize(graph.V);
    handleValid.assign(graph.V, false);
}

template <typename PQ>
void AbstractDijkstraSeqMutableAlgo<PQ>::resetInternalData()
{
    AbstractDijkstraAlgo::resetInternalData();

    double sourceEstimatedCost = estimateCost(this->source);
    handles[this->source] = pq.push({this->source, sourceEstimatedCost});
    handleValid[this->source] = true;
}

template class AbstractDijkstraSeqMutableAlgo<
    boost::heap::pairing_heap<edge, boost::heap::compare<compareEdges>>>;
template class AbstractDijkstraSeqMutableAlgo<
    boost::heap::binomial_heap<edge, boost::heap::compare<compareEdges>>>;
template class AbstractDijkstraSeqMutableAlgo<boost::heap::d_ary_heap<
    edge, boost::heap::compare<compareEdges>, boost::heap::arity<4>,
    boost::heap::mutable_<true>>>;

} // namespace SP
