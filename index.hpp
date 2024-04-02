#ifndef INDEX_HPP
#define INDEX_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <deque>

class IndexStore
{
private:
    const std::string m_data_folder;
    const std::string m_index_map_filename;
    const std::string m_index_deque_filename;
    const size_t m_index_length;

    std::unordered_map<std::string, std::string> m_file_index_map {};
    std::deque<std::string> m_file_index_deque {};
public:
    IndexStore(
        const std::string& data_folder,
        const std::string& index_map_filename,
        const std::string& index_deque_filename,
        const std::string& charsets
    );
    ~IndexStore();
    void save_index() const;
    void print_map() const;
    std::vector<std::string> saved_files() const;
    void print_deque(size_t limit = 10) const;
    bool add_file(const std::string& new_filename, const std::string& new_file);
    std::string get_file(const std::string& old_filename) const;
    bool has_file(const std::string& old_filename) const;
    bool remove_file(const std::string& old_filename);
};

#endif