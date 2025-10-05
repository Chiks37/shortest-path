#include <dijkstra.hpp>
#include <iostream>
#include <string>

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

  // Out data
  double shortestDistance;
  std::vector<int> shortestPath;

  std::string filename(argv[1]);
  SP::BaseAlgo::AlgoInData data{.graphFileName = filename, .source = source, .destination = destination, .shortestDistance = shortestDistance, .shortestPath = shortestPath};
  SP::DijkstraAlgo dijkstra(data);

  dijkstra.execute();

  std::cout << "Shortest path is ";
  for (auto vertex : shortestPath)
  {
    std::cout << ++vertex << ' ';
  }
  std::cout << std::endl << "Distance of the path equals " << shortestDistance << std::endl;
  // for (int i = 0; i < graph.nz; i++)       ??????
  // {
  //     graph.Eweights[i] = 1;
  // }

  // Output distances
  // cout << "Shortest distances from vertex " << sourceVertice << ":\n";
  // for (int i = 0; i < graph.V; ++i) {
  //     cout << "Vertex " << i + 1 << " : " << distances[i] << "\n";
  // }

  return 0;
}
