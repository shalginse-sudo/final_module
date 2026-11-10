#pragma once
#include "text-utils.h"
#include "config.h"
#include <pqxx/pqxx>
#include <map>
#include <set>
#include <string>

class Database {
public:
    explicit Database(const Config& config);

    void create_tables();
    void clear();

    void save(const std::string& path,
              const std::map<std::string, long long>& frequencies);

    pqxx::result search(const std::set<std::string>& query);

private:
    pqxx::connection m_connection;
};
