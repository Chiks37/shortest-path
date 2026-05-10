/**
 * @file launcher.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Base launcher class header file
 */
#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

namespace SP
{

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

  public:
    explicit Launcher(const std::string &graphFileName)
        : graphFileName(graphFileName), lastResult{}
    {
    }
    virtual ~Launcher() = default;
    virtual void execute(int source, int destination) = 0;
    virtual std::string getAlgoName() const = 0;
    const LauncherResult &getResult() const { return lastResult; }
    void printReport() const;
    const std::string &getGraphFilename() const { return graphFileName; }
};

} // namespace SP
