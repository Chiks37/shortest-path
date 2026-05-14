#include "custom_launcher.hpp"
#include "gapbs_launcher.hpp"
#include "networkit_launcher.hpp"
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <string>
#include <vector>

namespace
{

constexpr int kRunsCount = 5;

struct BenchmarkStat
{
    std::string algoName;
    double avgPreTimeMs;
    double avgExecTimeMs;
    double distance;
};

// Re-build launcher each iteration: every Launcher::execute() rebuilds its
// algo/graph state from scratch, so reusing a launcher would still time the
// same work, but a fresh object guarantees no leftover caches between runs.
template <typename FactoryT>
BenchmarkStat measure(const std::string &name, int source, int destination,
                      FactoryT factory)
{
    double preSum = 0.0;
    double execSum = 0.0;
    double lastDistance = 0.0;
    for (int i = 0; i < kRunsCount; ++i)
    {
        auto launcher = factory();
        launcher.execute(source, destination);
        const auto &res = launcher.getResult();
        preSum += res.preProccessTimeMs;
        execSum += res.executionTimeMs;
        lastDistance = res.shortestDistance;
    }
    return {name, preSum / kRunsCount, execSum / kRunsCount, lastDistance};
}

void printStats(const std::vector<BenchmarkStat> &stats)
{
    constexpr int nameW = 28;
    constexpr int colW = 20;
    std::cout << std::left << std::setw(nameW) << "Algorithm" << std::right
              << std::setw(colW) << "Avg preproc, ms" << std::setw(colW)
              << "Avg execute, ms" << std::setw(colW) << "Distance"
              << std::endl;
    std::cout << std::string(nameW + colW * 3, '-') << std::endl;
    for (const auto &s : stats)
    {
        std::cout << std::left << std::setw(nameW) << s.algoName << std::right
                  << std::fixed << std::setprecision(3) << std::setw(colW)
                  << s.avgPreTimeMs << std::setw(colW) << s.avgExecTimeMs
                  << std::setw(colW) << s.distance << std::endl;
    }
}

void runBenchmark(const std::string &graphFilename,
                  const std::string &nodesMappingFilename)
{
#pragma omp parallel
    {
        int id = omp_get_thread_num();

#pragma omp critical
        {
            std::cout << "Thread " << id << std::endl;
        }
    }

    int source = 0;
    std::cout << "Source (start from 1) >> ";
    std::cin >> source;
    source--;

    int destination = 0;
    std::cout << "Destination (start from 1) >> ";
    std::cin >> destination;
    destination--;

    std::vector<BenchmarkStat> stats;
    stats.reserve(SP::CustomLauncher::algoIds.size() + 2);

    for (auto algoId : SP::CustomLauncher::algoIds)
    {
        const auto &name = SP::CustomLauncher::algoNames.at(algoId);
        std::cout << "Running " << name << " (" << kRunsCount << " runs)..."
                  << std::endl;
        stats.push_back(measure(name, source, destination, [&]() {
            return SP::CustomLauncher(algoId, graphFilename,
                                      nodesMappingFilename);
        }));
    }

    {
        const auto &name = SP::NetworkitLauncher::algoNames.at(
            SP::NetworkitLauncher::AlgoId::DIJKSTRA_SEQ);
        std::cout << "Running " << name << " (" << kRunsCount << " runs)..."
                  << std::endl;
        stats.push_back(measure(name, source, destination, [&]() {
            return SP::NetworkitLauncher(
                graphFilename, SP::NetworkitLauncher::AlgoId::DIJKSTRA_SEQ);
        }));
    }

    {
        const auto &name = SP::GapbsLauncher::algoNames.at(
            SP::GapbsLauncher::AlgoId::DIJKSTRA_SEQ);
        std::cout << "Running " << name << " (" << kRunsCount << " runs)..."
                  << std::endl;
        stats.push_back(measure(name, source, destination, [&]() {
            return SP::GapbsLauncher(graphFilename,
                                     SP::GapbsLauncher::AlgoId::DIJKSTRA_SEQ);
        }));
    }

    std::cout << std::endl
              << "=== Benchmark results (avg over " << kRunsCount
              << " runs) ===" << std::endl;
    printStats(stats);
}

} // namespace

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <.mtx file>" << std::endl;
        return 1;
    }

    std::string graphFilename(argv[1]);
    std::string nodesMappingFilename =
        graphFilename.substr(0, graphFilename.find_last_of('.')) +
        "_nodes_mapping.txt";

    runBenchmark(graphFilename, nodesMappingFilename);
    return 0;
}
