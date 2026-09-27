/**
 * @file alt_delta_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional ALT delta-stepping algorithm class header file
 */
#pragma once

#include "abstract_astar_delta_bidir.hpp"
#include "landmarks.hpp"

namespace SP
{
class ALTDeltaBiDirAlgo final : public AbstractAStarDeltaBiDirAlgo
{
  public:
    ALTDeltaBiDirAlgo(std::string graphFileName)
        : AbstractAStarDeltaBiDirAlgo(graphFileName)
    {
    }
    virtual ~ALTDeltaBiDirAlgo() {}

  protected:
    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex, int target) override;

    Landmarks landmarks;
};
} // namespace SP
