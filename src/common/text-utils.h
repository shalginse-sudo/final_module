#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::wstring to_lowercase(const std::wstring& text);
std::wstring from_utf8(const std::string& text);
std::string to_utf8(const std::wstring& text);
std::wstring read_text(const fs::path& file);
std::vector<std::wstring> split_words(const std::wstring& text);
