/**
 * @file main.cpp
 * @brief Research benchmark harness for the diploma's experimental chapter.
 *
 * Drives BaseAlgo subclasses directly (NOT via CustomLauncher), so the graph is
 * loaded and the algorithm is pre-processed exactly once per process; afterwards
 * many (source, destination) queries are timed by repeating setSrcDest+compute.
 *
 * Two modes:
 *   genpairs  -- run DijkstraSsspAlgo from K random sources, emit query pairs
 *                annotated with their Dijkstra rank and reference distance.
 *   query     -- time one algorithm on a precomputed pairs file.
 *
 * All vertex ids are 0-indexed (internal representation). CSV is flushed per
 * row so that a process killed by timeout/ulimit still leaves partial results.
 */

#include "custom_launcher.hpp"
#include "dijkstra_sssp.hpp"
#include "gapbs_launcher.hpp"
#include "networkit_launcher.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <omp.h>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace
{

using Clock = std::chrono::high_resolution_clock;

double msSince(const Clock::time_point &start)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - start)
        .count();
}

std::unique_ptr<SP::BaseAlgo> makeCustomAlgo(const std::string &name,
                                             const std::string &graph,
                                             const std::string &mapping)
{
    using namespace SP;
    if (name == "dijkstra_seq")
        return std::make_unique<DijkstraSeqAlgo>(graph);
    if (name == "dijkstra_seq_pairing_heap")
        return std::make_unique<DijkstraSeqPairingHeapAlgo>(graph);
    if (name == "dijkstra_seq_binomial_heap")
        return std::make_unique<DijkstraSeqBinomialHeapAlgo>(graph);
    if (name == "dijkstra_seq_d_ary_heap")
        return std::make_unique<DijkstraSeqDaryHeapAlgo>(graph);
    if (name == "dijkstra_bidir")
        return std::make_unique<DijkstraBiDirAlgo>(graph);
    if (name == "dijkstra_par_expansion")
        return std::make_unique<DijkstraParExpansionAlgo>(graph);
    if (name == "dijkstra_par_relaxation")
        return std::make_unique<DijkstraParRelaxationAlgo>(graph);
    if (name == "delta_stepping")
        return std::make_unique<DeltaSteppingAlgo>(graph);
    if (name == "astarg")
        return std::make_unique<AStarGAlgo>(graph, mapping);
    if (name == "alt")
        return std::make_unique<ALTAlgo>(graph);
    if (name == "astarg_delta")
        return std::make_unique<AStarGDeltaAlgo>(graph, mapping);
    if (name == "alt_delta")
        return std::make_unique<ALTDeltaAlgo>(graph);
    if (name == "astarg_bidir")
        return std::make_unique<AStarGBiDirAlgo>(graph, mapping);
    if (name == "alt_bidir")
        return std::make_unique<ALTBiDirAlgo>(graph);
    if (name == "delta_stepping_bidir")
        return std::make_unique<DeltaSteppingBiDirAlgo>(graph);
    if (name == "astarg_delta_bidir")
        return std::make_unique<AStarGDeltaBiDirAlgo>(graph, mapping);
    if (name == "alt_delta_bidir")
        return std::make_unique<ALTDeltaBiDirAlgo>(graph);
    return nullptr;
}

bool isReferenceAlgo(const std::string &name)
{
    return name == "networkit_dijkstra" || name == "gapbs_dijkstra";
}

std::string baseName(const std::string &path)
{
    auto slash = path.find_last_of('/');
    std::string f = (slash == std::string::npos) ? path : path.substr(slash + 1);
    auto dot = f.find_last_of('.');
    return (dot == std::string::npos) ? f : f.substr(0, dot);
}

