/**
 * @file abstract_delta_stepping_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional delta-stepping algorithm class header file
 */
#pragma once

#include "abstract_delta_stepping.hpp"

namespace SP
{
// The forward direction on the original graph and the backward one on the
// reverse graph settle one bucket each in turn. The search stops once the
// buckets reached by the two directions sum up to the best path found (Pohl
// stopping criterion on bucket bounds).
class AbstractDeltaSteppingBiDirAlgo : public AbstractDeltaSteppingAlgo
{
  protected:
    AbstractDeltaSteppingBiDirAlgo(std::string graphFileName)
        : AbstractDeltaSteppingAlgo(graphFileName)
    {
    }

    virtual void initInternalData() override;
    virtual void initQuery() override;
    virtual void resetInternalData() override;
    virtual ReturnCode preProcessImpl() override;
    virtual ReturnCode runSearch() override;
    virtual ReturnCode buildResult() override;

    void buildReverseGraph();

    std::vector<double> distancesBackward;
    std::vector<int> parentsBackward;
    std::vector<int> lightEndBackward;
    Direction backwardDirection;

    // Transpose of graph for the backward half-search.
    std::vector<int> reverseXadj;
    std::vector<int> reverseAdjncy;
    std::vector<double> reverseEweights;
    crsGraph reverseGraph{};
};
} // namespace SP
