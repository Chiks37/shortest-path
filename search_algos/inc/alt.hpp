/**
 * @file alt.hpp
 * @author tarakanov.2004@mail.ru
 * @brief ALT algorithm class header file
 */
#pragma once

#include "astar.hpp"

namespace SP
{
class ALTAlgo : public AStarAlgo
{
  public:
    ALTAlgo(std::string graphFileName) : AStarAlgo(graphFileName) {}
    virtual ~ALTAlgo() {}

  protected:
    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex) override;

    void initLandmarks();

    std::vector<std::vector<double>> distToLandmarks;
    std::vector<std::vector<double>> distFromLandmarks;
    std::vector<int> landmarks;
    static constexpr int landmarksCount = 16; 

};
} // namespace SP