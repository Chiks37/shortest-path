/**
 * @file alt_delta.hpp
 * @author tarakanov.2004@mail.ru
 * @brief ALT delta-stepping algorithm class header file
 */
#pragma once

#include "abstract_astar_delta.hpp"
#include "landmarks.hpp"

namespace SP
{
class ALTDeltaAlgo final : public AbstractAStarDeltaAlgo
{
  public:
    ALTDeltaAlgo(std::string graphFileName)
        : AbstractAStarDeltaAlgo(graphFileName)
    {
    }
    virtual ~ALTDeltaAlgo() {}

  protected:
    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex) override;

    Landmarks landmarks;
};
} // namespace SP
