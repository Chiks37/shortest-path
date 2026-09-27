/**
 * @file landmarks.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Landmarks distance tables for ALT heuristic source file
 */

#include "landmarks.hpp"
#include "general.hpp"
#include "graph_transpose.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace SP
{

ReturnCode Landmarks::build(const crsGraph &graph, int landmarksCount)
{
    if (graph.V <= 0 || landmarksCount <= 0)
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    const int V = graph.V;
    const double inf = std::numeric_limits<double>::infinity();

    landmarks.clear();
    stride = static_cast<std::size_t>(landmarksCount);
    symmetric = mm_is_symmetric(graph.matcode);
    distFromLandmarks.assign(static_cast<std::size_t>(V) * stride, inf);
    distToLandmarks.clear();

    // The first landmark is the vertex farthest from vertex 0, every next one
    // is the vertex farthest from all landmarks chosen so far. Unreachable
    // vertices are never picked, otherwise the landmarks end up in tiny
    // disconnected components.
    std::vector<double> distances;
    computeDistances(graph, 0, distances);
    int nextLandmark = farthestVertex(distances, 0);

    std::vector<double> distToChosen(V, inf);
    while (nextLandmark != -1 && landmarks.size() < stride)
    {
        std::size_t i = landmarks.size();
        landmarks.push_back(nextLandmark);
        computeDistances(graph, nextLandmark, distances);

        for (int vertex = 0; vertex < V; ++vertex)
        {
            distFromLandmarks[static_cast<std::size_t>(vertex) * stride + i] =
                distances[vertex];
            distToChosen[vertex] =
                std::min(distToChosen[vertex], distances[vertex]);
        }

        nextLandmark = farthestVertex(distToChosen, -1);
    }

    if (symmetric)
    {
        return ReturnCode::OK;
    }

    // In a directed graph the distance to a landmark is the distance from it
    // in the transposed graph
    std::vector<int> reverseXadj;
    std::vector<int> reverseAdjncy;
    std::vector<double> reverseEweights;
    crsGraph reverseGraph{};
    transposeGraph(graph, reverseXadj, reverseAdjncy, reverseEweights,
                   reverseGraph);

    distToLandmarks.assign(static_cast<std::size_t>(V) * stride, inf);
    for (std::size_t i = 0; i < landmarks.size(); ++i)
    {
        computeDistances(reverseGraph, landmarks[i], distances);
        for (int vertex = 0; vertex < V; ++vertex)
        {
            distToLandmarks[static_cast<std::size_t>(vertex) * stride + i] =
                distances[vertex];
        }
    }

    return ReturnCode::OK;
}

double Landmarks::lowerBound(int from, int to) const
{
    const double *landmarkToFrom = row(distFromLandmarks, from);
    const double *landmarkToTo = row(distFromLandmarks, to);
    const double *fromToLandmark =
        symmetric ? landmarkToFrom : row(distToLandmarks, from);
    const double *toToLandmark =
        symmetric ? landmarkToTo : row(distToLandmarks, to);

    // A term with an unreachable vertex is infinite or NaN and bounds nothing
    // useful, so it is skipped. The result stays finite, which the algorithms
    // working on reduced edge weights rely on.
    double maxH = 0.0;
    for (std::size_t i = 0; i < landmarks.size(); ++i)
    {
        // dist(from, to) >= dist(from, L) - dist(to, L)
        double hTo = fromToLandmark[i] - toToLandmark[i];
        // dist(from, to) >= dist(L, to) - dist(L, from)
        double hFrom = landmarkToTo[i] - landmarkToFrom[i];

        if (std::isfinite(hTo) && hTo > maxH)
        {
            maxH = hTo;
        }
        if (std::isfinite(hFrom) && hFrom > maxH)
        {
            maxH = hFrom;
        }
    }

    return maxH;
}

void Landmarks::computeDistances(const crsGraph &graph, int source,
                                 std::vector<double> &distances)
{
    distances.assign(graph.V, std::numeric_limits<double>::infinity());
    distances[source] = 0.0;

    std::priority_queue<edge, std::vector<edge>, compareEdges> pq;
    pq.push({source, 0.0});

    while (!pq.empty())
    {
        int currentVertex = pq.top().vertex;
        double currentDistance = pq.top().val;
        pq.pop();

        if (currentDistance > distances[currentVertex])
        {
            continue;
        }

        for (int i = graph.Xadj[currentVertex];
             i < graph.Xadj[currentVertex + 1]; ++i)
        {
            int neighborVertex = graph.Adjncy[i];
            double newDistance = currentDistance + graph.Eweights[i];
            if (newDistance < distances[neighborVertex])
            {
                distances[neighborVertex] = newDistance;
                pq.push({neighborVertex, newDistance});
            }
        }
    }
}

int Landmarks::farthestVertex(const std::vector<double> &distances,
                              int fallback)
{
    int farthest = fallback;
    double maxDistance = 0.0;

    for (int vertex = 0; vertex < static_cast<int>(distances.size()); ++vertex)
    {
        if (std::isfinite(distances[vertex]) && distances[vertex] > maxDistance)
        {
            maxDistance = distances[vertex];
            farthest = vertex;
        }
    }

    return farthest;
}

} // namespace SP
