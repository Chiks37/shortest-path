/**
 * @file delta_stepping.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Delta-Stepping algorithm class header file
 */
#pragma once

#include "abstract_delta_stepping.hpp"

namespace SP
{
class DeltaSteppingAlgo final : public AbstractDeltaSteppingAlgo
{
  public:
    DeltaSteppingAlgo(std::string graphFileName)
        : AbstractDeltaSteppingAlgo(graphFileName)
    {
    }
};
} // namespace SP
