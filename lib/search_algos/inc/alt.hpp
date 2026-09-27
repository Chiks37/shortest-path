/**
 * @file alt.hpp
 * @author tarakanov.2004@mail.ru
 * @brief ALT algorithm class header file
 */
#pragma once

#include "abstract_astar.hpp"
#include "landmarks.hpp"

namespace SP
{
class ALTAlgo final : public AbstractAStarAlgo
{
  public:
    ALTAlgo(std::string graphFileName) : AbstractAStarAlgo(graphFileName) {}
    virtual ~ALTAlgo() {}

  protected:
    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex) override;

    Landmarks landmarks;
};
} // namespace SP
