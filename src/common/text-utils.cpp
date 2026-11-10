#include "text-utils.h"
#include <boost/locale.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <locale>

namespace {
    auto create_ru_locale()
    {
        auto backends = boost::locale::localization_backend_manager::global();
        backends.select("winapi");
        boost::locale::generator generator(backends);
        return generator("ru_RU.UTF-8");
    }
}

std::wstring to_lowercase(const std::wstring& text) {
    static const std::locale locale = create_ru_locale();
    return boost::locale::to_lower(text, locale);
}

std::wstring from_utf8(const std::string& text) {
    return boost::locale::conv::utf_to_utf<wchar_t>(text, boost::locale::conv::stop);
}

std::string to_utf8(const std::wstring& text) {
    return boost::locale::conv::utf_to_utf<char>(text, boost::locale::conv::stop);
}

std::wstring read_text(const fs::path& file) {
    std::ifstream input(file, std::ios::binary);
    if(!input) {
        throw std::runtime_error("Cannot open file");
    }

    std::string text;
    char buffer[4096];
    while(input.read(buffer, sizeof(buffer))) {
        text.append(buffer, sizeof(buffer));
    }

    text.append(buffer, static_cast<std::size_t>(input.gcount()));
    if(input.bad() || !input.eof()) {
        throw std::runtime_error("Cannot read file");
    }

    return from_utf8(text);
}

std::vector<std::wstring> split_words(const std::wstring& text) {
    auto cleaned = text;
    static const std::locale locale("ru-RU");
    for (wchar_t& c : cleaned) {
        if (!std::isalpha(c, locale)) {
            c = L' ';
        }
    }
    boost::algorithm::trim(cleaned);
    if(cleaned.empty()) {
        return {};
    }
    cleaned = to_lowercase(cleaned);

    std::vector<std::wstring> tokens;
    boost::algorithm::split(tokens, cleaned, boost::algorithm::is_any_of(L" "), boost::token_compress_on);

    return tokens;
}
