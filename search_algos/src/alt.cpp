/**
 * @file alt.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Alt algorithm class source file
 */

#include "alt.hpp"
#include "dijkstra_sssp.hpp"

namespace SP
{
ReturnCode ALTAlgo::preProcessImpl()
{
    DijkstraAlgo::preProcessImpl();

    initLandmarks();

    // Continue here to estimate distances betwen landmarks and vetexes

    return ReturnCode();
}

void ALTAlgo::initLandmarks()
{
    landmarks.reserve(landmarksCount);
    DijkstraSsspAlgo dijkstra(data);

    int currentVertex = 0;
    dijkstra
}

} // namespace SP