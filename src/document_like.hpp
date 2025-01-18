#ifndef DOCUMENT_LIKE_HPP
#define DOCUMENT_LIKE_HPP

#include <iostream>
#include <unordered_map>
#include <variant>
#include <string_view>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>

class document_like {
public:
    using value_type = std::variant<int, bool, std::string, std::vector<int>, struct no_value>;

    explicit document_like(const std::string& filename);

    void add(std::string_view key, const value_type& value);

    const value_type get(std::string_view key) const;

    void remove(std::string_view key) noexcept;

private:
    static value_type create_empty_value();
    static value_type read_value_at_offset(std::fstream& file, size_t offset, int type_id);
    int get_type_id_from_offset(size_t offset) const;

    std::string filename_;
    mutable std::unordered_map<std::string, size_t> data_; // offset
    mutable std::fstream file_;

    template<typename T>
    static int get_type_id();

    void write_value(const value_type& value, size_t offset) const;
};

struct no_value {
    friend std::ostream& operator<<(std::ostream& os, const no_value&) {
        os << "No value";
        return os;
    }
};

#endif // DOCUMENT_LIKE_HPP