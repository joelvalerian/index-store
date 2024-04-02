#include "index.hpp"

#include <filesystem>
#include <string>
#include <cmath>
#include <fstream>
#include <utility>
#include <sstream>
#include <random>
#include <algorithm>
#include <vector>

#include "crow/logging.h"

#include "utils.hpp"

namespace fs = std::filesystem;

IndexStore::IndexStore(
    const std::string& data_folder,
    const std::string& index_map_filename,
    const std::string& index_deque_filename,
    const std::string& charsets
) : m_data_folder { data_folder + '/' },
    m_index_map_filename { index_map_filename },
    m_index_deque_filename { index_deque_filename },
    m_index_length { static_cast<size_t>(std::pow(charsets.length(), 9)) }
{
    if (fs::exists(m_index_map_filename) && fs::exists(m_index_deque_filename))
    {
        CROW_LOG_INFO << "Loading IndexStore from existing configuration (IS_CHARSETS ignored)";
        std::string fileline;

        std::ifstream indexMapFile { m_index_map_filename };
        while (std::getline(indexMapFile, fileline, '\n'))
        {
            auto delimIndex = fileline.find('\t');
            std::pair<std::string, std::string> keyVal { std::make_pair(fileline.substr(0, delimIndex), fileline.substr(delimIndex+1)) };
            m_file_index_map[keyVal.first] = keyVal.second;
        }

        std::string filepath;
        std::ifstream indexDequeFile { m_index_deque_filename };
        while (std::getline(indexDequeFile, filepath))
        {
            m_file_index_deque.push_back(filepath);
        }
    }
    else
    {
        CROW_LOG_INFO << "Creating directories and index for the first time";
        
        for (auto &&cOne : charsets)
        {
            for (auto &&cTwo : charsets)
            {
                for (auto &&cThree : charsets)
                {
                    for (auto &&cFour : charsets)
                    {
                        for (auto &&cFive : charsets)
                        {
                            for (auto &&cSix : charsets)
                            {
                                std::stringstream dirPath;
                                dirPath << m_data_folder << cOne << cTwo << cThree << '/' << cFour << cFive << cSix;
                                fs::create_directories(dirPath.str());
                                for (auto &&cSeven : charsets)
                                {
                                    for (auto &&cEight : charsets)
                                    {
                                        for (auto &&cNine : charsets)
                                        {
                                            std::stringstream filename;
                                            filename << cOne << cTwo << cThree << '-' << cFour << cFive << cSix << '-' << cSeven << cEight << cNine;
                                            m_file_index_deque.push_back(filename.str());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        std::random_device rd;
        std::mt19937 randomizer(rd());
        std::shuffle(m_file_index_deque.begin(), m_file_index_deque.end(), randomizer);

        std::ofstream indexDequeFile { m_index_deque_filename };
        std::ofstream indexMapFile { m_index_map_filename };
        
        for (auto &&filepath : m_file_index_deque)
        {
            indexDequeFile << filepath << '\n';
        }
    }
}

IndexStore::~IndexStore()
{
    save_index();
}

void IndexStore::save_index() const
{
    std::ofstream indexDequeFile { m_index_deque_filename };
    for (auto &&filepath : m_file_index_deque)
    {
        indexDequeFile << filepath << '\n';
    }

    std::ofstream indexMapFile { m_index_map_filename };
    for (auto &&[key, value] : m_file_index_map)
    {
        indexMapFile << key << '\t' << value << '\n';
    }
}

void IndexStore::print_map() const
{
    for (auto &&[key, value] : m_file_index_map)
    {
        CROW_LOG_DEBUG << "Filename: " << key << "\t Path: " << value;
    }   
}

std::vector<std::string> IndexStore::saved_files() const
{
    std::vector<std::string> savedFiles {};
    for (auto &&[key, value] : m_file_index_map)
    {
        savedFiles.push_back(key);
    }
    
    return savedFiles;
}

void IndexStore::print_deque(size_t limit) const
{
    CROW_LOG_DEBUG << "Unused Paths (Top " << limit << "):";
    size_t i { 1 };
    for (auto &&filepath : m_file_index_deque)
    {
        CROW_LOG_DEBUG << i << ". " << filepath;

        if (i++ >= limit)
        {
            break;
        }
    }
}

bool IndexStore::add_file(const std::string& new_filename, const std::string& new_file)
{
    if (m_file_index_deque.empty() || m_file_index_map.count(new_filename) != 0)
    {
        // Index full or file with same name already exists
        return false;
    }
    std::string random_path { m_data_folder + utils::replace_string(m_file_index_deque.front(), '-', '/') };
    std::ofstream out_file { random_path };
    out_file << new_file;
    m_file_index_map[new_filename] = m_file_index_deque.front();
    m_file_index_deque.pop_front();
    return true;
}

std::string IndexStore::get_file(const std::string& old_filename) const
{
    std::string old_file {};
    if (this->has_file(old_filename))
    {
        std::ifstream out_file { m_data_folder + utils::replace_string(m_file_index_map.at(old_filename), '-', '/') };
        std::stringstream buffer {};
        buffer << out_file.rdbuf();
        old_file = buffer.str();
        return old_file;
    }
    else
    {
        return old_file;
    }
}

bool IndexStore::has_file(const std::string& old_filename) const
{
    if (m_file_index_map.count(old_filename) == 0)
    {
        return false;
    }
    return true;
}

bool IndexStore::remove_file(const std::string& old_filename)
{
    if (m_file_index_map.count(old_filename) == 0)
    {
        return false;
    }

    std::string random_path { m_data_folder + utils::replace_string(m_file_index_map.at(old_filename), '-', '/') };
    if (!fs::remove(random_path))
    {
        return false;
    }
    else
    {
        m_file_index_deque.push_back(m_file_index_map.at(old_filename));
        m_file_index_map.erase(old_filename);
        return true;
    }
}