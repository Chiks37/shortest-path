/**
 * @file base.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Base algorithms class source file
 */

#include "base.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

namespace SP
{
BaseAlgo::~BaseAlgo()
{
    if (graph.Xadj != nullptr)
    {
        free_graph_pointers(&graph);
    }
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

    // graphio leaves zero weights in a pattern matrix, while networkit and
    // GAPBS treat each of its edges as weighing 1
    int nz = graph.Xadj[graph.V];
    if (mm_is_pattern(graph.matcode))
    {
        std::fill(graph.Eweights, graph.Eweights + nz, 1.0);
    }

    for (int i = 0; i < nz; ++i)
    {
        if (!std::isfinite(graph.Eweights[i]) || graph.Eweights[i] < 0.0)
        {
            return ReturnCode::BAD_ARGUMENTS;
        }
    }

    return ReturnCode::OK;
}

ReturnCode BaseAlgo::preProcess()
{
    auto rc = preProcessImpl();
    if (ReturnCode::OK != rc)
    {
        // Every query is rejected on a graph without vertices, so nothing runs
        // on a graph that failed to load or to be preprocessed
        graph.V = 0;
    }
    return rc;
}

ReturnCode BaseAlgo::preProcessImpl() { return loadGraph(); }

ReturnCode BaseAlgo::setSrcDest(int source, int destination)
{
    this->source = source;
    this->destination = destination;
    if (sourceDestValidation())
    {
        return ReturnCode::OK;
    }

    // A rejected query must not leave the previous one ready to compute
    this->source = -1;
    this->destination = -1;
    return ReturnCode::BAD_ARGUMENTS;
}

ReturnCode BaseAlgo::compute()
{
    outData = OutData();
    if (!sourceDestValidation())
    {
        return ReturnCode::BAD_ARGUMENTS;
    }

    return computeImpl();
}

} // namespace SP