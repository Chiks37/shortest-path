/**
 * @file astarg_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar with geometrical heuristic algorithm class header
 * file
 */
#pragma once

#include "astar_bidir.hpp"
#include <utility>
#include <vector>

namespace SP
{

class AStarGBiDirAlgo : public AStarBiDirAlgo
{
  public:
    AStarGBiDirAlgo(std::string graphFileName, std::string nodesMappingFileName)
        : AStarBiDirAlgo(graphFileName),
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
