/**
 * @file astarg.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar with geometical heuristic algorithm class header file
 */
#pragma once

#include "abstract_astar.hpp"
#include "coordinates.hpp"

namespace SP
{

class AStarGAlgo final : public AbstractAStarAlgo
{
  public:
    AStarGAlgo(std::string graphFileName, std::string nodesMappingFileName)
        : AbstractAStarAlgo(graphFileName),
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