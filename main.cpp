#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <functional>
#include <networkit/graph/Graph.hpp>
#include <networkit/distance/Dijkstra.hpp>

using namespace std;

extern "C" {
    #include "graphio.h"
}

// pair of (destination, weight)
typedef pair<int, double> Edge;

void dijkstra(const crsGraph& graph, int sourceVertice, vector<double>& distances) {
    // Initialize distances
    distances.assign(graph.V, numeric_limits<double>::infinity());
    distances[sourceVertice] = 0;

    priority_queue<Edge, vector<Edge>, greater<Edge>> pq;
    pq.push({sourceVertice, 0.0});

    while (!pq.empty()) {
        int currentVertice = pq.top().first;
        double currentVerticeDistance = pq.top().second;
        pq.pop();

        // Skip if not the shortest path to currentVertice
        if (currentVerticeDistance > distances[currentVertice]) continue;

        // Researching neighbors
        for (int i = graph.Xadj[currentVertice]; i < graph.Xadj[currentVertice + 1]; i++) {
            int neighborVertice = graph.Adjncy[i];
            double neighborVerticeWeight = graph.Eweights[i];

            // Updating values
            if (distances[neighborVertice] > distances[currentVertice] + neighborVerticeWeight) {
                distances[neighborVertice] = distances[currentVertice] + neighborVerticeWeight;
                pq.push({neighborVertice, distances[neighborVertice]});
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <.mtx file>" << endl;
        return 1;
    }

    char* filename = argv[1];

    // Graph initialization
    crsGraph graph;
    init_graph(&graph);
    read_mtx_to_crs(&graph, filename);
    cout << "\tA graph with " << graph.V << " vertices and " << graph.nz / 2 << " edges is loaded\n\n";

    int sourceVertice = 0;
    vector<double> distances;

    for (int i = 0; i < graph.nz; i++)
    {
        graph.Eweights[i] = 1;
    }
    
    dijkstra(graph, sourceVertice, distances);

    // Output distances
    cout << "Shortest distances from vertex " << sourceVertice << ":\n";
    for (int i = 0; i < graph.V; ++i) {
        cout << "Vertex " << i + 1 << " : " << distances[i] << "\n";
    }

    // Networkit graph
    NetworKit::Graph networkitGraph (graph.V);
    for (int tempVertice = 0; tempVertice < graph.V; tempVertice++)
    {
        for (int tempNeighbourVertices = graph.Xadj[tempVertice]; tempNeighbourVertices < graph.Xadj[tempVertice + 1]; tempNeighbourVertices++)
        {
            int k = graph.Adjncy[tempNeighbourVertice];
            networkitGraph.addEdge(tempVertice, k);
        }
    }
    std::cout << "Graph created with " << networkitGraph.numberOfNodes() << " nodes and " << networkitGraph.numberOfEdges() << " edges.\n";
    
    NetworKit::Dijkstra networkitDijkstra(networkitGraph, 0);
    networkitDijkstra.run();

    vector<double> networkitDistances = networkitDijkstra.getDistances();

    std::cout << "Distances from node 0:\n";
    for (size_t i = 0; i < distances.size(); ++i) {
        std::cout << "Distance to node " << i << ": " << distances[i] << "\n";
    }

    return 0;
}
