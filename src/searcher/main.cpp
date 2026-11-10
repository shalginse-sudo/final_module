#include <boost/algorithm/string/trim.hpp>
#include "text-utils.h"
#include "config.h"
#include "database.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        Config config = parse_config(argc > 1 ? fs::u8path(argv[1]) : fs::path("config.ini"));

        Database database(config);
        std::string line;
        while (std::cout << "Search (empty line to exit): " && std::getline(std::cin, line)) {
            if(boost::algorithm::trim_copy(line).empty()) {
                break;
            }

            try {
                auto tokens = split_words(from_utf8(line));
                if (tokens.empty() || tokens.size() > 4) {
                    std::cout << "Enter between 1 and 4 words.\n";
                    continue;
                }

                std::set<std::string> query;
                for(const auto &token : tokens) {
                    query.insert(to_utf8(token));
                }
                auto rows = database.search(query);
                if(rows.empty()) {
                    std::cout << "No results found.\n";
                }
                for(const auto &row : rows) {
                    std::cout << row[0].as<std::string>() << " (score: " << row[1].as<std::string>() << ")\n";
                }
            } catch (const std::exception& e) {
                std::cerr << "Search error: " << e.what() << '\n';
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Startup error: " << e.what() << '\n';
        return 1;
    }
}


