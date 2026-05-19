/**
 * @file alt_delta.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Alt delta-stepping algorithm class source file
 */

#include "alt_delta.hpp"
#include "dijkstra_sssp.hpp"
#include <limits>
#include <utility>

namespace SP
{
ReturnCode ALTDeltaAlgo::preProcessImpl()
{
    ReturnCode rc = AStarDeltaAlgo::preProcessImpl();
    landmarks.reserve(landmarksCount);
    distFromLandmarks.resize(landmarksCount, std::vector<double>(graph.V));

    rc = rc != ReturnCode::OK ? rc : initLandmarks();
    distToLandmarks = distFromLandmarks;

    return rc;
}

double ALTDeltaAlgo::heuristic(int vertex)
{
    double maxH = 0.0;

    for (int i = 0; i < landmarksCount; ++i)
    {
        double h_to =
            distToLandmarks[i][vertex] - distToLandmarks[i][this->destination];
        double h_from = distFromLandmarks[i][this->destination] -
                        distFromLandmarks[i][vertex];

        maxH = std::max({maxH, h_to, h_from});
    }

    return maxH;
}

ReturnCode ALTDeltaAlgo::runDijkstraSssp(DijkstraSsspAlgo &dijkstra, int source,
                                         std::vector<double> &distances)
{
    ReturnCode rc = dijkstra.setSource(source);
    rc = rc != ReturnCode::OK ? rc : dijkstra.compute();
    distances = std::move(dijkstra.getDistances());

    return rc;
}

ReturnCode ALTDeltaAlgo::initLandmarks()
{
    /**
     * dist - "distance"
     * Ls   - "landmarks"
     */

    if (graph.V == 0)
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    DijkstraSsspAlgo dijkstra(graphFileName);
    dijkstra.preProcess();

    // 0 - is first landmark
    int currentLandmark = 0;
    landmarks.emplace_back(currentLandmark);

    // Calculate distances from all vertexes to the first landmark
    ReturnCode rc =
        runDijkstraSssp(dijkstra, currentLandmark, distFromLandmarks[0]);
    if (rc != ReturnCode::OK)
    {
        return rc;
    }
    std::vector<double> distToLs(distFromLandmarks[0]);

    for (int i = 1; i < landmarksCount; ++i)
    {
        double maxDistToLs = -1.0;
        int nextLandmark = -1;

        // Define the most remote vertex from already selected landmarks
        for (int vertex = 0; vertex < graph.V; ++vertex)
        {
            if (distToLs[vertex] > maxDistToLs)
            {
                maxDistToLs = distToLs[vertex];
                nextLandmark = vertex;
            }
        }

        // No more vertexes to select as landmark
        if (nextLandmark == -1)
        {
            landmarksCount = i;
            landmarks.resize(landmarksCount);
            distFromLandmarks.resize(landmarksCount);
            break;
        }

        // The most remote vertex is the next landmark
        currentLandmark = nextLandmark;
        landmarks.emplace_back(currentLandmark);

        // Calculate distances from all vertexes to the new landmark
        rc = runDijkstraSssp(dijkstra, currentLandmark, distFromLandmarks[i]);
        if (rc != ReturnCode::OK)
        {
            return rc;
        }

        // Looking for shorter distances to any landmarks including the new one
        for (int vertex = 0; vertex < graph.V; ++vertex)
        {
            distToLs[vertex] =
                std::min(distToLs[vertex], distFromLandmarks[i][vertex]);
        }
    }
    return rc;
}

} // namespace SP
