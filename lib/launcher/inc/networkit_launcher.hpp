/**
 * @file networkit_launcher.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Networkit algorithms launcher class header file
 */
#pragma once

#include "launcher.hpp"
#include <networkit/distance/AStar.hpp>
#include <networkit/distance/Dijkstra.hpp>
#include <networkit/io/MTXGraphReader.hpp>

namespace SP
{

class NetworkitLauncher : public Launcher
{
  private:
    std::vector<double> astarHeuristics;
    std::shared_ptr<NetworKit::Algorithm>
    createAlgoObject(const NetworKit::Graph &graph, int source,
                     int destination);

  public:
    void execute(int source, int destination) override;

    explicit NetworkitLauncher(const std::string &graphFileName, AlgoId algoId)
        : Launcher(graphFileName, algoId)
    {
    }
};

} // namespace SP
