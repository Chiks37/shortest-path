/**
 * @file delta_stepping_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional delta-stepping algorithm class source file
 */

#include "delta_stepping_bidir.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace SP
{

void DeltaSteppingBiDirAlgo::initInternalData()
{
    DeltaSteppingAlgo::initInternalData();

    distancesBackward.assign(graph.V, std::numeric_limits<double>::infinity());
    distancesBackward[this->destination] = 0.0;
    parentsBackward.assign(graph.V, -1);

    vertexLocksBackward = std::vector<std::atomic_flag>(graph.V);
    for (auto &vertexLock : vertexLocksBackward)
    {
        vertexLock.clear(std::memory_order_release);
    }

    bucketsBackward.clear();
    bucketsBackward.resize(1);
    bucketInsertStampBackward.assign(graph.V, -1);
    bucketInsertBucketBackward.assign(graph.V, 0);
    insertStampBackward = 0;

    meetingVertex = -1;
    shortestPathLength = std::numeric_limits<double>::infinity();
}

void DeltaSteppingBiDirAlgo::resetInternalData()
{
    DeltaSteppingAlgo::resetInternalData();

    for (auto &vertexLock : vertexLocksBackward)
    {
        vertexLock.clear(std::memory_order_release);
    }

    if (bucketsBackward.empty())
    {
        bucketsBackward.resize(1);
    }
    bucketsBackward[0].clear();
    bucketsBackward[0].push_back(this->destination);

    if (!bucketInsertStampBackward.empty() && this->destination >= 0 &&
        this->destination < graph.V)
    {
        bucketInsertStampBackward[this->destination] = 0;
        bucketInsertBucketBackward[this->destination] = 0;
    }
}

ReturnCode DeltaSteppingBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = DeltaSteppingAlgo::preProcessImpl();
    if (rc != ReturnCode::OK)
    {
        return rc;
    }

    distancesBackward.resize(graph.V);
    parentsBackward.resize(graph.V);

    buildReverseGraph();
    classifyBackwardEdges();

    return rc;
}

void DeltaSteppingBiDirAlgo::buildReverseGraph()
{
    int V = graph.V;
    int nz = graph.Xadj[V];

    reverseXadj.assign(V + 1, 0);
    reverseAdjncy.resize(nz);
    reverseEweights.resize(nz);

    for (int i = 0; i < nz; ++i)
    {
        reverseXadj[graph.Adjncy[i] + 1]++;
    }
    for (int v = 0; v < V; ++v)
    {
        reverseXadj[v + 1] += reverseXadj[v];
    }

    std::vector<int> cursor(reverseXadj.begin(), reverseXadj.end() - 1);
    for (int u = 0; u < V; ++u)
    {
        for (int i = graph.Xadj[u]; i < graph.Xadj[u + 1]; ++i)
        {
            int v = graph.Adjncy[i];
            int pos = cursor[v]++;
            reverseAdjncy[pos] = u;
            reverseEweights[pos] = graph.Eweights[i];
        }
    }

    reverseGraph.Xadj = reverseXadj.data();
    reverseGraph.Adjncy = reverseAdjncy.data();
    reverseGraph.Eweights = reverseEweights.data();
    reverseGraph.V = V;
    reverseGraph.nz = nz;
}

void DeltaSteppingBiDirAlgo::classifyBackwardEdges()
{
    lightEdgesBackward.assign(graph.V, {});
    heavyEdgesBackward.assign(graph.V, {});

    for (int vertex = 0; vertex < graph.V; ++vertex)
    {
        for (int i = reverseGraph.Xadj[vertex];
             i < reverseGraph.Xadj[vertex + 1]; ++i)
        {
            int neighborVertex = reverseGraph.Adjncy[i];
            double edgeWeight = reverseGraph.Eweights[i];

            if (edgeWeight < delta)
            {
                lightEdgesBackward[vertex].push_back(
                    {neighborVertex, edgeWeight});
            }
            else
            {
                heavyEdgesBackward[vertex].push_back(
                    {neighborVertex, edgeWeight});
            }
        }
    }
}

ReturnCode DeltaSteppingBiDirAlgo::runSearch()
{
    if (graph.V <= 0)
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    bool forwardDone = false;
    bool backwardDone = false;
    std::size_t forwardBucket = 0;
    std::size_t backwardBucket = 0;

    while (!forwardDone || !backwardDone)
    {
        if (!forwardDone)
        {
            forwardDone = processOneBucket(true, forwardBucket);
        }
        if (!backwardDone)
        {
            backwardDone = processOneBucket(false, backwardBucket);
        }
    }

    return ReturnCode::OK;
}

