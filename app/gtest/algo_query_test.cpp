#include "custom_launcher.hpp"
#include "networkit_launcher.hpp"
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace
{
using SP::CustomLauncher;
using AlgoId = CustomLauncher::AlgoId;

const std::string kGraphPath = TEST_GRAPH_PATH;
const std::string kNodesMappingPath =
    kGraphPath.substr(0, kGraphPath.find_last_of('.')) + "_nodes_mapping.txt";
const double kInf = std::numeric_limits<double>::infinity();

// The test graph read by graphio directly, to check the returned paths
class TestGraph
{
  public:
    explicit TestGraph(const std::string &fileName)
    {
        read_mtx_to_crs(&graph, fileName.c_str());
    }
    TestGraph(const TestGraph &) = delete;
    TestGraph &operator=(const TestGraph &) = delete;
    ~TestGraph() { free_graph_pointers(&graph); }

    int vertexCount() const { return graph.V; }

    // Weight of the lightest edge from -> to, infinity if there is none
    double edgeWeight(int from, int to) const
    {
        double weight = kInf;
        for (int i = graph.Xadj[from]; i < graph.Xadj[from + 1]; ++i)
        {
            if (graph.Adjncy[i] == to)
            {
                weight = std::min(weight, graph.Eweights[i]);
            }
        }
        return weight;
    }

  private:
    crsGraph graph{};
};

const TestGraph &testGraph()
{
    static const TestGraph graph(kGraphPath);
    return graph;
}

struct Query
{
    int source;
    int destination;
    // Distance found by DijkstraSeqAlgo, infinity if unreachable
    double distance;
};

// Random pairs, every tenth one with the source equal to the destination
std::vector<Query> makeQueries(const std::string &graphFileName, int count)
{
    SP::DijkstraSeqAlgo reference(graphFileName);
    reference.preProcess();

    std::mt19937 generator(42);
    std::uniform_int_distribution<int> vertex(0, testGraph().vertexCount() - 1);
    std::vector<Query> queries;
    for (int i = 0; i < count; ++i)
    {
        int source = vertex(generator);
        int destination = i % 10 == 0 ? source : vertex(generator);
        reference.setSrcDest(source, destination);
        reference.compute();
        queries.push_back(
            {source, destination, reference.getResult().shortestDistance});
    }
    return queries;
}

const std::vector<Query> &queries()
{
    static const std::vector<Query> result = makeQueries(kGraphPath, 200);
    return result;
}

bool sameDistance(double actual, double expected)
{
    if (std::isinf(actual) || std::isinf(expected))
    {
        return actual == expected;
    }
    return std::fabs(actual - expected) <=
           1e-9 * std::max(1.0, std::fabs(expected));
}

// The expected distance and a path of that length from the source to the
// destination along the graph edges, or no path for an unreachable destination
testing::AssertionResult isShortestPath(const TestGraph &graph,
                                        const SP::OutData &result,
                                        const Query &query)
{
    const auto &path = result.shortestPath;
    if (!sameDistance(result.shortestDistance, query.distance))
    {
        return testing::AssertionFailure()
               << "distance " << result.shortestDistance << ", expected "
               << query.distance;
    }
    if (std::isinf(query.distance))
    {
        if (!path.empty())
        {
            return testing::AssertionFailure()
                   << "path of " << path.size()
                   << " vertices to an unreachable destination";
        }
        return testing::AssertionSuccess();
    }
    if (path.empty() || path.front() != query.source ||
        path.back() != query.destination)
    {
        return testing::AssertionFailure()
               << "path does not lead from the source to the destination";
    }

    double length = 0.0;
    for (size_t i = 0; i + 1 < path.size(); ++i)
    {
        if (path[i + 1] < 0 || path[i + 1] >= graph.vertexCount())
        {
            return testing::AssertionFailure()
                   << "vertex " << path[i + 1] << " is out of the graph";
        }
        length += graph.edgeWeight(path[i], path[i + 1]);
    }
    if (!sameDistance(length, query.distance))
    {
        return testing::AssertionFailure()
               << "path length " << length << ", expected " << query.distance;
    }
    return testing::AssertionSuccess();
}

// Temporary file removed together with the object
class TempFile
{
  public:
    TempFile(const std::string &name, const std::string &content)
        : path((std::filesystem::temp_directory_path() / name).string())
    {
        std::ofstream(path) << content;
    }
    ~TempFile() { std::filesystem::remove(path); }

    const std::string path;
};

class AlgoQueryTest : public testing::TestWithParam<AlgoId>
{
  protected:
    std::shared_ptr<SP::BaseAlgo> createAlgo()
    {
        auto algo = CustomLauncher::createAlgoObject(GetParam(), kGraphPath,
                                                     kNodesMappingPath);
        EXPECT_EQ(algo->preProcess(), SP::ReturnCode::OK);
        return algo;
    }
};

// One object answers all queries in a row, including pairs with the source
// equal to the destination and unreachable destinations
TEST_P(AlgoQueryTest, AnswersQueriesInARow)
{
    auto algo = createAlgo();
    for (const auto &query : queries())
    {
        ASSERT_EQ(algo->setSrcDest(query.source, query.destination),
                  SP::ReturnCode::OK);
        ASSERT_EQ(algo->compute(), SP::ReturnCode::OK);
        EXPECT_TRUE(isShortestPath(testGraph(), algo->getResult(), query))
            << query.source << " -> " << query.destination;
    }
}

TEST_P(AlgoQueryTest, RepeatedComputeGivesSameAnswer)
{
    auto algo = createAlgo();
    for (size_t i = 0; i < 20; ++i)
    {
        const auto &query = queries()[i];
        ASSERT_EQ(algo->setSrcDest(query.source, query.destination),
                  SP::ReturnCode::OK);
        ASSERT_EQ(algo->compute(), SP::ReturnCode::OK);
        ASSERT_EQ(algo->compute(), SP::ReturnCode::OK);
        EXPECT_TRUE(isShortestPath(testGraph(), algo->getResult(), query))
            << query.source << " -> " << query.destination;
    }
}

TEST_P(AlgoQueryTest, RejectsInvalidVertices)
{
    auto algo = createAlgo();
    auto expectRejected = [&algo]()
    {
        EXPECT_EQ(algo->compute(), SP::ReturnCode::BAD_ARGUMENTS);
        EXPECT_TRUE(std::isnan(algo->getResult().shortestDistance));
        EXPECT_TRUE(algo->getResult().shortestPath.empty());
    };

    EXPECT_EQ(algo->setSrcDest(0, testGraph().vertexCount()),
              SP::ReturnCode::BAD_ARGUMENTS);
    expectRejected();

    // A rejected query must not reuse the one computed before it
    const auto &query = queries()[1];
    ASSERT_EQ(algo->setSrcDest(query.source, query.destination),
              SP::ReturnCode::OK);
    ASSERT_EQ(algo->compute(), SP::ReturnCode::OK);
    EXPECT_EQ(algo->setSrcDest(query.destination, -1),
              SP::ReturnCode::BAD_ARGUMENTS);
    expectRejected();
}

INSTANTIATE_TEST_SUITE_P(AllAlgos, AlgoQueryTest,
                         testing::ValuesIn(CustomLauncher::algoIds),
                         [](const testing::TestParamInfo<AlgoId> &info)
                         { return CustomLauncher::algoNames.at(info.param); });

// The reference of the tests above agrees with networkit on distances and
// paths, including unreachable destinations
TEST(ReferenceTest, NetworkitAgreesWithDijkstra)
{
    // networkit reads the graph on every query, so only a part of the queries
    std::vector<Query> checked(queries().begin(), queries().begin() + 20);
    for (const auto &query : queries())
    {
        if (std::isinf(query.distance))
        {
            checked.push_back(query);
        }
    }

    for (auto algoId : SP::NetworkitLauncher::algoIds)
    {
        for (const auto &query : checked)
        {
            SP::NetworkitLauncher networkit(kGraphPath, algoId,
                                            kNodesMappingPath);
            networkit.execute(query.source, query.destination);
            SP::OutData result{networkit.getResult().shortestDistance,
                               networkit.getResult().shortestPath};
            EXPECT_TRUE(isShortestPath(testGraph(), result, query))
                << SP::NetworkitLauncher::algoNames.at(algoId) << ": "
                << query.source << " -> " << query.destination;
        }
    }
}

TEST(ReferenceTest, NetworkitRejectsInvalidVertices)
{
    for (auto algoId : SP::NetworkitLauncher::algoIds)
    {
        SP::NetworkitLauncher networkit(kGraphPath, algoId, kNodesMappingPath);
        networkit.execute(0, testGraph().vertexCount());
        EXPECT_TRUE(std::isnan(networkit.getResult().shortestDistance));
        EXPECT_TRUE(networkit.getResult().shortestPath.empty());
    }
}

TEST(GraphLoadingTest, PatternMatrixHasUnitWeights)
{
    // Path 1-2-3-4-5 with a chord 2-4
    TempFile file("sp_tests_pattern.mtx",
                  "%%MatrixMarket matrix coordinate pattern symmetric\n"
                  "5 5 5\n2 1\n3 2\n4 3\n5 4\n4 2\n");
    for (AlgoId algoId :
         {AlgoId::DIJKSTRA_SEQ, AlgoId::DELTA_STEPPING, AlgoId::ALT})
    {
        auto algo = CustomLauncher::createAlgoObject(algoId, file.path);
        ASSERT_EQ(algo->preProcess(), SP::ReturnCode::OK);
        ASSERT_EQ(algo->setSrcDest(0, 4), SP::ReturnCode::OK);
        ASSERT_EQ(algo->compute(), SP::ReturnCode::OK);
        EXPECT_EQ(algo->getResult().shortestDistance, 3.0)
            << CustomLauncher::algoNames.at(algoId);
    }
}

TEST(GraphLoadingTest, NegativeWeightIsRejected)
{
    TempFile file("sp_tests_negative.mtx",
                  "%%MatrixMarket matrix coordinate real general\n"
                  "3 3 2\n1 2 1.5\n2 3 -1\n");
    SP::DijkstraSeqAlgo algo(file.path);
    EXPECT_EQ(algo.preProcess(), SP::ReturnCode::BAD_ARGUMENTS);
    EXPECT_EQ(algo.setSrcDest(0, 2), SP::ReturnCode::BAD_ARGUMENTS);
    EXPECT_EQ(algo.compute(), SP::ReturnCode::BAD_ARGUMENTS);
    EXPECT_TRUE(std::isnan(algo.getResult().shortestDistance));
}

const AlgoId kGeometricAlgos[] = {AlgoId::ASTARG, AlgoId::ASTARG_BIDIR,
                                  AlgoId::ASTARG_DELTA,
                                  AlgoId::ASTARG_DELTA_BIDIR};

// Weights in kilometres with coordinates in metres: an unscaled heuristic
// would overestimate distances a thousand times
TEST(GeometricHeuristicTest, WeightsInOtherUnits)
{
    std::ifstream input(kGraphPath);
    std::ostringstream scaled;
    scaled.precision(17);
    std::string line;
    // Banner, comments and the size line are copied as is
    while (std::getline(input, line) && (line.empty() || line[0] == '%'))
    {
        scaled << line << '\n';
    }
    scaled << line << '\n';
    int row, column;
    double weight;
    while (input >> row >> column >> weight)
    {
        scaled << row << ' ' << column << ' ' << weight / 1000.0 << '\n';
    }
    TempFile file("sp_tests_km.mtx", scaled.str());
    TestGraph kmGraph(file.path);

    std::vector<Query> kmQueries = makeQueries(file.path, 100);
    for (AlgoId algoId : kGeometricAlgos)
    {
        auto algo = CustomLauncher::createAlgoObject(algoId, file.path,
                                                     kNodesMappingPath);
        ASSERT_EQ(algo->preProcess(), SP::ReturnCode::OK);
        for (const auto &query : kmQueries)
        {
            algo->setSrcDest(query.source, query.destination);
            algo->compute();
            EXPECT_TRUE(isShortestPath(kmGraph, algo->getResult(), query))
                << CustomLauncher::algoNames.at(algoId) << ": " << query.source
                << " -> " << query.destination;
        }
    }

    // networkit reads the graph on every query, so only a part of the queries
    for (int i = 0; i < 20; ++i)
    {
        const auto &query = kmQueries[i];
        SP::NetworkitLauncher networkit(file.path,
                                        SP::NetworkitLauncher::AlgoId::ASTARG,
                                        kNodesMappingPath);
        networkit.execute(query.source, query.destination);
        SP::OutData result{networkit.getResult().shortestDistance,
                           networkit.getResult().shortestPath};
        EXPECT_TRUE(isShortestPath(kmGraph, result, query))
            << "networkit_astar: " << query.source << " -> "
            << query.destination;
    }
}

TEST(GeometricHeuristicTest, MissingOrShortCoordinatesAreRejected)
{
    std::ifstream mapping(kNodesMappingPath);
    std::string shortMapping, line;
    for (int i = 0; i < 100 && std::getline(mapping, line); ++i)
    {
        shortMapping += line + '\n';
    }
    TempFile shortFile("sp_tests_short_mapping.txt", shortMapping);

    for (const std::string &mappingPath :
         {std::string("/nonexistent/mapping.txt"), shortFile.path})
    {
        for (AlgoId algoId : kGeometricAlgos)
        {
            auto algo = CustomLauncher::createAlgoObject(algoId, kGraphPath,
                                                         mappingPath);
            EXPECT_EQ(algo->preProcess(), SP::ReturnCode::BAD_ARGUMENTS);
            EXPECT_EQ(algo->setSrcDest(0, 10), SP::ReturnCode::BAD_ARGUMENTS);
            EXPECT_EQ(algo->compute(), SP::ReturnCode::BAD_ARGUMENTS);
            EXPECT_TRUE(std::isnan(algo->getResult().shortestDistance));
        }

        SP::NetworkitLauncher networkit(
            kGraphPath, SP::NetworkitLauncher::AlgoId::ASTARG, mappingPath);
        networkit.execute(0, 10);
        EXPECT_TRUE(std::isnan(networkit.getResult().shortestDistance));
        EXPECT_TRUE(networkit.getResult().shortestPath.empty());
    }
}
} // namespace
