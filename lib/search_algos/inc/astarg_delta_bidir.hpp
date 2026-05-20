/**
 * @file astarg_delta_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar with geometrical heuristic delta-stepping
 * algorithm class header file
 */
#pragma once

#include "astar_delta_bidir.hpp"
#include <utility>
#include <vector>

namespace SP
{

class AStarGDeltaBiDirAlgo : public AStarDeltaBiDirAlgo
{
  public:
    AStarGDeltaBiDirAlgo(std::string graphFileName,
                         std::string nodesMappingFileName)
        : AStarDeltaBiDirAlgo(graphFileName),
          nodesMappingFileName(nodesMappingFileName)
    {
    }

  protected:
    std::string nodesMappingFileName;
    std::vector<std::pair<double, double>> vertexCoordinates;

    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex, int target) override;
    ReturnCode loadNodeMapping();
};

} // namespace SP
