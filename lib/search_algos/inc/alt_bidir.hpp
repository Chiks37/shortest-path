/**
 * @file alt_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional ALT algorithm class header file
 */
#pragma once

#include "abstract_astar_bidir.hpp"
#include "landmarks.hpp"

namespace SP
{
class ALTBiDirAlgo final : public AbstractAStarBiDirAlgo
{
  public:
    ALTBiDirAlgo(std::string graphFileName)
        : AbstractAStarBiDirAlgo(graphFileName)
    {
    }
    virtual ~ALTBiDirAlgo() {}

  protected:
    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex, int target) override;

    Landmarks landmarks;
};
} // namespace SP
