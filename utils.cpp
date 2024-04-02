#include "utils.hpp"

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>
#include <cstdlib>

namespace utils
{
    void load_env()
    {
        if (!std::filesystem::exists(".env"))
        {
            return;
        }

        std::ifstream dotenv { ".env" };
        std::string tempLine {};

        while (std::getline(dotenv, tempLine, '\n'))
        {
            auto delimIndex = tempLine.find('=');
            std::pair<std::string, std::string> keyVal { std::make_pair(tempLine.substr(0, delimIndex), tempLine.substr(delimIndex+1)) };
            if (keyVal.first.empty() || keyVal.second.empty())
            {
                continue;
            }
            setenv(keyVal.first.c_str(), keyVal.second.c_str(), 1);
        }
    }

    std::vector<std::string> split_string(const std::string& string, char delimiter)
    {
        std::stringstream stream { string };
        std::string temp {};
        std::vector<std::string> splits {};

        while (std::getline(stream, temp, delimiter))
        {
            splits.push_back(temp);
        }

        return splits;
    }

    std::string replace_string(const std::string& original, char toReplace, char replacement)
    {
        std::string modified { original };
        size_t pos { modified.find(toReplace) };
        while (pos != std::string::npos)
        {
            modified[pos] = replacement;
            pos = modified.find(toReplace, pos + 1);
        }
        return modified;
    }

    std::string get_env(const std::string& env, const std::string& default_value)
    {
        auto value = std::getenv(env.c_str());
        if (value == nullptr)
        {
            return default_value;
        }
        else
        {
            return value;
        }
    }

    std::unordered_set<std::string> get_allowed_files(const std::string& env, const std::string& default_value)
    {
        std::vector<std::string> splitted { split_string(get_env(env, default_value)) };
        std::unordered_set<std::string> allowedFiles ( splitted.begin(), splitted.end() );

        return allowedFiles;
    }
}