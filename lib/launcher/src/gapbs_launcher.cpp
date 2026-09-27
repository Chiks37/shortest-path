/**
 * @file gapbs_launcher.cpp
 * @author tarakanov.2004@mail.ru
 * @brief GAPBS algorithms launcher class implementation
 *
 * NOTE: This file is intentionally excluded from sp_lib (see CMakeLists.txt
 * FILTER). It is compiled via cmake/gapbs/ into cache/deps/gapbs/ so that
 * GAPBS headers are not pulled in as a global dependency.
 *
 * We use GAPBS's Reader for .mtx parsing (NodeWeight / EdgePair types) and
 * build a plain adjacency list for Dijkstra. BuilderBase is intentionally
 * avoided: its Generator::InsertWeights calls make_unsigned<WeightT> which
 * fails to compile with a floating-point WeightT.
 */
#include "gapbs_launcher.hpp"

#include <graph.h>
#include <pvector.h>
#include <reader.h>

#include <chrono>
#include <fstream>
#include <limits>
#include <queue>
#include <sstream>
#include <vector>

namespace SP
{

using GNodeID = int32_t;
using GWeightT = double;
using GWNode = NodeWeight<GNodeID, GWeightT>;
using GEdgeList = pvector<EdgePair<GNodeID, GWNode>>;
using GAdjList = std::vector<std::vector<GWNode>>;

static constexpr GWeightT kGDistInf = std::numeric_limits<GWeightT>::infinity();

// Isolated vertices at the end of the numbering have no edges, so the vertex
// count is taken from the size line of the .mtx file
static GNodeID readVertexCount(const std::string &fileName)
{
    std::ifstream file(fileName);
    std::string line;
    while (std::getline(file, line) && (line.empty() || line[0] == '%'))
    {
    }
    GNodeID rows = 0;
    std::istringstream(line) >> rows;
    return rows;
}

static GAdjList buildAdjList(const GEdgeList &el, GNodeID vertexCount)
{
    GNodeID maxNode = vertexCount - 1;
    for (const auto &e : el)
        maxNode = std::max(maxNode, std::max(e.u, e.v.v));

    GAdjList adj(static_cast<size_t>(maxNode) + 1);
    for (const auto &e : el)
        adj[e.u].push_back(e.v);

    return adj;
}

static std::vector<GWeightT> dijkstra(const GAdjList &adj, GNodeID source,
                                      GNodeID destination)
{
    std::vector<GWeightT> dist(adj.size(), kGDistInf);
    dist[source] = 0;

    using Entry = std::pair<GWeightT, GNodeID>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;
    pq.push({0.0, source});

    while (!pq.empty())
    {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u])
            continue;
        if (u == destination)
            break;
        for (const GWNode &wn : adj[static_cast<size_t>(u)])
        {
            GWeightT nd = d + wn.w;
            if (nd < dist[wn.v])
            {
                dist[wn.v] = nd;
                pq.push({nd, wn.v});
            }
        }
    }
    return dist;
}

void GapbsLauncher::execute(int source, int destination)
{
    auto preStart = std::chrono::high_resolution_clock::now();
    Reader<GNodeID, GWNode, GWeightT, false> reader(graphFileName);
    bool needsWeights = true;
    GEdgeList el = reader.ReadFile(needsWeights);
    GAdjList adj = buildAdjList(el, readVertexCount(graphFileName));
    auto preEnd = std::chrono::high_resolution_clock::now();

    lastResult.shortestPath = {};
    lastResult.shortestDistance = std::numeric_limits<double>::quiet_NaN();
    lastResult.preProccessTimeMs =
        std::chrono::duration<double, std::milli>(preEnd - preStart).count();
    lastResult.executionTimeMs = 0.0;
    GNodeID vertexCount = static_cast<GNodeID>(adj.size());
    if (source < 0 || source >= vertexCount || destination < 0 ||
        destination >= vertexCount)
    {
        return;
    }

    auto computeStart = std::chrono::high_resolution_clock::now();
    std::vector<GWeightT> dist = dijkstra(adj, source, destination);
    auto computeEnd = std::chrono::high_resolution_clock::now();

    lastResult.shortestDistance = dist[destination];
    lastResult.executionTimeMs =
        std::chrono::duration<double, std::milli>(computeEnd - computeStart)
            .count();
}

} // namespace SP
