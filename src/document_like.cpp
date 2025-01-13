#include "document_like.hpp"
#include <fstream>

void document_like::add(std::string_view key, const int& value) {
    data[std::string(key)] = value;
}

void document_like::add(std::string_view key, const bool& value) {
    data[std::string(key)] = value;
}

void document_like::add(std::string_view key, const std::string& value) {
    data[std::string(key)] = value;
}

void document_like::add(std::string_view key, const std::vector<int>& value) {
    data[std::string(key)] = value;
}

void document_like::remove(std::string_view key) noexcept {
    data.erase(std::string(key));
}

void document_like::write_to_disk(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    size_t size = data.size();
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto& [key, value] : data) {
        size_t key_size = key.size();
        file.write(reinterpret_cast<const char*>(&key_size), sizeof(key_size));
        file.write(key.data(), key_size);

        size_t variant_index = value.index();
        file.write(reinterpret_cast<const char*>(&variant_index), sizeof(variant_index));

        switch (variant_index) {
            case 0: { // int
                int val = std::get<int>(value);
                file.write(reinterpret_cast<const char*>(&val), sizeof(val));
                break;
            }
            case 1: { // bool
                bool val = std::get<bool>(value);
                file.write(reinterpret_cast<const char*>(&val), sizeof(val));
                break;
            }
            case 2: { // string
                const std::string& str = std::get<std::string>(value);
                size_t str_size = str.size();
                file.write(reinterpret_cast<const char*>(&str_size), sizeof(str_size));
                file.write(str.data(), str_size);
                break;
            }
            case 3: { // vector<int>
                const std::vector<int>& vec = std::get<std::vector<int>>(value);
                size_t vec_size = vec.size();
                file.write(reinterpret_cast<const char*>(&vec_size), sizeof(vec_size));
                file.write(reinterpret_cast<const char*>(vec.data()), vec_size * sizeof(int));
                break;
            }
        }
    }
}

void document_like::read_from_disk(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));

    for (size_t i = 0; i < size; ++i) {
        size_t key_size;
        file.read(reinterpret_cast<char*>(&key_size), sizeof(key_size));
        std::string key(key_size, '\0');
        file.read(&key[0], key_size);

        size_t variant_index;
        file.read(reinterpret_cast<char*>(&variant_index), sizeof(variant_index));

        switch (variant_index) {
            case 0: { // int
                int value;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                add(key, value);
                break;
            }
            case 1: { // bool
                bool value;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                add(key, value);
                break;
            }
            case 2: { // string
                size_t str_size;
                file.read(reinterpret_cast<char*>(&str_size), sizeof(str_size));
                std::string value(str_size, '\0');
                file.read(&value[0], str_size);
                add(key, value);
                break;
            }
            case 3: { // vector<int>
                size_t vec_size;
                file.read(reinterpret_cast<char*>(&vec_size), sizeof(vec_size));
                std::vector<int> value(vec_size);
                file.read(reinterpret_cast<char*>(value.data()), vec_size * sizeof(int));
                add(key, value);
                break;
            }
        }
    }
}