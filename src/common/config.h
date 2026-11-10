#pragma once
#include <filesystem>
#include <set>
#include <string>

struct Config {
    std::string connection_params;
    std::filesystem::path directory;
    std::set<std::string> extensions;
};

Config parse_config(const std::filesystem::path &file);
