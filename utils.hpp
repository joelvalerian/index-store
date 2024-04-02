#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <unordered_set>
#include <vector>

namespace utils
{
    void load_env();
    std::vector<std::string> split_string(const std::string& string, char delimiter = ',');
    std::string replace_string(const std::string& original, char toReplace, char replacement);
    std::string get_env(const std::string& env, const std::string& default_value);
    std::unordered_set<std::string> get_allowed_files(const std::string& env, const std::string& default_value);
}

#endif