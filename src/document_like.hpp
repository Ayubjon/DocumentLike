#ifndef DOCUMENT_LIKE_HPP
#define DOCUMENT_LIKE_HPP

#include <iostream>
#include <unordered_map>
#include <variant>
#include <string_view>
#include <string>
#include <vector>
#include <type_traits>
#include <cstring>
#include <fstream>

class document_like {
public:
    using value_type = std::variant<int, bool, std::string, std::vector<int>, struct no_value>;

    document_like(const std::string& filename);

    ~document_like();

    void add(std::string_view key, const value_type& value);

    const value_type& get(std::string_view key) const noexcept;

    void remove(std::string_view key) noexcept;

private:
    static value_type create_empty_value();

    std::string filename_;
    std::unordered_map<std::string, std::pair<value_type, size_t>> data_;
    std::fstream file_;

    struct FreeBlock {
        size_t offset;
        size_t size;
    };

    std::vector<FreeBlock> free_blocks_;

    void load_index();
    void save_index();

    size_t get_free_offset();
    void remove_offset(size_t offset);

    void write_value(const value_type& value, size_t offset);
    value_type read_value_at_offset(size_t offset);

    template<typename T>
    static int get_type_id();
};

struct no_value {
    friend std::ostream& operator<<(std::ostream& os, const no_value&) {
        os << "No value";
        return os;
    }
};

#endif

