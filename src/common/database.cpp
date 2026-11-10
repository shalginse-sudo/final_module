#include "database.h"

Database::Database(const Config& config)
    : m_connection(config.connection_params)
{}

void Database::create_tables() {
    pqxx::work tx(m_connection);
    tx.exec(R"(
        CREATE TABLE IF NOT EXISTS documents (
            id BIGSERIAL PRIMARY KEY,
            path TEXT UNIQUE NOT NULL
        )
    )");
    tx.exec(R"(
        CREATE TABLE IF NOT EXISTS words (
            id BIGSERIAL PRIMARY KEY,
            word TEXT UNIQUE NOT NULL
        )
    )");
    tx.exec(R"(
        CREATE TABLE IF NOT EXISTS document_words (
            document_id BIGINT REFERENCES documents(id) ON DELETE CASCADE,
            word_id BIGINT REFERENCES words(id) ON DELETE CASCADE,
            frequency BIGINT NOT NULL CHECK (frequency > 0),
            PRIMARY KEY(document_id, word_id)
        )
    )");
    tx.exec(R"(
        ALTER TABLE documents
        DROP COLUMN IF EXISTS root
    )");
    tx.commit();
}

void Database::clear() {
    pqxx::work tx(m_connection);
    tx.exec(R"(
        TRUNCATE TABLE document_words, words, documents
        RESTART IDENTITY
    )");
    tx.commit();
}

void Database::save(const std::string& path, const std::map<std::string, long long>& frequencies) {
    pqxx::work tx(m_connection);
    auto row = tx.exec(R"(
        INSERT INTO documents(path)
        VALUES ($1)
        RETURNING id
    )", pqxx::params{path});
    auto id = row[0][0].as<long long>();
    for (const auto& [word, count] : frequencies) {
        tx.exec(R"(
            INSERT INTO words(word)
            VALUES ($1)
            ON CONFLICT(word) DO NOTHING
        )", pqxx::params{word});
        tx.exec(R"(
            INSERT INTO document_words(document_id, word_id, frequency)
            VALUES (
                $1,
                (SELECT id FROM words WHERE word = $2),
                $3
            )
        )", pqxx::params{id, word, count});
    }
    tx.commit();
}

pqxx::result Database::search(const std::set<std::string>& query) {
    pqxx::read_transaction tx(m_connection);
    std::string list;
    for (const auto& word : query) {
        if (!list.empty()) list += ',';
        list += tx.quote(word);
    }
    return tx.exec("SELECT d.path, SUM(dw.frequency) AS score "
        "FROM documents d "
        "JOIN document_words dw ON dw.document_id = d.id "
        "JOIN words w ON w.id = dw.word_id "
        "WHERE w.word IN (" + list + ") "
        "GROUP BY d.id, d.path "
        "HAVING COUNT(*) = " + std::to_string(query.size()) + " "
        "ORDER BY score DESC, d.path "
        "LIMIT 10");
}


