/**
 * @file astar_delta_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar delta-stepping algorithm class source file
 */

#include "astar_delta_bidir.hpp"
#include <cmath>
#include <limits>

namespace SP
{

void AStarDeltaBiDirAlgo::resetInternalData()
{
    DeltaSteppingBiDirAlgo::resetInternalData();

    cachedHsrc.assign(graph.V, 0.0);
    cachedHdst.assign(graph.V, 0.0);
    for (int v = 0; v < graph.V; ++v)
    {
        cachedHsrc[v] = heuristic(v, this->source);
        cachedHdst[v] = heuristic(v, this->destination);
    }

    auto pi = [&](int v) -> double
    { return 0.5 * (cachedHdst[v] - cachedHsrc[v]); };

    // Symmetric Johnson reduction needs |pi(u) - pi(v)| <= w(u,v) on every
    // edge in both directions (forward edge u->v requires pi(u)-pi(v) <= w;
    // backward edge u->v on the reverse graph requires pi(v)-pi(u) <= w).
    // Scale the cached heuristic by 1/maxRatio to enforce consistency while
    // preserving admissibility.
    double maxRatio = 0.0;
    auto accumulateRatio = [&](const crsGraph &g, bool reverseSign)
    {
        for (int u = 0; u < g.V; ++u)
        {
            for (int i = g.Xadj[u]; i < g.Xadj[u + 1]; ++i)
            {
                double w = g.Eweights[i];
                if (w <= 0.0)
                {
                    continue;
                }
                int v = g.Adjncy[i];
                double diff = reverseSign ? (pi(v) - pi(u)) : (pi(u) - pi(v));
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
    };
    accumulateRatio(graph, false);
    accumulateRatio(reverseGraph, true);

    if (maxRatio > 1.0)
    {
        double scale = 1.0 / maxRatio;
        for (int v = 0; v < graph.V; ++v)
        {
            cachedHsrc[v] *= scale;
            cachedHdst[v] *= scale;
        }
    }

    lightEdges.assign(graph.V, {});
    heavyEdges.assign(graph.V, {});
    lightEdgesBackward.assign(graph.V, {});
    heavyEdgesBackward.assign(graph.V, {});

    double sumReduced = 0.0;
    int countReduced = 0;
    for (int u = 0; u < graph.V; ++u)
    {
        for (int i = graph.Xadj[u]; i < graph.Xadj[u + 1]; ++i)
        {
            int v = graph.Adjncy[i];
            double wReduced = graph.Eweights[i] + pi(v) - pi(u);
            if (wReduced < 0.0)
            {
                wReduced = 0.0;
            }
            sumReduced += wReduced;
            ++countReduced;
        }
    }
    for (int u = 0; u < graph.V; ++u)
    {
        for (int i = reverseGraph.Xadj[u]; i < reverseGraph.Xadj[u + 1]; ++i)
        {
            int v = reverseGraph.Adjncy[i];
            double wReduced = reverseGraph.Eweights[i] + pi(u) - pi(v);
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
            double wReduced = graph.Eweights[i] + pi(v) - pi(u);
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
    for (int u = 0; u < graph.V; ++u)
    {
        for (int i = reverseGraph.Xadj[u]; i < reverseGraph.Xadj[u + 1]; ++i)
        {
            int v = reverseGraph.Adjncy[i];
            double wReduced = reverseGraph.Eweights[i] + pi(u) - pi(v);
            if (wReduced < 0.0)
            {
                wReduced = 0.0;
            }
            if (wReduced < delta)
            {
                lightEdgesBackward[u].push_back({v, wReduced});
            }
            else
            {
                heavyEdgesBackward[u].push_back({v, wReduced});
            }
        }
    }
}

ReturnCode AStarDeltaBiDirAlgo::buildResult()
{
    ReturnCode rc = DeltaSteppingBiDirAlgo::buildResult();
    if (ReturnCode::OK != rc)
    {
        return rc;
    }

    if (outData.shortestDistance != std::numeric_limits<double>::infinity())
    {
        double piSrc =
            0.5 * (cachedHdst[source] - cachedHsrc[source]);
        double piDst =
            0.5 * (cachedHdst[destination] - cachedHsrc[destination]);
        outData.shortestDistance += piSrc - piDst;
    }

    return rc;
}

} // namespace SP
