/**
 * @file abstract_dijkstra_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra (bidirectional version) algorithm class header file
 */
#pragma once

#include "abstract_dijkstra_seq.hpp"
#include <limits>

namespace SP
{
// The forward search from the source and the backward search from the
// destination on the transposed graph take turns scanning one vertex each.
// shortestPathLength is the best path found through a vertex labeled by both,
// the search stops once the smallest keys of the two queues sum up to it.
class AbstractDijkstraBiDirAlgo : public AbstractDijkstraSeqAlgo
{
  protected:
    AbstractDijkstraBiDirAlgo(std::string graphFileName)
        : AbstractDijkstraSeqAlgo(graphFileName), meetingVertex(-1),
          shortestPathLength(std::numeric_limits<double>::infinity())
    {
    }

    std::vector<double> distancesBackward;
    std::vector<int> parentsBackward;
    std::vector<int> touchedBackward;
    EdgeQueue pqBackward;
    int meetingVertex;
    double shortestPathLength;

    // Transpose of graph: the backward search must traverse reversed edges
    std::vector<int> reverseXadj;
    std::vector<int> reverseAdjncy;
    std::vector<double> reverseEweights;
    crsGraph reverseGraph{};

    virtual void initInternalData() override;
    virtual void initQuery() override;
    virtual void resetInternalData() override;
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode runSearch() override;
    virtual ReturnCode buildResult() override;
    void buildReverseGraph();
    void scanNextVertex(bool forward);
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
