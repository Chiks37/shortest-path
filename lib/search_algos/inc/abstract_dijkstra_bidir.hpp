/**
 * @file abstract_dijkstra_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra (bidirectional version) algorithm class header file
 */
#pragma once

#include "abstract_dijkstra_seq.hpp"
#include <atomic>
#include <limits>

namespace SP
{
class AbstractDijkstraBiDirAlgo : public AbstractDijkstraSeqAlgo
{
  protected:
    AbstractDijkstraBiDirAlgo(std::string graphFileName)
        : AbstractDijkstraSeqAlgo(graphFileName), meetingVertex(-1),
          shortestPathLength(std::numeric_limits<double>::infinity())
    {
    }

    using CostEstimator = double (AbstractDijkstraBiDirAlgo::*)(int vertex);
    std::vector<double> distancesBackward;
    std::vector<int> parentsBackward;
    std::vector<int> shortestPathBackward;
    std::priority_queue<edge, std::vector<edge>, compareEdges> pqBackward;
    int meetingVertex;
    std::atomic<double> shortestPathLength;
    // The half-searches run concurrently, each publishing the smallest key it
    // may still scan. Once the two bounds sum up to shortestPathLength no
    // shorter path exists (Pohl stopping criterion).
    std::atomic<double> forwardBound;
    std::atomic<double> backwardBound;

    // Transpose of graph: the backward search must traverse reversed edges
    std::vector<int> reverseXadj;
    std::vector<int> reverseAdjncy;
    std::vector<double> reverseEweights;
    crsGraph reverseGraph{};

    virtual void initInternalData() override;
    virtual void initQuery() override;
    virtual void resetInternalData() override;
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode computeImpl() override;
    virtual ReturnCode buildResult() override;
    void buildReverseGraph();
    ReturnCode runHalfSearch(
        const crsGraph &searchGraph,
        std::priority_queue<edge, std::vector<edge>, compareEdges> &myPq,
        std::vector<double> &myDistances, std::vector<int> &myParents,
        std::vector<double> &otherDistances, std::atomic<double> &myBound,
        const std::atomic<double> &otherBound, CostEstimator costEstimator);
    std::vector<int> reconstructPath(int source, int destination,
                                     const std::vector<int> &myParents);
    virtual double getDistanceBackward(int vertex)
    {
        return distancesBackward[vertex];
    }
    virtual double estimateCostBackward(int vertex)
    {
        return getDistanceBackward(vertex);
    }
};
} // namespace SP