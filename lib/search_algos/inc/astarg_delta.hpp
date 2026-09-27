/**
 * @file astarg_delta.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar with geometical heuristic delta-stepping algorithm class header
 * file
 */
#pragma once

#include "abstract_astar_delta.hpp"
#include "coordinates.hpp"

namespace SP
{

class AStarGDeltaAlgo final : public AbstractAStarDeltaAlgo
{
  public:
    AStarGDeltaAlgo(std::string graphFileName, std::string nodesMappingFileName)
        : AbstractAStarDeltaAlgo(graphFileName),
          nodesMappingFileName(nodesMappingFileName)
    {
    }

  protected:
    std::string nodesMappingFileName;
    Coordinates coordinates;

    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex) override;
};

} // namespace SP