bool DeltaSteppingBiDirAlgo::processOneBucket(bool forward,
                                              std::size_t &currentBucket)
{
    auto &myDistances = forward ? distances : distancesBackward;
    auto &myParents = forward ? parents : parentsBackward;
    const auto &myLightEdges = forward ? lightEdges : lightEdgesBackward;
    const auto &myHeavyEdges = forward ? heavyEdges : heavyEdgesBackward;
    auto &myBuckets = forward ? buckets : bucketsBackward;
    auto &myBucketInsertStamp =
        forward ? bucketInsertStamp : bucketInsertStampBackward;
    auto &myBucketInsertBucket =
        forward ? bucketInsertBucket : bucketInsertBucketBackward;
    int &myInsertStamp = forward ? insertStamp : insertStampBackward;
    auto &myVertexLocks = forward ? vertexLocks : vertexLocksBackward;
    const auto &otherDistances = forward ? distancesBackward : distances;

    while (currentBucket < myBuckets.size() && myBuckets[currentBucket].empty())
    {
        ++currentBucket;
    }

    if (currentBucket >= myBuckets.size())
    {
        return true;
    }

    double mySnapshot;
#pragma omp critical(deltaBidirMeeting)
    {
        mySnapshot = shortestPathLength;
    }
    if (static_cast<double>(currentBucket) * delta >= mySnapshot)
    {
        return true;
    }

    const double inf = std::numeric_limits<double>::infinity();

    auto relaxVertices = [&](const std::vector<int> &vertices,
                             const std::vector<std::vector<edge>> &adj)
    {
        if (vertices.empty())
        {
            return;
        }

        int maxThreads = 1;
#ifdef _OPENMP
        maxThreads = omp_get_max_threads();
#endif

        std::vector<std::vector<std::pair<int, std::size_t>>> localBuffers(
            static_cast<std::size_t>(maxThreads));

#pragma omp parallel
        {
            int tid = 0;
#ifdef _OPENMP
            tid = omp_get_thread_num();
#endif
            auto &buffer = localBuffers[static_cast<std::size_t>(tid)];
            buffer.clear();

#pragma omp for schedule(static)
            for (std::size_t idx = 0; idx < vertices.size(); ++idx)
            {
                int currentVertex = vertices[idx];

                double baseDist = 0.0;
                while (myVertexLocks[currentVertex].test_and_set(
                    std::memory_order_acquire))
                {
                }
                baseDist = myDistances[currentVertex];
                myVertexLocks[currentVertex].clear(std::memory_order_release);

                if (baseDist == inf)
                {
                    continue;
                }

                const auto &edges = adj[currentVertex];
                for (const auto &e : edges)
                {
                    int neighborVertex = e.vertex;
                    double newDist = baseDist + e.val;
                    bool updated = false;

                    while (myVertexLocks[neighborVertex].test_and_set(
                        std::memory_order_acquire))
                    {
                    }

                    if (newDist < myDistances[neighborVertex])
                    {
                        myDistances[neighborVertex] = newDist;
                        myParents[neighborVertex] = currentVertex;
                        updated = true;
                    }

                    myVertexLocks[neighborVertex].clear(
                        std::memory_order_release);

                    if (updated)
                    {
                        std::size_t bucketIndex =
                            static_cast<std::size_t>(newDist / delta);
                        buffer.push_back({neighborVertex, bucketIndex});

                        // Cross-check: snapshot the other-direction distance
                        // once so the guard and the sum stay consistent.
                        double otherDistance = otherDistances[neighborVertex];
                        if (otherDistance != inf)
                        {
                            double potentialPath = newDist + otherDistance;
#pragma omp critical(deltaBidirMeeting)
                            {
                                if (potentialPath < shortestPathLength)
                                {
                                    shortestPathLength = potentialPath;
                                    meetingVertex = neighborVertex;
                                }
                            }
                        }
                    }
                }
            }
        }

        ++myInsertStamp;
        std::size_t requiredSize = myBuckets.size();
        for (const auto &buffer : localBuffers)
        {
            for (const auto &item : buffer)
            {
                std::size_t bucketIndex = item.second;
                if (bucketIndex + 1 > requiredSize)
                {
                    requiredSize = bucketIndex + 1;
                }
            }
        }

        if (requiredSize > myBuckets.size())
        {
            myBuckets.resize(requiredSize);
        }

        for (const auto &buffer : localBuffers)
        {
            for (const auto &item : buffer)
            {
                int vertex = item.first;
                std::size_t bucketIndex = item.second;

                if (myBucketInsertStamp[vertex] == myInsertStamp &&
                    myBucketInsertBucket[vertex] == bucketIndex)
                {
                    continue;
                }

                myBucketInsertStamp[vertex] = myInsertStamp;
                myBucketInsertBucket[vertex] = bucketIndex;
                myBuckets[bucketIndex].push_back(vertex);
            }
        }
    };

    std::vector<int> settled;
    settled.reserve(myBuckets[currentBucket].size());

    while (!myBuckets[currentBucket].empty())
    {
        std::vector<int> request;
        request.swap(myBuckets[currentBucket]);
        if (request.empty())
        {
            break;
        }

        settled.insert(settled.end(), request.begin(), request.end());
        relaxVertices(request, myLightEdges);
    }

    if (!settled.empty())
    {
        relaxVertices(settled, myHeavyEdges);
    }

    ++currentBucket;
    return false;
}

ReturnCode DeltaSteppingBiDirAlgo::buildResult()
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
    outData.shortestDistance = shortestPathLength;

    return ReturnCode::OK;
}

} // namespace SP
