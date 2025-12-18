/**
 * @file base.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Base algorithms class source file
 */

#include "base.hpp"
#include <fstream>
#include <iostream>

namespace SP
{
ReturnCode BaseAlgo::getGraphLink(const crsGraph & graph)
{
    if (State::UNCONFIGURED == currentState)
    {
        return ReturnCode::NOT_CONFIGURED;
    }

    return ReturnCode::OK;
}

ReturnCode BaseAlgo::setVertex(int &vertex, int value)
{
    if (State::READY != currentState && State::COMPUTED != currentState)
    {
        return ReturnCode::NOT_READY;
    }

    vertex = value;
    if (true != sourceDestValidation())
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    currentState = State::READY; // If it was computed this say that the out data is not actual

    return ReturnCode::OK;
}

bool BaseAlgo::sourceDestValidation()
{
    bool rc = true;
    if (source < 0 || source >= graph.V || destination < 0 ||
        destination >= graph.V)
    {
        rc = false;
    }

    return rc;
}

ReturnCode BaseAlgo::loadGraph()
{
    if (0 != init_graph(&graph))
    {
        return ReturnCode::ERROR;
    }

    std::ifstream file(graphFileName);
    if (!file.is_open())
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    // Read the graph from the file
    if (0 != read_mtx_to_crs(&graph, graphFileName.c_str()))
    {
        return ReturnCode::ERROR;
    }

    file.close();

    if (true != sourceDestValidation())
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    return ReturnCode::OK;
}

ReturnCode BaseAlgo::preProcess()
{
    auto rc = preProcessImpl();
    currentState = ReturnCode::OK == rc ? State::READY : State::ERROR;
    return rc;
}

ReturnCode BaseAlgo::preProcessImpl()
{
    if (State::UNCONFIGURED != currentState) // Graph is already loaded
    {
        return ReturnCode::OK;
    }
    return loadGraph();
}

ReturnCode BaseAlgo::setSource(int source)
{
    return setVertex(this->source, source);
}

ReturnCode BaseAlgo::setDestination(int destination)
{
    return setVertex(this->destination, destination);
}

ReturnCode BaseAlgo::compute()
{
    if (currentState != State::READY)
    {
        return ReturnCode::NOT_READY;
    }

    auto rc = computeImpl();

    currentState = ReturnCode::OK == rc ? State::COMPUTED : State::ERROR;
    return rc;
}

} // namespace SP