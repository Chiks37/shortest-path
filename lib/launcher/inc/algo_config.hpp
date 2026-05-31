/**
 * @file algo_config.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Runtime ON/OFF configuration of algorithms
 */
#pragma once

#include "custom_launcher.hpp"

#include <string>
#include <vector>

namespace SP
{

// Reads a plain-text config file (one "<name> ON|OFF" per line) that toggles
// which algorithms the app targets run. The file is loaded lazily once per
// process and cached. It is auto-created with every algorithm enabled when
// missing, and any newly added algorithm absent from the file is appended as
// ON, so the file always lists every method. sp_tests does not use this and
// always exercises all algorithms.
class AlgoConfig
{
  public:
    static bool isEnabled(const std::string &name);
    static std::vector<CustomLauncher::AlgoId> enabledCustom();
};

} // namespace SP
