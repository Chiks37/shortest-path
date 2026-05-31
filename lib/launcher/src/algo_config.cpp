/**
 * @file algo_config.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Runtime ON/OFF configuration of algorithms source file
 */

#include "algo_config.hpp"
#include "gapbs_launcher.hpp"
#include "networkit_launcher.hpp"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>

#ifndef ALGOS_CONFIG_PATH
#define ALGOS_CONFIG_PATH "algorithms.conf"
#endif

namespace SP
{

namespace
{

struct Group
{
    std::string title;
    std::vector<std::string> names;
};

std::vector<Group> canonicalGroups()
{
    Group custom{"# Custom algorithms", {}};
    for (auto id : CustomLauncher::algoIds)
        custom.names.push_back(CustomLauncher::algoNames.at(id));

    Group reference{"# Reference baselines", {}};
    for (auto id : NetworkitLauncher::algoIds)
        reference.names.push_back(NetworkitLauncher::algoNames.at(id));
    for (auto id : GapbsLauncher::algoIds)
        reference.names.push_back(GapbsLauncher::algoNames.at(id));

    return {custom, reference};
}

std::string configPath()
{
    if (const char *env = std::getenv("SP_ALGOS_CONFIG"))
    {
        if (env[0] != '\0')
            return env;
    }
    return ALGOS_CONFIG_PATH;
}

bool parseBool(const std::string &token, bool fallback)
{
    std::string t;
    for (char c : token)
        t += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (t == "on" || t == "1" || t == "true" || t == "yes")
        return true;
    if (t == "off" || t == "0" || t == "false" || t == "no")
        return false;
    return fallback;
}

size_t nameColumnWidth(const std::vector<Group> &groups)
{
    size_t width = 0;
    for (const auto &g : groups)
        for (const auto &n : g.names)
            width = std::max(width, n.size());
    return width + 4;
}

void writeEntry(std::ofstream &out, const std::string &name, bool enabled,
                size_t width)
{
    out << std::left << std::setw(static_cast<int>(width)) << name
        << (enabled ? "ON" : "OFF") << '\n';
}

void writeDefaultFile(const std::string &path, const std::vector<Group> &groups)
{
    std::ofstream out(path);
    if (!out)
        return;

    const size_t width = nameColumnWidth(groups);
    out << "# Algorithm toggles for the app/ targets (read at runtime, no "
           "rebuild needed).\n"
        << "# Set OFF to skip an algorithm. sp_tests ignores this file.\n\n";
    for (const auto &g : groups)
    {
        out << g.title << '\n';
        for (const auto &n : g.names)
            writeEntry(out, n, true, width);
        out << '\n';
    }
}

void appendMissing(const std::string &path,
                   const std::vector<std::string> &missing,
                   const std::vector<Group> &groups)
{
    std::ofstream out(path, std::ios::app);
    if (!out)
        return;

    const size_t width = nameColumnWidth(groups);
    out << "\n# Added automatically\n";
    for (const auto &n : missing)
        writeEntry(out, n, true, width);
}

std::unordered_map<std::string, bool> loadConfig()
{
    const std::string path = configPath();
    const auto groups = canonicalGroups();
    std::unordered_map<std::string, bool> state;

    std::ifstream in(path);
    if (!in)
    {
        writeDefaultFile(path, groups);
        std::cerr << "[algo-config] created " << path
                  << " with all algorithms enabled" << std::endl;
        for (const auto &g : groups)
            for (const auto &n : g.names)
                state[n] = true;
        return state;
    }

    std::string line;
    while (std::getline(in, line))
    {
        const auto hash = line.find('#');
        if (hash != std::string::npos)
            line.erase(hash);

        std::istringstream iss(line);
        std::string name, value;
        if (!(iss >> name))
            continue;
        iss >> value;
        state[name] = parseBool(value, true);
    }
    in.close();

    std::vector<std::string> missing;
    for (const auto &g : groups)
        for (const auto &n : g.names)
            if (state.find(n) == state.end())
            {
                state[n] = true;
                missing.push_back(n);
            }
    if (!missing.empty())
        appendMissing(path, missing, groups);

    std::string disabled;
    for (const auto &g : groups)
        for (const auto &n : g.names)
            if (!state.at(n))
                disabled += (disabled.empty() ? "" : ", ") + n;
    if (!disabled.empty())
        std::cerr << "[algo-config] disabled (" << path << "): " << disabled
                  << std::endl;

    return state;
}

const std::unordered_map<std::string, bool> &configState()
{
    static const std::unordered_map<std::string, bool> state = loadConfig();
    return state;
}

} // namespace

bool AlgoConfig::isEnabled(const std::string &name)
{
    const auto &state = configState();
    const auto it = state.find(name);
    return it == state.end() ? true : it->second;
}

std::vector<CustomLauncher::AlgoId> AlgoConfig::enabledCustom()
{
    std::vector<CustomLauncher::AlgoId> result;
    for (auto id : CustomLauncher::algoIds)
        if (isEnabled(CustomLauncher::algoNames.at(id)))
            result.push_back(id);
    return result;
}

} // namespace SP
