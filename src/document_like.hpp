#pragma once

#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include <string>

class document_like {
public:
    void add(std::string_view key, const int& value);
    void add(std::string_view key, const bool& value);
    void add(std::string_view key, const std::string& value);
    void add(std::string_view key, const std::vector<int>& value);

    template<typename T>
    const T& get(std::string_view key) const noexcept {
        static T default_value{};
        auto it = data.find(std::string(key));
        if (it != data.end()) {
            return std::get<T>(it->second);
        }
        return default_value;
    }

    void remove(std::string_view key) noexcept;

    void write_to_disk(const std::string& filename) const;
    void read_from_disk(const std::string& filename);

private:
    using ValueVariant = std::variant<int, bool, std::string, std::vector<int>>;
    std::unordered_map<std::string, ValueVariant> data;
};