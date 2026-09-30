/**
 * @file networkit_launcher.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Networkit algorithms launcher class source file
 */
#include "networkit_launcher.hpp"
#include "coordinates.hpp"
#include <chrono>
#include <limits>

namespace SP
{

std::shared_ptr<NetworKit::Algorithm>
NetworkitLauncher::createAlgoObject(const NetworKit::Graph &graph, int source,
                                    int destination)
{
    // Set reconstruct path = true, sort vertices = false
    switch (algoId)
    {
    case AlgoId::DIJKSTRA_SEQ:
        return std::make_shared<NetworKit::Dijkstra>(graph, source, true, false,
                                                     destination);
    case AlgoId::ASTARG:
    {
        // The same heuristic as our A*G algorithms. It depends on the
        // destination, so execute fills it in the timed part of the query.
        int vertexCount = static_cast<int>(graph.upperNodeIdBound());
        if (astarCoordinates.load(nodesMappingFileName, vertexCount) !=
            ReturnCode::OK)
        {
            return nullptr;
        }
        graph.forEdges(
            [&](NetworKit::node u, NetworKit::node v, NetworKit::edgeweight w)
            { astarCoordinates.fitEdge(u, v, w); });

        astarHeuristics.assign(vertexCount, 0.0);
        return std::make_shared<NetworKit::AStar>(graph, astarHeuristics,
                                                  source, destination, true);
    }
    case AlgoId::COUNT:
        break;
    }
    return nullptr;
}

void NetworkitLauncher::execute(int source, int destination)
{
    auto preStart = std::chrono::steady_clock::now();
    NetworKit::MTXGraphReader reader;
    auto graph = reader.read(graphFileName);

    lastResult = LauncherResult{};
    lastResult.shortestDistance = std::numeric_limits<double>::quiet_NaN();
    if (source < 0 || destination < 0 || !graph.hasNode(source) ||
        !graph.hasNode(destination))
    {
        return;
    }

    auto algo = createAlgoObject(graph, source, destination);
    if (algo == nullptr)
    {
        return;
    }
    auto preEnd = std::chrono::steady_clock::now();
    lastResult.preProcessTimeMs =
        std::chrono::duration<double, std::milli>(preEnd - preStart).count();

    auto computeStart = std::chrono::steady_clock::now();
    if (AlgoId::ASTARG == algoId)
    {
        for (int v = 0; v < static_cast<int>(astarHeuristics.size()); ++v)
        {
            astarHeuristics[v] = astarCoordinates.lowerBound(v, destination);
        }
    }
    algo->run();
    auto computeEnd = std::chrono::steady_clock::now();
    lastResult.executionTimeMs =
        std::chrono::duration<double, std::milli>(computeEnd - computeStart)
            .count();

    std::vector<int> path;
    double distance = std::numeric_limits<double>::quiet_NaN();
    switch (algoId)
    {
    case AlgoId::DIJKSTRA_SEQ:
        distance = std::static_pointer_cast<NetworKit::Dijkstra>(algo)
                       ->getDistances()[destination];
        break;
    case AlgoId::ASTARG:
        distance =
            std::static_pointer_cast<NetworKit::AStar>(algo)->getDistance();
        break;
    default:
        break;
    }

    // networkit marks an unreachable destination with the largest double
    if (distance == std::numeric_limits<double>::max())
    {
        distance = std::numeric_limits<double>::infinity();
    }
    else if (source == destination)
    {
        path = {source};
    }
    else if (AlgoId::DIJKSTRA_SEQ == algoId)
    {
        auto p = std::static_pointer_cast<NetworKit::Dijkstra>(algo)->getPath(
            destination);
        path.assign(p.begin(), p.end());
    }
    else if (AlgoId::ASTARG == algoId)
    {
        // A* leaves the source and the destination out of the path
        auto p = std::static_pointer_cast<NetworKit::AStar>(algo)->getPath();
        path.push_back(source);
        path.insert(path.end(), p.begin(), p.end());
        path.push_back(destination);
    }
    lastResult.shortestPath = path;
    lastResult.shortestDistance = distance;
}

} // namespace SP
