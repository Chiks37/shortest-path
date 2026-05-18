#include "custom_launcher.hpp"
#include "gapbs_launcher.hpp"
#include "networkit_launcher.hpp"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <sstream>
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
    bool failed;
    std::string errorMsg;
};

template <typename FactoryT>
BenchmarkStat measure(const std::string &name, int source, int destination,
                      FactoryT factory)
{
    double preSum = 0.0;
    double execSum = 0.0;
    double lastDistance = 0.0;
    try
    {
        for (int i = 0; i < kRunsCount; ++i)
        {
            auto launcher = factory();
            launcher.execute(source, destination);
            const auto &res = launcher.getResult();
            preSum += res.preProccessTimeMs;
            execSum += res.executionTimeMs;
            lastDistance = res.shortestDistance;
        }
    }
    catch (const std::exception &e)
    {
        return {name, 0.0, 0.0, 0.0, true, e.what()};
    }
    return {
        name, preSum / kRunsCount, execSum / kRunsCount, lastDistance, false,
        ""};
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
        std::cout << std::left << std::setw(nameW) << s.algoName << std::right;
        if (s.failed)
        {
            std::cout << "  FAILED: " << s.errorMsg << std::endl;
            continue;
        }
        std::cout << std::fixed << std::setprecision(3) << std::setw(colW)
                  << s.avgPreTimeMs << std::setw(colW) << s.avgExecTimeMs
                  << std::setw(colW) << s.distance << std::endl;
    }
}

std::vector<int> parseThreadList(const std::string &arg)
{
    std::vector<int> result;
    std::stringstream ss(arg);
    std::string item;
    while (std::getline(ss, item, ','))
    {
        if (!item.empty())
            result.push_back(std::atoi(item.c_str()));
    }
    return result;
}

void runForThreads(int numThreads, const std::string &graphFilename,
                   const std::string &nodesMappingFilename, int source,
                   int destination)
{
    omp_set_num_threads(numThreads);

    std::cout << std::endl
              << "########################################" << std::endl
              << "# Threads: " << numThreads << std::endl
              << "########################################" << std::endl;

    std::vector<BenchmarkStat> stats;
    stats.reserve(SP::CustomLauncher::algoIds.size() + 2);

    for (auto algoId : SP::CustomLauncher::algoIds)
    {
        const auto &name = SP::CustomLauncher::algoNames.at(algoId);
        std::cout << "Running " << name << " (" << kRunsCount << " runs)..."
                  << std::endl;
        stats.push_back(measure(name, source, destination,
                                [&]() {
                                    return SP::CustomLauncher(
                                        algoId, graphFilename,
                                        nodesMappingFilename);
                                }));
    }

    {
        const auto &name = SP::NetworkitLauncher::algoNames.at(
            SP::NetworkitLauncher::AlgoId::DIJKSTRA_SEQ);
        std::cout << "Running " << name << " (" << kRunsCount << " runs)..."
                  << std::endl;
        stats.push_back(measure(
            name, source, destination,
            [&]()
            {
                return SP::NetworkitLauncher(
                    graphFilename, SP::NetworkitLauncher::AlgoId::DIJKSTRA_SEQ);
            }));
    }

    {
        const auto &name = SP::GapbsLauncher::algoNames.at(
            SP::GapbsLauncher::AlgoId::DIJKSTRA_SEQ);
        std::cout << "Running " << name << " (" << kRunsCount << " runs)..."
                  << std::endl;
        stats.push_back(measure(
            name, source, destination,
            [&]()
            {
                return SP::GapbsLauncher(
                    graphFilename, SP::GapbsLauncher::AlgoId::DIJKSTRA_SEQ);
            }));
    }

    std::cout << std::endl
              << "=== Results for " << numThreads << " thread(s) (avg over "
              << kRunsCount << " runs) ===" << std::endl;
    printStats(stats);
}

} // namespace

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <.mtx file> <source> <destination> "
                     "[-t|--threads N[,N...]]"
                  << std::endl;
        return 1;
    }

    std::string graphFilename(argv[1]);
    int source = std::atoi(argv[2]) - 1;
    int destination = std::atoi(argv[3]) - 1;
    std::vector<int> threadCounts{omp_get_max_threads()};

    for (int i = 4; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if ((arg == "-t" || arg == "--threads") && i + 1 < argc)
        {
            threadCounts = parseThreadList(argv[++i]);
        }
    }

    if (threadCounts.empty())
    {
        std::cerr << "No valid thread counts parsed." << std::endl;
        return 1;
    }

    std::string nodesMappingFilename =
        graphFilename.substr(0, graphFilename.find_last_of('.')) +
        "_nodes_mapping.txt";

    std::cout << "=== Configuration ===" << std::endl;
    std::cout << "Graph: " << graphFilename << std::endl;
    std::cout << "Route: " << (source + 1) << " -> " << (destination + 1)
              << std::endl;
    std::cout << "Thread sweep:";
    for (int n : threadCounts)
        std::cout << ' ' << n;
    std::cout << std::endl;

    for (int n : threadCounts)
        runForThreads(n, graphFilename, nodesMappingFilename, source,
                      destination);

    return 0;
}
