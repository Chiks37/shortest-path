/**
 * @file alt.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Alt algorithm class source file
 */

#include "alt.hpp"
#include "dijkstra_sssp.hpp"
#include <limits>
#include <utility>

namespace SP
{
ReturnCode ALTAlgo::preProcessImpl()
{
    DijkstraAlgo::preProcessImpl();

    initLandmarks();

    // Continue here to estimate distances betwen landmarks and vetexes

    // @TODO Dist from landmark is the same as to landmark because for beginning
    // we think that graph is not oriented
    return ReturnCode();
}

ReturnCode ALTAlgo::runDijkstraSssp(DijkstraSsspAlgo &dijkstra, int source,
                                    std::vector<double> &distances)
{
    ReturnCode rc = dijkstra.setSource(source);
    rc = rc != ReturnCode::OK ? rc : dijkstra.compute();
    distances = std::move(dijkstra.getDistances());

    return rc;
}

ReturnCode ALTAlgo::initLandmarks()
{
    /**
     *
     * dist - distance
     * L    - landmark
     */

    if (graph.V == 0)
    {
        return ReturnCode::BAD_ARGUMENTS;
    }
    landmarks.reserve(landmarksCount);
    distFromLandmarks.resize(landmarksCount, std::vector<double>(graph.V));

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