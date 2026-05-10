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

#include <array>
#include <unordered_map>

namespace SP
{

class NetworkitLauncher : public Launcher
{
  public:
    enum class AlgoId
    {
        DIJKSTRA_SEQ,
        ASTARG,
        COUNT
    };

    static constexpr std::array<AlgoId, static_cast<size_t>(AlgoId::COUNT)>
        algoIds = {AlgoId::DIJKSTRA_SEQ, AlgoId::ASTARG};

    static inline const std::unordered_map<AlgoId, std::string> algoNames = {
        {AlgoId::DIJKSTRA_SEQ, "networkit_dijkstra"},
        {AlgoId::ASTARG, "networkit_astar"}};

  private:
    AlgoId algoId;
    std::vector<double> astarHeuristics;
    std::shared_ptr<NetworKit::Algorithm>
    createAlgoObject(const NetworKit::Graph &graph, int source,
                     int destination);

  public:
    std::string getAlgoName() const override { return algoNames.at(algoId); }
    void execute(int source, int destination) override;

    explicit NetworkitLauncher(const std::string &graphFileName, AlgoId algoId)
        : Launcher(graphFileName), algoId(algoId)
    {
    }
};

} // namespace SP
