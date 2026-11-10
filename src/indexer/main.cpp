#include "text-utils.h"
#include "config.h"
#include "database.h"
#include "thread-pool.h"
#include <algorithm>
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        Config config = parse_config(argc > 1 ? fs::u8path(argv[1]) : fs::path("config.ini"));

        const auto root = fs::canonical(config.directory);
        if(!fs::is_directory(root)) {
            throw std::runtime_error("Indexing path is not a directory");
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::recursive_directory_iterator(root)) {
            if(entry.is_symlink() || !entry.is_regular_file()) {
                continue;
            }

            auto extension = to_utf8(to_lowercase(entry.path().extension().wstring()));
            if (config.extensions.contains(extension)) {
                files.push_back(entry.path());
            }
        }

        Database database(config);
        database.create_tables();
        database.clear();

        ThreadPool pool(std::max(2u, std::thread::hardware_concurrency()));
        std::vector<std::future<std::map<std::string, long long>>> results;
        for(const auto &file : files) {
            auto future = pool.post<std::map<std::string, long long>>([file] {
                std::wstring text = read_text(file);
                std::map<std::string, long long> frequencies;
                for(const auto &word : split_words(text)) {
                    ++frequencies[to_utf8(word)];
                }
                return frequencies;
            });

            results.push_back(std::move(future));
        }

        std::size_t saved = 0;
        for (std::size_t i = 0; i < files.size(); ++i) {
            const auto path = files[i].string();
            std::map<std::string, long long> frequencies;
            try {
                frequencies = results[i].get();
            } catch (const std::exception& e) {
                std::cerr << path << ": " << e.what() << '\n';
                continue;
            }
            database.save(path, frequencies);
            ++saved;
        }

        std::cout << "Indexed: " << saved << ", failed: " << files.size() - saved << '\n';
        return saved == files.size() ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Indexing error: " << e.what() << '\n';
        return 1;
    }
}



