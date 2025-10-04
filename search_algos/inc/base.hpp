/**
 * @file algo.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Base algorithms class header file
 */
#pragma once

extern "C" {
    #include "graphio.h"
}
#include <vector>
#include <string>

namespace SP {

enum ReturnCode : int
{
    OK = 0,
    ERROR = 1
};

class BaseAlgo {
 public:

    struct AlgoInData
    {
    std::string graphFileName;
    int source;
    int destination;
    int& shortestDistance;
    std::vector<int>& shortestPath;
    };

    BaseAlgo(AlgoInData inData) : data(inData) {}
    virtual ~BaseAlgo() {}

    virtual void execute();

 protected:
    virtual ReturnCode preProcess();
    virtual ReturnCode process() = 0;
    virtual ReturnCode postProcess();

    crsGraph graph;
    AlgoInData data;
};

} // namespace SP