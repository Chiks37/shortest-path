#include "custom_launcher.hpp"
#include "networkit_launcher.hpp"
#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <omp.h>
#include <string>

void setWeightsToOne(crsGraph &graph)
{
    for (int i = 0; i < graph.nz; i++)
    {
        graph.Eweights[i] = 1;
    }
}

void testSearchingLongTimeProcessing(std::string graphFilename)
{
    const int source = 0;
    const auto algoId = SP::AlgoId::DIJKSTRA_SEQ;
    const std::string nodesMappingFilename = "none";

#pragma omp parallel default(none)                                             \
    firstprivate(source, graphFilename, algoId, nodesMappingFilename)          \
        shared(std::cout)
    {
        // Keep launcher state private for each worker thread.
        SP::CustomLauncher customLauncher(algoId, graphFilename,
                                          nodesMappingFilename);

#pragma omp for schedule(dynamic)
        for (int destination = 1; destination < 1971280; destination++)
        {
            customLauncher.execute(source, destination);
            const auto result = customLauncher.getResult();

            if (result.executionTimeMs > 1000)
            {
#pragma omp critical
                {
                    std::cout << "Route " << source << " -> " << destination
                              << " time is " << result.executionTimeMs << " ms"
                              << std::endl;
                }
            }
        }
    }
}

void testAllMethods(std::string graphFilename, std::string nodesMappingFilename)
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

    for (auto algoId : SP::algoIds)
    {
        SP::CustomLauncher customLauncher(algoId, graphFilename,
                                          nodesMappingFilename);
        customLauncher.execute(source, destination);
        customLauncher.printReport();
    }

    std::cout << std::endl
              << std::endl
              << "=== Networkit results ===" << std::endl
              << std::endl
              << std::endl;

    SP::NetworkitLauncher networkitLauncher(graphFilename,
                                            SP::AlgoId::DIJKSTRA_SEQ);
    networkitLauncher.execute(source, destination);
    networkitLauncher.printReport();
    networkitLauncher =
        SP::NetworkitLauncher(graphFilename, SP::AlgoId::ASTARG);
    networkitLauncher.execute(source, destination);
    networkitLauncher.printReport();
}

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

    testAllMethods(graphFilename, nodesMappingFilename);
    return 0;
}
