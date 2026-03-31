#include "launcher.hpp"

namespace SP
{

void Launcher::printReport() const
{
    std::cout << "=== Report for " << algoNames.at(algoId)
              << " ===" << std::endl
              << "Distance: " << lastResult.shortestDistance << std::endl
              << "Prepoccess time: " << lastResult.preProccessTimeMs << " ms\n"
              << "Execution time: " << lastResult.executionTimeMs << " ms\n"
              << std::endl
              << "Path vertices count: " << lastResult.shortestPath.size()
              << std::endl
              << "Shortest path: ";

    for (auto vertex : lastResult.shortestPath)
    {
        std::cout << vertex + 1 << ' ';
    }

    std::cout << "\n===========================\n" << std::endl;
}

} // namespace SP
