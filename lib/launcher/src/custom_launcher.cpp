#include "custom_launcher.hpp"
#include <chrono>

namespace SP
{

std::shared_ptr<SP::BaseAlgo> CustomLauncher::createAlgoObject()
{
    switch (algoId)
    {
    case AlgoId::DIJKSTRA:
        return std::make_shared<SP::DijkstraAlgo>(graphFileName);
    case AlgoId::DIJKSTRA_BIDIR:
        return std::make_shared<SP::dijkstraBiDirAlgo>(graphFileName);
    case AlgoId::ASTARG:
        return std::make_shared<SP::AStarGAlgo>(graphFileName,
                                                nodesMappingFileName);
    case AlgoId::ALT:
        return std::make_shared<SP::ALTAlgo>(graphFileName);
    default:
        return std::make_shared<SP::DijkstraAlgo>(graphFileName);
    }
}

void CustomLauncher::execute(int source, int destination)
{
    auto algo = createAlgoObject();

    auto preStart = std::chrono::high_resolution_clock::now();
    algo->preProcess();
    algo->setSrcDest(source, destination);
    auto preEnd = std::chrono::high_resolution_clock::now();
    lastResult.preProccessTimeMs =
        std::chrono::duration<double, std::milli>(preEnd - preStart).count();

    auto computeStart = std::chrono::high_resolution_clock::now();
    algo->compute();
    auto computeEnd = std::chrono::high_resolution_clock::now();
    lastResult.executionTimeMs =
        std::chrono::duration<double, std::milli>(computeEnd - computeStart)
            .count();

    const auto &result = algo->getResult();
    lastResult.shortestPath = result.shortestPath;
    lastResult.shortestDistance = result.shortestDistance;
}

} // namespace SP
