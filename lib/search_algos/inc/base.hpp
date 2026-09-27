/**
 * @file base.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Base algorithms class header file
 */
#pragma once

extern "C"
{
#include "graphio.h"
}
#include <limits>
#include <string>
#include <vector>

namespace SP
{

enum class ReturnCode : int
{
    OK = 0,
    ERROR = 1,
    BAD_ARGUMENTS = 2
};

struct OutData
{
    // NaN while there is no result, infinity for an unreachable destination
    double shortestDistance = std::numeric_limits<double>::quiet_NaN();
    std::vector<int> shortestPath;
};

class BaseAlgo
{
  public:
    BaseAlgo(std::string graphFileName)
        : source(-1), destination(-1), graphFileName(graphFileName){};
    virtual ~BaseAlgo() = default;

    ReturnCode preProcess();
    virtual ReturnCode preProcessImpl();
    virtual ReturnCode setSrcDest(int source, int destination);

    ReturnCode compute();
    virtual ReturnCode computeImpl() = 0;

    const OutData &getResult() const { return outData; }
    int getCurrentSource() const { return source; }
    int getCurrentDestination() const { return destination; }

  protected:
    crsGraph graph{};
    int source;
    int destination;
    OutData outData;
    std::string graphFileName;

  private:
    bool sourceDestValidation();
    ReturnCode loadGraph();
};

} // namespace SP