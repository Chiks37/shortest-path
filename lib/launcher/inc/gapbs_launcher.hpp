/**
 * @file gapbs_launcher.hpp
 * @author tarakanov.2004@mail.ru
 * @brief GAPBS algorithms launcher class header file
 */
#pragma once

#include "launcher.hpp"

#include <array>
#include <unordered_map>

namespace SP
{

class GapbsLauncher : public Launcher
{
  public:
    enum class AlgoId
    {
        DIJKSTRA_SEQ,
        COUNT
    };

    static constexpr std::array<AlgoId, static_cast<size_t>(AlgoId::COUNT)>
        algoIds = {AlgoId::DIJKSTRA_SEQ};

    static inline const std::unordered_map<AlgoId, std::string> algoNames = {
        {AlgoId::DIJKSTRA_SEQ, "gapbs_dijkstra"}};

  private:
    AlgoId algoId;

  public:
    std::string getAlgoName() const override { return algoNames.at(algoId); }
    void execute(int source, int destination) override;

    explicit GapbsLauncher(const std::string &graphFileName,
                           AlgoId algoId = AlgoId::DIJKSTRA_SEQ)
        : Launcher(graphFileName), algoId(algoId)
    {
    }
};

} // namespace SP
