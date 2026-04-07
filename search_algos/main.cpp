#include "custom_launcher.hpp"
#include "networkit_launcher.hpp"
#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <string>

void setWeightsToOne(crsGraph &graph)
{
    for (int i = 0; i < graph.nz; i++)
    {
        graph.Eweights[i] = 1;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <.mtx file>" << std::endl;
        return 1;
    }

    int source = 0;
    std::cout << "Source (start from 1) >> ";
    std::cin >> source;
    source--;

    int destination = 0;
    std::cout << "Destination (start from 1) >> ";
    std::cin >> destination;
    destination--;

    std::string graphFilename(argv[1]);
    std::string nodesMappingFilename = graphFilename.substr(0, graphFilename.find_last_of('.')) + "_nodes_mapping.txt";

    for (auto algoId : SP::algoIds)
    {
        SP::CustomLauncher customLauncher(algoId, graphFilename, nodesMappingFilename);
        customLauncher.execute(source, destination);
        customLauncher.printReport();
    }

    std::cout << std::endl
              << std::endl
              << "=== Networkit results ===" << std::endl
              << std::endl
              << std::endl;

    SP::NetworkitLauncher networkitLauncher(graphFilename,
                                            SP::AlgoId::DIJKSTRA);
    networkitLauncher.execute(source, destination);
    networkitLauncher.printReport();
    networkitLauncher = SP::NetworkitLauncher(graphFilename, SP::AlgoId::ASTAR);
    networkitLauncher.execute(source, destination);
    networkitLauncher.printReport();

    return 0;
}
