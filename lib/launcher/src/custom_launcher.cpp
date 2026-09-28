/**
 * @file custom_launcher.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Custom algorithms launcher class source file
 */

#include "custom_launcher.hpp"
#include <chrono>
#include <fstream>

namespace SP
{

std::shared_ptr<SP::BaseAlgo>
CustomLauncher::createAlgoObject(AlgoId algoId,
                                 const std::string &graphFileName,
                                 const std::string &nodesMappingFileName)
{
    switch (algoId)
    {
    case AlgoId::DIJKSTRA_SEQ:
        return std::make_shared<SP::DijkstraSeqAlgo>(graphFileName);
    case AlgoId::DIJKSTRA_SEQ_PAIRING_HEAP:
        return std::make_shared<SP::DijkstraSeqPairingHeapAlgo>(graphFileName);
    case AlgoId::DIJKSTRA_SEQ_BINOMIAL_HEAP:
        return std::make_shared<SP::DijkstraSeqBinomialHeapAlgo>(graphFileName);
    case AlgoId::DIJKSTRA_SEQ_D_ARY_HEAP:
        return std::make_shared<SP::DijkstraSeqDaryHeapAlgo>(graphFileName);
    case AlgoId::DIJKSTRA_BIDIR:
        return std::make_shared<SP::DijkstraBiDirAlgo>(graphFileName);
    case AlgoId::DIJKSTRA_PAR_EXPANSION:
        return std::make_shared<SP::DijkstraParExpansionAlgo>(graphFileName);
    case AlgoId::DIJKSTRA_PAR_RELAXATION:
        return std::make_shared<SP::DijkstraParRelaxationAlgo>(graphFileName);
    case AlgoId::DELTA_STEPPING:
        return std::make_shared<SP::DeltaSteppingAlgo>(graphFileName);
    case AlgoId::ASTARG:
        return std::make_shared<SP::AStarGAlgo>(graphFileName,
                                                nodesMappingFileName);
    case AlgoId::ALT:
        return std::make_shared<SP::ALTAlgo>(graphFileName);
    case AlgoId::ASTARG_DELTA:
        return std::make_shared<SP::AStarGDeltaAlgo>(graphFileName,
                                                     nodesMappingFileName);
    case AlgoId::ALT_DELTA:
        return std::make_shared<SP::ALTDeltaAlgo>(graphFileName);
    case AlgoId::ASTARG_BIDIR:
        return std::make_shared<SP::AStarGBiDirAlgo>(graphFileName,
                                                     nodesMappingFileName);
    case AlgoId::ALT_BIDIR:
        return std::make_shared<SP::ALTBiDirAlgo>(graphFileName);
    case AlgoId::DELTA_STEPPING_BIDIR:
        return std::make_shared<SP::DeltaSteppingBiDirAlgo>(graphFileName);
    case AlgoId::ASTARG_DELTA_BIDIR:
        return std::make_shared<SP::AStarGDeltaBiDirAlgo>(graphFileName,
                                                          nodesMappingFileName);
    case AlgoId::ALT_DELTA_BIDIR:
        return std::make_shared<SP::ALTDeltaBiDirAlgo>(graphFileName);
    case AlgoId::COUNT:
        break;
    }
    return nullptr;
}

std::string
CustomLauncher::findNodesMappingFile(const std::string &graphFileName)
{
    std::string base = graphFileName.substr(0, graphFileName.find_last_of('.'));
    std::string fileName = base + "_nodes_mapping.txt";
    if (!std::ifstream(fileName).is_open() &&
        std::ifstream(base + "_node_mapping.txt").is_open())
    {
        fileName = base + "_node_mapping.txt";
    }
    return fileName;
}

void CustomLauncher::execute(int source, int destination)
{
    auto algo = createAlgoObject(algoId, graphFileName, nodesMappingFileName);

    auto preStart = std::chrono::steady_clock::now();
    algo->preProcess();
    algo->setSrcDest(source, destination);
    auto preEnd = std::chrono::steady_clock::now();
    lastResult.preProcessTimeMs =
        std::chrono::duration<double, std::milli>(preEnd - preStart).count();

    auto computeStart = std::chrono::steady_clock::now();
    algo->compute();
    auto computeEnd = std::chrono::steady_clock::now();
    lastResult.executionTimeMs =
        std::chrono::duration<double, std::milli>(computeEnd - computeStart)
            .count();

    const auto &result = algo->getResult();
    lastResult.shortestPath = result.shortestPath;
    lastResult.shortestDistance = result.shortestDistance;
}

} // namespace SP
