/**
 * @file delta_stepping_bidir.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional delta-stepping algorithm class header file
 */
#pragma once

#include "abstract_delta_stepping_bidir.hpp"

namespace SP
{
class DeltaSteppingBiDirAlgo final : public AbstractDeltaSteppingBiDirAlgo
{
  public:
    DeltaSteppingBiDirAlgo(std::string graphFileName)
        : AbstractDeltaSteppingBiDirAlgo(graphFileName)
    {
    }
};
} // namespace SP
