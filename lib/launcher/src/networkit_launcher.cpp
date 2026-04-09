#include "networkit_launcher.hpp"
#include <chrono>

namespace SP
{

std::shared_ptr<NetworKit::Algorithm>
NetworkitLauncher::createAlgoObject(const NetworKit::Graph &graph, int source,
                                    int destination)
{
    // Set reconstruct path = true, sort vertices = false
    switch (algoId)
    {
    case AlgoId::DIJKSTRA:
        return std::make_shared<NetworKit::Dijkstra>(graph, source, true, false,
                                                     destination);
    case AlgoId::ASTARG:
    {
        astarHeuristics.assign(graph.upperNodeIdBound(), 0.0);
        return std::make_shared<NetworKit::AStar>(graph, astarHeuristics,
                                                  source, destination, true);
    }
    // case AlgoId::ALT:
    //     return std::make_shared<SP::ALTAlgo>(graphFileName);
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
    double distance;
    switch (algoId)
    {
    case AlgoId::DIJKSTRA:
    {
        auto p = std::static_pointer_cast<NetworKit::Dijkstra>(algo)->getPath(
            destination);
        path.assign(p.begin(), p.end());
        distance = static_cast<double>(
            std::static_pointer_cast<NetworKit::Dijkstra>(algo)
                ->getDistances()[destination]);
        break;
    }
    case AlgoId::ASTARG:
    {
        auto p = std::static_pointer_cast<NetworKit::AStar>(algo)->getPath();
        path.assign(p.begin(), p.end());
        distance = static_cast<double>(
            std::static_pointer_cast<NetworKit::AStar>(algo)->getDistance());
        break;
    }
    }
    lastResult.shortestPath = path;
    lastResult.shortestDistance = distance;
}

} // namespace SP
