#include "dijkstra.hpp"
#include "astar.hpp"
#include "alt.hpp"
#include <iostream>
#include <string>
#include <memory>
#include <array>
#include <map>
#include <networkit/io/MTXGraphReader.hpp>
#include <networkit/distance/Dijkstra.hpp>

enum class AlgoId
{
    dijkstra,
    astar,
    alt,
    count
};
constexpr std::array<AlgoId, static_cast<size_t>(AlgoId::count)> algoIds =
{
    AlgoId::dijkstra,
    AlgoId::astar,
    AlgoId::alt
};
std::unordered_map<AlgoId, std::string> algoNames = 
{
    {AlgoId::dijkstra, "dijkstra"},
    {AlgoId::astar, "astar"},
    {AlgoId::alt, "alt"}
};

void setWeightsToOne(crsGraph &graph)
{
    for (int i = 0; i < graph.nz; i++)
    {
        graph.Eweights[i] = 1;
    }
}

std::shared_ptr<SP::BaseAlgo> createCustomAlgoObject (AlgoId algoId, std::string graphFilename)
{
    switch (algoId)
    {
        case AlgoId::dijkstra:
            return make_shared<SP::DijkstraAlgo>(graphFilename);
            break;
        case AlgoId::astar:
            return make_shared<SP::AStarAlgo>(graphFilename);
            break;
        case AlgoId::alt:
            return make_shared<SP::ALTAlgo>(graphFilename);
            break;
        default:
            return make_shared<SP::DijkstraAlgo>(graphFilename);
            break;
    }
}

void customAlgoExecute(std::string graphFilename, int source, int destination, AlgoId algoId)
{
    auto algo = createCustomAlgoObject(algoId, graphFilename);
    algo->preProcess();
    algo->setSource(source);
    algo->setDestination(destination);

    algo->compute();
    SP::OutData result = algo->getResult();

    std::cout << "Shortest path is ";
    for (auto vertex : result.shortestPath)
    {
        std::cout << ++vertex << ' ';
    }

    std::cout << std::endl
            << "Distance of the path equals " << result.shortestDistance
            << std::endl;

}

void networkitDijkstra(std::string graphFilename, int source, int destination)
{
    NetworKit::MTXGraphReader reader;
    NetworKit::Graph graph = reader.read(graphFilename);
    NetworKit::Dijkstra dijkstra(graph, source, /* reconstruct path */ true, /* sort vertexes */ false, destination);
    dijkstra.run();

    std::cout << "Shortest path is ";
    auto path = dijkstra.getPath(destination);
    for (auto vertex : path)
    {
        std::cout << ++vertex << ' ';
    }

    std::cout << std::endl
              << "Distance of the path equals " << dijkstra.getDistances()[destination]
              << std::endl;
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

    for (auto algoId : algoIds)
    {
        std::cout << "Custom " << algoNames[algoId] << " algorithm:" << std::endl;
        customAlgoExecute(graphFilename, source, destination, algoId);
        std::cout << std::endl << "==========" << std::endl << std::endl;
    }

    std::cout << "Networkit Dijkstra:" << std::endl;
    networkitDijkstra(graphFilename, source, destination);

    return 0;
}
