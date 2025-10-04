/**
 * @file algo.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Base algorithms class source file
 */

#include "base.hpp"
#include <fstream>
#include <iostream>

namespace SP
{
    ReturnCode BaseAlgo::preProcess()
    {
        init_graph(&graph);

        std::ifstream file(data.graphFileName);
        if (!file.is_open())
        {
            std::cout << "Failed to open graph file (no such file?)" << std::endl;
            return ReturnCode::ERROR;
        }

        // Read the graph from the file
        read_mtx_to_crs(&graph, data.graphFileName.c_str());
        std::cout << "A graph with " << graph.V << " vertexes and " << graph.nz / 2 << " edges is loaded\n";

        file.close();

        return ReturnCode::OK;
    }

    ReturnCode BaseAlgo::postProcess()
    {
        std::cout << "Algorithm execution completed\n";
        return ReturnCode::OK;
    }

    void BaseAlgo::execute()
    {
        ReturnCode rc = preProcess();
        rc = (ReturnCode::OK != rc) ? rc : process();
        rc = (ReturnCode::OK != rc) ? rc : postProcess();

        if (rc != ReturnCode::OK)
        {
            std::cout << "FAILED" << std::endl;
        }
    }
} // namespace SP