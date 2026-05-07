/**
 * @file launcher.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Base launcher class header file
 */
#pragma once

#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace SP
{

enum class AlgoId
{
    DIJKSTRA_SEQ,
    DIJKSTRA_BIDIR,
    DIJKSTRA_PAR_EXPANSION,
    DIJKSTRA_PAR_RELAXATION,
    DELTA_STEPPING,
    ASTARG,
    ALT,
    COUNT
};
inline constexpr std::array<AlgoId, static_cast<size_t>(AlgoId::COUNT)>
    algoIds = {AlgoId::DIJKSTRA_SEQ,
               AlgoId::DIJKSTRA_BIDIR,
               AlgoId::DIJKSTRA_PAR_EXPANSION,
               AlgoId::DIJKSTRA_PAR_RELAXATION,
               AlgoId::DELTA_STEPPING,
               AlgoId::ASTARG,
               AlgoId::ALT};
inline const std::unordered_map<AlgoId, std::string> algoNames = {
    {AlgoId::DIJKSTRA_SEQ, "dijkstra_seq"},
    {AlgoId::DIJKSTRA_BIDIR, "dijkstra_bidir"},
    {AlgoId::DIJKSTRA_PAR_EXPANSION, "dijkstra_par_expansion"},
    {AlgoId::DIJKSTRA_PAR_RELAXATION, "dijkstra_par_relaxation"},
    {AlgoId::DELTA_STEPPING, "delta_stepping"},
    {AlgoId::ASTARG, "astarg"},
    {AlgoId::ALT, "alt"}};

struct LauncherResult
{
    std::vector<int> shortestPath;
    double shortestDistance;
    double executionTimeMs;
    double preProccessTimeMs;
};

class Launcher
{
  protected:
    std::string graphFileName;
    LauncherResult lastResult;
    AlgoId algoId;

  public:
    explicit Launcher(const std::string &graphFileName, AlgoId algoId)
        : graphFileName(graphFileName), lastResult{}, algoId(algoId)
    {
    }
    virtual ~Launcher() = default;
    virtual void execute(int source, int destination) = 0;
    const LauncherResult &getResult() const { return lastResult; }
    void setAlgoId(AlgoId id) { algoId = id; }
    AlgoId getAlgoId() const { return algoId; }
    void printReport() const;
    const std::string &getGraphFilename() const { return graphFileName; }
};

} // namespace SP
