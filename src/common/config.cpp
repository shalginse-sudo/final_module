#include <boost/algorithm/string/trim.hpp>
#include "config.h"
#include "text-utils.h"
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <sstream>
#include <stdexcept>

namespace {
    std::string escape_postgresql_special_characters(const std::string &str)
    {
        std::string ans;
        for(char c : str) {
            if(c == '\'' || c == '\\') {
                ans += '\\';
            } 
            ans += c;
        }

        return ans;
    }
}

Config parse_config(const std::filesystem::path &file)
{
    Config config;

    std::ifstream input(file);

    if(!input) {
        throw std::runtime_error("Cannot open configuration file");
    }

    boost::property_tree::ptree values;
    boost::property_tree::ini_parser::read_ini(input, values);
    if(input.bad()) {
        throw std::runtime_error("Cannot read configuration file");
    }

    for(const auto *key : { "host", "port", "dbname", "user", "password" }) {
        auto value = escape_postgresql_special_characters(values.get<std::string>(std::string("database.") + key));
        config.connection_params += std::string(key) + "='" + value + "' ";
    }
    config.connection_params += "client_encoding='UTF8'";

    auto directory = values.get<std::string>("indexer.directory");
    if(directory.empty()) {
        throw std::runtime_error("Missing indexing directory");
    }

    config.directory = fs::u8path(directory);
    if(config.directory.is_relative()) {
        config.directory = fs::absolute(file).parent_path() / config.directory;
    } 

    std::string line;
    std::istringstream list(values.get<std::string>("indexer.extensions"));
    while(std::getline(list, line, ',')) {
        line = boost::algorithm::trim_copy(line);
        if(line.empty()) {
            continue;
        }
        if(line.front() != '.') {
            line.insert(line.begin(), '.');
        }
        line = to_utf8(to_lowercase(from_utf8(line)));
        config.extensions.insert(line);
    }

    if(config.extensions.empty()) {
        throw std::runtime_error("Missing file extensions");
    }
    return config;
}
