/**
 * @file abstract_delta_stepping.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Delta-Stepping algorithm class header file
 */
#pragma once

#include "abstract_dijkstra.hpp"
#include <atomic>
#include <cstddef>

namespace SP
{
class AbstractDeltaSteppingAlgo : public AbstractDijkstraAlgo
{
  protected:
    AbstractDeltaSteppingAlgo(std::string graphFileName)
        : AbstractDijkstraAlgo(graphFileName)
    {
    }

    virtual void initInternalData() override;
    virtual void resetInternalData() override;
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode runSearch() override;

    void computeDelta();
    void classifyEdges();

    // Bucket width / edge-weight threshold. Bucket i collects vertices whose
    // ordering cost lies in [i*delta, (i+1)*delta). Edges with weight < delta
    // are "light", the rest are "heavy".
    double delta{1.0};
    // Per-vertex adjacency list holding only light edges (weight < delta).
    // Relaxed repeatedly while the current bucket keeps refilling.
    std::vector<std::vector<edge>> lightEdges;
    // Per-vertex adjacency list holding only heavy edges (weight >= delta).
    // Relaxed once, after the current bucket is fully settled.
    std::vector<std::vector<edge>> heavyEdges;
    // Bucket array: buckets[i] is the list of vertices currently assigned to
    // bucket i. The algorithm processes buckets in ascending index order.
    std::vector<std::vector<int>> buckets;
    // Per-vertex stamp of the relaxation phase in which the vertex was last
    // queued. Compared against insertStamp to detect duplicates within a phase.
    std::vector<int> bucketInsertStamp;
    // Per-vertex index of the bucket the vertex was last queued into. Together
    // with bucketInsertStamp it prevents inserting the same vertex into the
    // same bucket twice during one relaxation phase.
    std::vector<std::size_t> bucketInsertBucket;
    // Monotonic counter bumped once per relaxation phase; written into
    // bucketInsertStamp to tag insertions as belonging to the current phase.
    int insertStamp{0};
    // Per-vertex spinlocks guarding concurrent writes to distances[]/parents[]
    // (and the cost read) during parallel edge relaxation.
    std::vector<std::atomic_flag> vertexLocks;
};
} // namespace SP