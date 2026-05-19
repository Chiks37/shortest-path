/**
 * @file custom_launcher.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Custom algorithms launcher class header file
 */
#pragma once

#include "alt.hpp"
#include "alt_delta.hpp"
#include "astarg.hpp"
#include "astarg_delta.hpp"
#include "delta_stepping.hpp"
#include "dijkstra_bidir.hpp"
#include "dijkstra_par_expansion.hpp"
#include "dijkstra_par_relaxation.hpp"
#include "dijkstra_seq.hpp"
#include "launcher.hpp"

#include <array>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace SP
{

class CustomLauncher : public Launcher
{
  public:
    enum class AlgoId
    {
        DIJKSTRA_SEQ,
        DIJKSTRA_BIDIR,
        DIJKSTRA_PAR_EXPANSION,
        DIJKSTRA_PAR_RELAXATION,
        DELTA_STEPPING,
        ASTARG,
        ALT,
        ASTARG_DELTA,
        ALT_DELTA,
        COUNT
    };

    static constexpr std::array<AlgoId, static_cast<size_t>(AlgoId::COUNT)>
        algoIds = {AlgoId::DIJKSTRA_SEQ,
                   AlgoId::DIJKSTRA_BIDIR,
                   AlgoId::DIJKSTRA_PAR_EXPANSION,
                   AlgoId::DIJKSTRA_PAR_RELAXATION,
                   AlgoId::DELTA_STEPPING,
                   AlgoId::ASTARG,
                   AlgoId::ALT,
                   AlgoId::ASTARG_DELTA,
                   AlgoId::ALT_DELTA};

    static inline const std::unordered_map<AlgoId, std::string> algoNames = {
        {AlgoId::DIJKSTRA_SEQ, "dijkstra_seq"},
        {AlgoId::DIJKSTRA_BIDIR, "dijkstra_bidir"},
        {AlgoId::DIJKSTRA_PAR_EXPANSION, "dijkstra_par_expansion"},
        {AlgoId::DIJKSTRA_PAR_RELAXATION, "dijkstra_par_relaxation"},
        {AlgoId::DELTA_STEPPING, "delta_stepping"},
        {AlgoId::ASTARG, "astarg"},
        {AlgoId::ALT, "alt"},
        {AlgoId::ASTARG_DELTA, "astarg_delta"},
        {AlgoId::ALT_DELTA, "alt_delta"}};

  private:
    AlgoId algoId;
    std::string nodesMappingFileName;
    std::shared_ptr<SP::BaseAlgo> createAlgoObject();

  public:
    std::string getAlgoName() const override { return algoNames.at(algoId); }
    void execute(int source, int destination) override;

    CustomLauncher(AlgoId algoId, const std::string &graphFileName,
                   const std::string &nodesMappingFileName = "")
        : Launcher(graphFileName), algoId(algoId),
          nodesMappingFileName(nodesMappingFileName)
    {
    }
};

} // namespace SP
