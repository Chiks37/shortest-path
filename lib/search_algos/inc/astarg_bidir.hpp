/**
 * @file astarg_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar with geometrical heuristic algorithm class header
 * file
 */
#pragma once

#include "abstract_astar_bidir.hpp"
#include "coordinates.hpp"

namespace SP
{

class AStarGBiDirAlgo final : public AbstractAStarBiDirAlgo
{
  public:
    AStarGBiDirAlgo(std::string graphFileName, std::string nodesMappingFileName)
        : AbstractAStarBiDirAlgo(graphFileName),
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
