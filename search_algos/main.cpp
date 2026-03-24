#include <dijkstra.hpp>
#include <iostream>
#include <string>
#include <networkit/io/MTXGraphReader.hpp>
#include <networkit/distance/Dijkstra.hpp>

void setWeightsToOne(crsGraph &graph)
{
    for (int i = 0; i < graph.nz; i++)
    {
        graph.Eweights[i] = 1;
    }
}

void customDijsktra(std::string graphFilename, int source, int destination)
{
    SP::DijkstraAlgo dijkstra(graphFilename);
    dijkstra.preProcess();
    dijkstra.setSource(source);
    dijkstra.setDestination(destination);

    dijkstra.compute();
    SP::OutData result = dijkstra.getResult();

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

    std::cout << "Custom Dijkstra:" << std::endl;
    customDijsktra(graphFilename, source, destination);
    std::cout << "Networkit Dijkstra:" << std::endl;
    networkitDijkstra(graphFilename, source, destination);

    return 0;
}
