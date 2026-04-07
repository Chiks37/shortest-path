/**
 * @file custom_launcher.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Custom launcher class header file
 */
#pragma once

#include "alt.hpp"
#include "astarg.hpp"
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
    std::string nodesMappingFileName;

  public:
    void execute(int source, int destination) override;

    CustomLauncher(AlgoId algoId, const std::string &graphFileName,
                   const std::string &nodesMappingFileName = "")
        : Launcher(graphFileName, algoId),
          nodesMappingFileName(nodesMappingFileName)
    {
    }
};

} // namespace SP
