/**
 * @file dijkstra_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Dijkstra (bidirectional version) algorithm class header file
 */

#include "dijkstra_seq.hpp"
#include <limits>

namespace SP
{
class dijkstraBiDirAlgo : public DijkstraSeqAlgo
{
  public:
    dijkstraBiDirAlgo(std::string graphFileName)
        : DijkstraSeqAlgo(graphFileName), meetingVertex(-1),
          shortestPathLength(std::numeric_limits<double>::infinity())
    {
    }

  protected:
    using CostEstimator = double (dijkstraBiDirAlgo::*)(int vertex);
    std::vector<double> distancesBackward;
    std::vector<int> parentsBackward;
    std::vector<int> shortestPathBackward;
    std::priority_queue<edge, std::vector<edge>, compareEdges> pqBackward;
    int meetingVertex;
    double shortestPathLength;

    virtual void initInternalData() override;
    virtual void resetInternalData() override;
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode computeImpl() override;
    virtual ReturnCode buildResult() override;
    ReturnCode runHalfSearch(
        std::priority_queue<edge, std::vector<edge>, compareEdges> &myPq,
        std::vector<double> &myDistances, std::vector<int> &myParents,
        const std::vector<double> &otherDistances,
        CostEstimator costEstimator);
    std::vector<int> reconstructPath(int source, int destination,
                                     const std::vector<int> &myParents);
    double getDistanceBackward(int vertex) { return distancesBackward[vertex]; }
    double estimateCostBackward(int vertex) { return getDistanceBackward(vertex); } 
};
} // namespace SP