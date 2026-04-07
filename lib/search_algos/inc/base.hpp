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
#include <string>
#include <vector>

namespace SP
{

enum class ReturnCode : int
{
    OK = 0,
    ERROR = 1,
    BAD_ARGUMENTS = 2,
    NOT_CONFIGURED = 3,
    NOT_READY = 4
};

struct OutData
{
    double shortestDistance;
    std::vector<int> shortestPath;
};

enum class State
{
    UNCONFIGURED, // Not preprocessed
    READY,        // Preprocessed and ready to compute
    COMPUTED,     // Computations are done
    ERROR         // ERROR occured
};

class BaseAlgo
{
  public:
    BaseAlgo(std::string graphFileName)
        : currentState(State::UNCONFIGURED), graphFileName(graphFileName),
          source(0), destination(0){};
    virtual ~BaseAlgo() = default;

    ReturnCode preProcess();
    virtual ReturnCode preProcessImpl();
    virtual ReturnCode setSource(int source);
    ReturnCode setDestination(int destination);

    ReturnCode compute();
    virtual ReturnCode computeImpl() = 0;

    const OutData &getResult() const { return outData; }
    int getCurrentSource() const { return source; }
    int getCurrentDestination() const { return destination; }
    State getState() const { return currentState; }

  protected:
    crsGraph graph;
    int source;
    int destination;
    OutData outData;
    State currentState;
    std::string graphFileName;

  private:
    ReturnCode setVertex(int &vertex, int value);
    bool sourceDestValidation();
    ReturnCode loadGraph();
};

} // namespace SP