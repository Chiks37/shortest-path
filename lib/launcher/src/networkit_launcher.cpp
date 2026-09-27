/**
 * @file networkit_launcher.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Networkit algorithms launcher class header file
 */
#include "networkit_launcher.hpp"
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
        astarHeuristics.assign(graph.upperNodeIdBound(), 0.0);
        return std::make_shared<NetworKit::AStar>(graph, astarHeuristics,
                                                  source, destination, true);
    }
    default:
        return std::make_shared<NetworKit::Dijkstra>(graph, source, true, false,
                                                     destination);
    }
}

void NetworkitLauncher::execute(int source, int destination)
{
    auto preStart = std::chrono::high_resolution_clock::now();
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
    auto preEnd = std::chrono::high_resolution_clock::now();
    lastResult.preProccessTimeMs =
        std::chrono::duration<double, std::milli>(preEnd - preStart).count();

    auto computeStart = std::chrono::high_resolution_clock::now();
    algo->run();
    auto computeEnd = std::chrono::high_resolution_clock::now();
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
