/**
 * @file astar_delta.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar delta-stepping algorithm class source file
 */

#include "astar_delta.hpp"
#include <cmath>
#include <limits>

namespace SP
{

void AStarDeltaAlgo::resetInternalData()
{
    DeltaSteppingAlgo::resetInternalData();

    cachedH.assign(graph.V, 0.0);
    for (int v = 0; v < graph.V; ++v)
    {
        cachedH[v] = heuristic(v);
    }

    // Heuristic may be admissible but not strictly consistent (e.g. projected
    // Euclidean on road graphs where the projection isn't exactly isometric).
    // Shrink h by 1/maxRatio if any edge violates h(u) - h(v) <= w(u,v); this
    // preserves admissibility (h stays a lower bound) and forces consistency,
    // so reduced weights are guaranteed non-negative without clamping. For a
    // consistent heuristic (e.g. ALT) maxRatio <= 1 and nothing changes.
    double maxRatio = 0.0;
    for (int u = 0; u < graph.V; ++u)
    {
        for (int i = graph.Xadj[u]; i < graph.Xadj[u + 1]; ++i)
        {
            double w = graph.Eweights[i];
            if (w <= 0.0)
            {
                continue;
            }
            int v = graph.Adjncy[i];
            double diff = cachedH[u] - cachedH[v];
            if (diff > 0.0)
            {
                double ratio = diff / w;
                if (ratio > maxRatio)
                {
                    maxRatio = ratio;
                }
            }
        }
    }
    if (maxRatio > 1.0)
    {
        double scale = 1.0 / maxRatio;
        for (int v = 0; v < graph.V; ++v)
        {
            cachedH[v] *= scale;
        }
    }

    // Johnson potential: replace w(u,v) with w'(u,v) = w(u,v) + h(v) - h(u).
    // For a consistent heuristic w' >= 0, and g'(v) = g(v) + h(v) - h(s) is a
    // true path-sum on the reduced graph. Running delta-stepping on w' is
    // equivalent to A* on the original weights while keeping the bucket
    // monotonicity invariant intact.
    lightEdges.assign(graph.V, {});
    heavyEdges.assign(graph.V, {});

    double sumReduced = 0.0;
    int countReduced = 0;
    for (int u = 0; u < graph.V; ++u)
    {
        for (int i = graph.Xadj[u]; i < graph.Xadj[u + 1]; ++i)
        {
            int v = graph.Adjncy[i];
            double wReduced = graph.Eweights[i] + cachedH[v] - cachedH[u];
            if (wReduced < 0.0)
            {
                wReduced = 0.0;
            }
            sumReduced += wReduced;
            ++countReduced;
        }
    }

    delta = (countReduced > 0) ? (sumReduced / countReduced) : 1.0;
    if (!std::isfinite(delta) || delta <= 0.0)
    {
        delta = 1.0;
    }

    for (int u = 0; u < graph.V; ++u)
    {
        for (int i = graph.Xadj[u]; i < graph.Xadj[u + 1]; ++i)
        {
            int v = graph.Adjncy[i];
            double wReduced = graph.Eweights[i] + cachedH[v] - cachedH[u];
            if (wReduced < 0.0)
            {
                wReduced = 0.0;
            }
            if (wReduced < delta)
            {
                lightEdges[u].push_back({v, wReduced});
            }
            else
            {
                heavyEdges[u].push_back({v, wReduced});
            }
        }
    }
}

double AStarDeltaAlgo::estimateCost(int vertex) { return distances[vertex]; }

ReturnCode AStarDeltaAlgo::buildResult()
{
    ReturnCode rc = DijkstraAlgo::buildResult();
    if (ReturnCode::OK != rc)
    {
        return rc;
    }

    if (outData.shortestDistance != std::numeric_limits<double>::infinity())
    {
        outData.shortestDistance += cachedH[source] - cachedH[destination];
    }

    return rc;
}

} // namespace SP
