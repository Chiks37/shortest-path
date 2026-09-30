/**
 * @file abstract_delta_stepping_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional delta-stepping algorithm class source file
 */

#include "abstract_delta_stepping_bidir.hpp"
#include "graph_transpose.hpp"
#include <algorithm>
#include <limits>

namespace SP
{

void AbstractDeltaSteppingBiDirAlgo::initInternalData()
{
    AbstractDeltaSteppingAlgo::initInternalData();

    distancesBackward.assign(graph.V, std::numeric_limits<double>::infinity());
    parentsBackward.assign(graph.V, -1);

    backwardDirection.graph = &reverseGraph;
    backwardDirection.lightEnd = &lightEndBackward;
    backwardDirection.labels = &distancesBackward;
    backwardDirection.parents = &parentsBackward;
    backwardDirection.potentialSign = -1.0;
}

void AbstractDeltaSteppingBiDirAlgo::initQuery()
{
    AbstractDeltaSteppingAlgo::initQuery();

    distancesBackward[this->destination] = 0.0;
    startDirection(backwardDirection, this->destination);
    backwardDirection.threads[0].touched.push_back(this->destination);

    // The meeting is detected on relaxation only, so a trivial query would
    // otherwise meet at a neighbor and report a non-zero distance.
    if (this->source == this->destination)
    {
        meetingVertex = this->source;
        shortestPathLength.store(0.0);
    }
}

void AbstractDeltaSteppingBiDirAlgo::resetInternalData()
{
    resetDirection(backwardDirection);

    AbstractDeltaSteppingAlgo::resetInternalData();
}

ReturnCode AbstractDeltaSteppingBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractDeltaSteppingAlgo::preProcessImpl();
    if (rc != ReturnCode::OK)
    {
        return rc;
    }

    buildReverseGraph();
    orderEdges(reverseGraph, lightEndBackward);

    return rc;
}

void AbstractDeltaSteppingBiDirAlgo::buildReverseGraph()
{
    transposeGraph(graph, reverseXadj, reverseAdjncy, reverseEweights,
                   reverseGraph);
}

ReturnCode AbstractDeltaSteppingBiDirAlgo::runSearch()
{
    if (this->source == this->destination)
    {
        return ReturnCode::OK;
    }

#pragma omp parallel
    {
        bool forward = true;
        while (forward
                   ? processBucket(forwardDirection, &backwardDirection, false)
                   : processBucket(backwardDirection, &forwardDirection, false))
        {
            forward = !forward;
        }
    }

    return ReturnCode::OK;
}

ReturnCode AbstractDeltaSteppingBiDirAlgo::buildResult()
{
    if (meetingVertex == -1)
    {
        outData.shortestDistance = std::numeric_limits<double>::infinity();
        outData.shortestPath.clear();
        return ReturnCode::OK;
    }

    std::vector<int> forwardPath;
    int v = meetingVertex;
    forwardPath.push_back(v);
    while (v != this->source)
    {
        int p = parents[v];
        if (p == -1)
        {
            break;
        }
        forwardPath.push_back(p);
        v = p;
    }
    std::reverse(forwardPath.begin(), forwardPath.end());

    std::vector<int> backwardPath;
    v = meetingVertex;
    backwardPath.push_back(v);
    while (v != this->destination)
    {
        int p = parentsBackward[v];
        if (p == -1)
        {
            break;
        }
        backwardPath.push_back(p);
        v = p;
    }

    outData.shortestPath.clear();
    outData.shortestPath.insert(outData.shortestPath.end(), forwardPath.begin(),
                                forwardPath.end());
    outData.shortestPath.insert(outData.shortestPath.end(),
                                backwardPath.begin() + 1, backwardPath.end());
    outData.shortestDistance = shortestPathLength.load();

    return ReturnCode::OK;
}

} // namespace SP
