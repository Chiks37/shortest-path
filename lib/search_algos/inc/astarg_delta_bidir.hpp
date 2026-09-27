/**
 * @file astarg_delta_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar with geometrical heuristic delta-stepping
 * algorithm class header file
 */
#pragma once

#include "abstract_astar_delta_bidir.hpp"
#include "coordinates.hpp"

namespace SP
{

class AStarGDeltaBiDirAlgo final : public AbstractAStarDeltaBiDirAlgo
{
  public:
    AStarGDeltaBiDirAlgo(std::string graphFileName,
                         std::string nodesMappingFileName)
        : AbstractAStarDeltaBiDirAlgo(graphFileName),
          nodesMappingFileName(nodesMappingFileName)
    {
    }

  protected:
    std::string nodesMappingFileName;
    Coordinates coordinates;

    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex, int target) override;
};

} // namespace SP
