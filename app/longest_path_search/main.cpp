#include "custom_launcher.hpp"
#include <iostream>
#include <omp.h>
#include <string>

void testSearchingLongTimeProcessing(std::string graphFilename)
{
    const int source = 0;
    const auto algoId = SP::CustomLauncher::AlgoId::DIJKSTRA_SEQ;
    const std::string nodesMappingFilename = "none";

#pragma omp parallel default(none)                                             \
    firstprivate(source, graphFilename, algoId, nodesMappingFilename)          \
        shared(std::cout)
    {
        // Keep launcher state private for each worker thread.
        SP::CustomLauncher customLauncher(algoId, graphFilename,
                                          nodesMappingFilename);

#pragma omp for schedule(dynamic)
        for (int destination = 40; destination < 100; destination++)
        {
            customLauncher.execute(source, destination);
            const auto result = customLauncher.getResult();

            if (result.executionTimeMs > 1000)
            {
#pragma omp critical
                {
                    std::cout << "Route " << source + 1 << " -> "
                              << destination + 1 << " time is "
                              << result.executionTimeMs << " ms" << std::endl;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <.mtx file>" << std::endl;
        return 1;
    }

    std::string graphFilename(argv[1]);
    testSearchingLongTimeProcessing(graphFilename);
    return 0;
}
