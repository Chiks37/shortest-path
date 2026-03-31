/**
 * @file custom_launcher.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Custom launcher class header file
 */
#pragma once

#include "alt.hpp"
#include "astar.hpp"
#include "dijkstra.hpp"
#include "launcher.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>

namespace SP
{

class CustomLauncher : public Launcher
{
  private:
    std::shared_ptr<SP::BaseAlgo> createAlgoObject();

  public:
    void execute(int source, int destination) override;

    CustomLauncher(const std::string &graphFileName, AlgoId algoId)
        : Launcher(graphFileName, algoId)
    {
    }
};

} // namespace SP
