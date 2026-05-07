/**
 * @file custom_launcher.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Custom algorithms launcher class header file
 */
#pragma once

#include "alt.hpp"
#include "astarg.hpp"
#include "dijkstra_bidir.hpp"
#include "dijkstra_par_expansion.hpp"
#include "dijkstra_par_relaxation.hpp"
#include "dijkstra_seq.hpp"
#include "delta_stepping.hpp"
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
