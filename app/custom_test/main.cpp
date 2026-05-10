#include "custom_launcher.hpp"
#include "networkit_launcher.hpp"
#include <iostream>
#include <omp.h>
#include <string>

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

    for (auto algoId : SP::CustomLauncher::algoIds)
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

    SP::NetworkitLauncher networkitLauncher(
        graphFilename, SP::NetworkitLauncher::AlgoId::DIJKSTRA_SEQ);
    networkitLauncher.execute(source, destination);
    networkitLauncher.printReport();
    networkitLauncher = SP::NetworkitLauncher(
        graphFilename, SP::NetworkitLauncher::AlgoId::ASTARG);
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
