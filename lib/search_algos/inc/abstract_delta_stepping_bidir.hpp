/**
 * @file abstract_delta_stepping_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional delta-stepping algorithm class header file
 */
#pragma once

#include "abstract_delta_stepping.hpp"
#include <limits>

namespace SP
{
class AbstractDeltaSteppingBiDirAlgo : public AbstractDeltaSteppingAlgo
{
  protected:
    AbstractDeltaSteppingBiDirAlgo(std::string graphFileName)
        : AbstractDeltaSteppingAlgo(graphFileName), meetingVertex(-1),
          shortestPathLength(std::numeric_limits<double>::infinity())
    {
    }

    virtual void initInternalData() override;
    virtual void initQuery() override;
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode runSearch() override;
    virtual ReturnCode buildResult() override;

    void buildReverseGraph();
    void classifyBackwardEdges();

    // Process one bucket of the selected direction (forward = true uses the
    // inherited delta-stepping state on the original graph; forward = false
    // mirrors it on the reverse graph). Returns true when the whole search is
    // done: either this direction ran out of buckets or the lower bounds of
    // both directions sum up to the best path found (Pohl stopping criterion).
    bool processOneBucket(bool forward, std::size_t &currentBucket,
                          std::size_t otherBucket);

    // Backward direction mirror of all per-vertex state inherited from
    // AbstractDeltaSteppingAlgo. The reverse graph drives the backward bucket
    // loop.
    std::vector<double> distancesBackward;
    std::vector<int> parentsBackward;
    std::vector<std::vector<edge>> lightEdgesBackward;
    std::vector<std::vector<edge>> heavyEdgesBackward;
    std::vector<std::vector<int>> bucketsBackward;
    std::vector<int> bucketInsertStampBackward;
    std::vector<std::size_t> bucketInsertBucketBackward;
    int insertStampBackward{0};
    std::vector<std::atomic_flag> vertexLocksBackward;

    // Transpose of graph for the backward half-search.
    std::vector<int> reverseXadj;
    std::vector<int> reverseAdjncy;
    std::vector<double> reverseEweights;
    crsGraph reverseGraph{};

    // Best path length seen so far and the vertex where the two searches met.
    // Updated under critical(deltaBidirMeeting) from either direction.
    int meetingVertex;
    double shortestPathLength;
};
} // namespace SP
