/**
 * @file launcher.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Base launcher class source file
 */
#include "launcher.hpp"

namespace SP
{

void Launcher::printReport() const
{
    // The default six significant digits print 24839.065 as 24839.1
    auto defaultPrecision = std::cout.precision(12);
    std::cout << "=== Report for " << getAlgoName() << " ===" << std::endl
              << "Distance: " << lastResult.shortestDistance << std::endl;
    std::cout.precision(defaultPrecision);
    std::cout << "Preprocess time: " << lastResult.preProcessTimeMs << " ms\n"
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
