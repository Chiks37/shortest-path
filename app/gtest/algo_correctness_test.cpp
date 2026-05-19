#include "custom_launcher.hpp"
#include "networkit_launcher.hpp"
#include <gtest/gtest.h>
#include <string>

static const std::string kGraphPath = TEST_GRAPH_PATH;
static const std::string kNodesMappingPath =
    kGraphPath.substr(0, kGraphPath.find_last_of('.')) + "_nodes_mapping.txt";

struct Route
{
    int source;
    int destination;
    std::string name() const
    {
        return std::to_string(source) + "_to_" + std::to_string(destination);
    }
};

static const Route kTestRoutes[] = {
    {0, 10},
    {0, 50},
};

class AlgoCorrectnessTest : public testing::TestWithParam<
                                std::tuple<SP::CustomLauncher::AlgoId, Route>>
{
};

TEST_P(AlgoCorrectnessTest, DistanceMatchesNetworkit)
{
    auto [algoId, route] = GetParam();

    SP::NetworkitLauncher reference(
        kGraphPath, SP::NetworkitLauncher::AlgoId::DIJKSTRA_SEQ);
    reference.execute(route.source, route.destination);
    const double expected = reference.getResult().shortestDistance;

    SP::CustomLauncher algo(algoId, kGraphPath, kNodesMappingPath);
    algo.execute(route.source, route.destination);
    const double actual = algo.getResult().shortestDistance;

    EXPECT_NEAR(actual, expected, 0.1);
}

INSTANTIATE_TEST_SUITE_P(
    AllAlgos, AlgoCorrectnessTest,
    testing::Combine(testing::ValuesIn(SP::CustomLauncher::algoIds),
                     testing::ValuesIn(kTestRoutes)),
    [](const testing::TestParamInfo<AlgoCorrectnessTest::ParamType> &info)
    {
        return SP::CustomLauncher::algoNames.at(std::get<0>(info.param)) + "_" +
               std::get<1>(info.param).name();
    });
