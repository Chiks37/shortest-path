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
    DIJKSTRA,
    ASTAR,
    ALT,
    COUNT
};
inline constexpr std::array<AlgoId, static_cast<size_t>(AlgoId::COUNT)>
    algoIds = {AlgoId::DIJKSTRA, AlgoId::ASTAR, AlgoId::ALT};
inline const std::unordered_map<AlgoId, std::string> algoNames = {
    {AlgoId::DIJKSTRA, "dijkstra"},
    {AlgoId::ASTAR, "astar"},
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
        : graphFileName(graphFileName), lastResult{}, algoId(algoId){}
    virtual ~Launcher() = default;
    virtual void execute(int source, int destination) = 0;
    const LauncherResult &getResult() const { return lastResult; }
    void setAlgoId(AlgoId id) { algoId = id; }
    AlgoId getAlgoId() const { return algoId; }
    void printReport() const;
    const std::string &getGraphFilename() const { return graphFileName; }
};

} // namespace SP