double median(std::vector<double> v)
{
    if (v.empty())
        return -1.0;
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    return (n % 2) ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

bool distancesMatch(double a, double r)
{
    if (std::isinf(a) && std::isinf(r))
        return true;
    if (std::isinf(a) || std::isinf(r))
        return false;
    return std::fabs(a - r) <= 1e-4 * std::max(1.0, std::fabs(r)) + 1e-6;
}

// ------------------------------- arg parsing -------------------------------

std::map<std::string, std::string> parseArgs(int argc, char **argv, int start)
{
    std::map<std::string, std::string> m;
    for (int i = start; i + 1 < argc; i += 2)
    {
        std::string key = argv[i];
        if (key.rfind("--", 0) == 0)
            m[key.substr(2)] = argv[i + 1];
    }
    return m;
}

std::string get(const std::map<std::string, std::string> &m,
                const std::string &k, const std::string &def = "")
{
    auto it = m.find(k);
    return it == m.end() ? def : it->second;
}

// -------------------------------- weightgen --------------------------------
// Reads a (possibly pattern/unweighted) .mtx and writes a 3-column weighted
// .mtx. With --mapping, each edge weight is the Euclidean distance between its
// endpoints' coordinates (columns 5,6 of the mapping file, vertex = line
// order) -- this is exactly the space the A*G heuristic uses, so the resulting
// weights make A*G admissible. Without --mapping, every edge weight is 1.0.
// The output preserves the original vertex/edge structure and the
// symmetric/general flag, so graphio and networkit/GAPBS all read the same
// weighted graph.

int runWeightGen(const std::map<std::string, std::string> &a)
{
    const std::string in = get(a, "graph");
    const std::string mapping = get(a, "mapping");
    const std::string out = get(a, "out");
    if (in.empty() || out.empty())
    {
        std::cerr << "weightgen needs --graph and --out\n";
        return 1;
    }
    const bool unit = mapping.empty();

    std::vector<double> cx, cy;
    if (!unit)
    {
        std::ifstream ms(mapping);
        if (!ms)
        {
            std::cerr << "cannot open mapping " << mapping << "\n";
            return 1;
        }
        std::string line;
        while (std::getline(ms, line))
        {
            if (line.empty())
                continue;
            double x = 0.0, y = 0.0;
            // four leading filler tokens, then x y (matches A*G loader)
            if (std::sscanf(line.c_str(), "%*s %*s %*s %*s %lf %lf", &x, &y) == 2)
            {
                cx.push_back(x);
                cy.push_back(y);
            }
        }
        std::cerr << "weightgen: read " << cx.size() << " coordinates\n";
    }

    std::FILE *fi = std::fopen(in.c_str(), "r");
    if (!fi)
    {
        std::cerr << "cannot open " << in << "\n";
        return 1;
    }
    char banner[4096];
    bool symmetric = false;
    // Copy through comment/banner lines; capture symmetry from the %%MatrixMarket
    // line; stop at the dimension line.
    long long V = 0, N = 0, nnz = 0;
    {
        char buf[1 << 16];
        bool gotDims = false;
        std::string firstBanner;
        while (std::fgets(buf, sizeof(buf), fi))
        {
            if (buf[0] == '%')
            {
                if (firstBanner.empty())
                {
                    firstBanner = buf;
                    if (firstBanner.find("symmetric") != std::string::npos)
                        symmetric = true;
                }
                continue;
            }
            if (std::sscanf(buf, "%lld %lld %lld", &V, &N, &nnz) == 3)
            {
                gotDims = true;
                break;
            }
        }
        if (!gotDims)
        {
            std::cerr << "could not read dimensions from " << in << "\n";
            std::fclose(fi);
            return 1;
        }
        (void)banner;
    }

    std::FILE *fo = std::fopen(out.c_str(), "w");
    if (!fo)
    {
        std::cerr << "cannot open output " << out << "\n";
        std::fclose(fi);
        return 1;
    }
    std::fprintf(fo, "%%%%MatrixMarket matrix coordinate real %s\n",
                 symmetric ? "symmetric" : "general");
    std::fprintf(fo, "%lld %lld %lld\n", V, N, nnz);

    long long u, v, written = 0;
    for (long long i = 0; i < nnz; ++i)
    {
        if (std::fscanf(fi, "%lld %lld", &u, &v) != 2)
            break;
        // skip any trailing tokens (old weight) up to end of line
        int c;
        while ((c = std::fgetc(fi)) != '\n' && c != EOF)
            ;
        double w;
        if (unit)
            w = 1.0;
        else
        {
            long long iu = u - 1, iv = v - 1;
            if (iu == iv)
                w = 0.0;
            else if (iu >= 0 && iv >= 0 && iu < (long long)cx.size() &&
                     iv < (long long)cx.size())
                w = std::hypot(cx[iu] - cx[iv], cy[iu] - cy[iv]);
            else
                w = 1.0; // fallback if a vertex lacks coordinates
        }
        std::fprintf(fo, "%lld %lld %.10g\n", u, v, w);
        ++written;
    }
    std::fclose(fi);
    std::fclose(fo);
    std::cerr << "weightgen done: " << written << " edges (" << (unit ? "unit" : "geometric")
              << (symmetric ? ", symmetric" : ", general") << ")\n";
    return 0;
}

// --------------------------------- genpairs --------------------------------

int runGenPairs(const std::map<std::string, std::string> &a)
{
    const std::string graph = get(a, "graph");
    const unsigned long long seed = std::stoull(get(a, "seed", "42"));
    const int sources = std::stoi(get(a, "sources", "16"));
    const int randomPer = std::stoi(get(a, "random", "3"));
    const std::string out = get(a, "out");
    if (graph.empty() || out.empty())
    {
        std::cerr << "genpairs needs --graph and --out\n";
        return 1;
    }

    SP::DijkstraSsspAlgo sssp(graph);
    if (sssp.preProcess() != SP::ReturnCode::OK)
    {
        std::cerr << "PREPROCESS_FAILED for genpairs on " << graph << "\n";
        return 2;
    }

    // One SSSP from vertex 0 to learn the vertex count.
    // NOTE: we call the virtual setSrcDest(s, s) (which dispatches to
    // AbstractDijkstraAlgo::setSrcDest and properly re-initializes distances /
    // seeds the queue) instead of DijkstraSsspAlgo::setSource, which routes
    // through BaseAlgo::setSrcDest and skips that initialization.
    sssp.setSrcDest(0, 0);
    sssp.compute();
    std::vector<double> d0 = sssp.getDistances();
    const int V = static_cast<int>(d0.size());
    if (V <= 0)
    {
        std::cerr << "empty graph\n";
        return 2;
    }

    std::ofstream os(out);
    os << "src,dst,rank,ref_dist,category\n";

    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<int> pick(0, V - 1);

    auto emitForSource = [&](int s, const std::vector<double> &dist)
    {
        std::vector<std::pair<double, int>> reach;
        reach.reserve(dist.size());
        for (int v = 0; v < V; ++v)
            if (v != s && std::isfinite(dist[v]))
                reach.emplace_back(dist[v], v);
        if (reach.size() < 2)
            return;
        std::sort(reach.begin(), reach.end());
        const int R = static_cast<int>(reach.size());

        // Rank targets at powers of two: 2^4, 2^5, ... < R.
        for (int e = 4; (1 << e) < R; ++e)
        {
            int pos = 1 << e;
            os << s << ',' << reach[pos].second << ',' << pos << ','
               << reach[pos].first << ",rank\n";
        }
        // A few uniformly-random reachable targets.
        std::uniform_int_distribution<int> rpos(1, R - 1);
        for (int j = 0; j < randomPer; ++j)
        {
            int pos = rpos(gen);
            os << s << ',' << reach[pos].second << ',' << pos << ','
               << reach[pos].first << ",random\n";
        }
    };

    for (int k = 0; k < sources; ++k)
    {
        int s = pick(gen);
        sssp.setSrcDest(s, s);
        sssp.compute();
        std::vector<double> dist = sssp.getDistances();
        emitForSource(s, dist);
        os.flush();
    }
    std::cerr << "genpairs done: " << sources << " sources, V=" << V << "\n";
    return 0;
}

// ----------------------------------- query ---------------------------------

struct Pair
{
    int src, dst;
    long long rank;
    double refDist;
    std::string category;
};

std::vector<Pair> readPairs(const std::string &path)
{
    std::vector<Pair> pairs;
    std::ifstream is(path);
    std::string line;
    std::getline(is, line); // header
    while (std::getline(is, line))
    {
        if (line.empty())
            continue;
        std::stringstream ss(line);
        std::string tok;
        Pair p;
        std::getline(ss, tok, ',');
        p.src = std::stoi(tok);
        std::getline(ss, tok, ',');
        p.dst = std::stoi(tok);
        std::getline(ss, tok, ',');
        p.rank = std::stoll(tok);
        std::getline(ss, tok, ',');
        p.refDist = std::stod(tok);
        std::getline(ss, p.category, ',');
        pairs.push_back(p);
    }
    return pairs;
}

int runQuery(const std::map<std::string, std::string> &a)
{
    const std::string graph = get(a, "graph");
    const std::string mapping = get(a, "mapping");
    const std::string algo = get(a, "algo");
    const std::string pairsFile = get(a, "pairs");
    const std::string out = get(a, "out");
    const int reps = std::stoi(get(a, "reps", "3"));
    const int warmup = std::stoi(get(a, "warmup", "1"));
    const int threads = std::stoi(get(a, "threads", "0"));
    const std::string filter = get(a, "category", ""); // "", "rank", "random"

    if (graph.empty() || algo.empty() || pairsFile.empty() || out.empty())
    {
        std::cerr << "query needs --graph --algo --pairs --out\n";
        return 1;
    }
    if (threads > 0)
        omp_set_num_threads(threads);
    const int usedThreads = omp_get_max_threads();

    std::vector<Pair> pairs = readPairs(pairsFile);
    const std::string gname = baseName(graph);

    std::ofstream os(out);
    os << "algo,graph,threads,src,dst,rank,category,ref_dist,algo_dist,correct,"
          "path_len,preproc_ms,exec_ms_median,exec_ms_min,reps\n";

    auto writeRow = [&](const Pair &p, double algoDist, int correct,
                        long long pathLen, double preMs, double medMs,
                        double minMs)
    {
        os << algo << ',' << gname << ',' << usedThreads << ',' << p.src << ','
           << p.dst << ',' << p.rank << ',' << p.category << ',' << p.refDist
           << ',' << algoDist << ',' << correct << ',' << pathLen << ','
           << preMs << ',' << medMs << ',' << minMs << ',' << reps << '\n';
        os.flush();
    };

    if (isReferenceAlgo(algo))
    {
        // Reference launchers re-create their graph per execute(); we run them
        // per pair. Used only on small/medium graphs as a baseline.
        for (const auto &p : pairs)
        {
            if (!filter.empty() && p.category != filter)
                continue;
            std::vector<double> times;
            double pre = -1.0, dist = INFINITY;
            long long plen = 0;
            try
            {
                for (int r = 0; r < reps; ++r)
                {
                    if (algo == "networkit_dijkstra")
                    {
                        SP::NetworkitLauncher l(
                            graph, SP::NetworkitLauncher::AlgoId::DIJKSTRA_SEQ);
                        l.execute(p.src, p.dst);
                        const auto &res = l.getResult();
                        times.push_back(res.executionTimeMs);
                        pre = res.preProccessTimeMs;
                        dist = res.shortestDistance;
                        plen = static_cast<long long>(res.shortestPath.size());
                    }
                    else
                    {
                        SP::GapbsLauncher l(
                            graph, SP::GapbsLauncher::AlgoId::DIJKSTRA_SEQ);
                        l.execute(p.src, p.dst);
                        const auto &res = l.getResult();
                        times.push_back(res.executionTimeMs);
                        pre = res.preProccessTimeMs;
                        dist = res.shortestDistance;
                        plen = static_cast<long long>(res.shortestPath.size());
                    }
                }
            }
            catch (const std::exception &e)
            {
                writeRow(p, INFINITY, 0, 0, pre, -1.0, -1.0);
                continue;
            }
            double med = median(times);
            double mn = times.empty() ? -1.0
                                      : *std::min_element(times.begin(),
                                                          times.end());
            writeRow(p, dist, distancesMatch(dist, p.refDist) ? 1 : 0, plen, pre,
                     med, mn);
        }
        std::cerr << "query(ref) done: " << algo << " on " << gname << "\n";
        return 0;
    }

    // ---- custom algorithm: construct once, preprocess once ----
    auto obj = makeCustomAlgo(algo, graph, mapping);
    if (!obj)
    {
        std::cerr << "unknown algo: " << algo << "\n";
        return 1;
    }

    double preMs = -1.0;
    {
        auto t0 = Clock::now();
        SP::ReturnCode rc;
        try
        {
            rc = obj->preProcess();
        }
        catch (const std::exception &e)
        {
            std::cerr << "PREPROCESS_EXCEPTION " << algo << " on " << gname
                      << ": " << e.what() << "\n";
            return 2;
        }
        preMs = msSince(t0);
        if (rc != SP::ReturnCode::OK)
        {
            std::cerr << "PREPROCESS_FAILED " << algo << " on " << gname
                      << " rc=" << static_cast<int>(rc) << "\n";
            return 2;
        }
    }

    for (const auto &p : pairs)
    {
        if (!filter.empty() && p.category != filter)
            continue;
        double algoDist = INFINITY;
        long long pathLen = 0;
        std::vector<double> times;
        try
        {
            for (int w = 0; w < warmup; ++w)
            {
                obj->setSrcDest(p.src, p.dst);
                obj->compute();
            }
            for (int r = 0; r < reps; ++r)
            {
                obj->setSrcDest(p.src, p.dst);
                auto t0 = Clock::now();
                obj->compute();
                times.push_back(msSince(t0));
            }
            const auto &res = obj->getResult();
            algoDist = res.shortestDistance;
            pathLen = static_cast<long long>(res.shortestPath.size());
        }
        catch (const std::exception &e)
        {
            writeRow(p, INFINITY, 0, 0, preMs, -1.0, -1.0);
            continue;
        }
        double med = median(times);
        double mn =
            times.empty() ? -1.0
                          : *std::min_element(times.begin(), times.end());
        writeRow(p, algoDist, distancesMatch(algoDist, p.refDist) ? 1 : 0,
                 pathLen, preMs, med, mn);
    }
    std::cerr << "query done: " << algo << " on " << gname << " (" << usedThreads
              << " thr, preproc " << preMs << " ms)\n";
    return 0;
}

} // namespace

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage:\n"
                  << "  " << argv[0]
                  << " genpairs --graph F.mtx --seed S --sources K "
                     "--random R --out pairs.csv\n"
                  << "  " << argv[0]
                  << " query --graph F.mtx [--mapping M.txt] --algo NAME "
                     "--pairs pairs.csv --reps N --warmup W --threads T "
                     "[--category rank|random] --out results.csv\n";
        return 1;
    }
    std::string mode = argv[1];
    auto args = parseArgs(argc, argv, 2);
    if (mode == "weightgen")
        return runWeightGen(args);
    if (mode == "genpairs")
        return runGenPairs(args);
    if (mode == "query")
        return runQuery(args);
    std::cerr << "unknown mode: " << mode << "\n";
    return 1;
}
