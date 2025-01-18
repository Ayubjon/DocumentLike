#include "document_like.hpp"

document_like::document_like(const std::string& filename) : filename_(filename) {
    file_.open(filename_, std::ios::binary | std::ios::in | std::ios::out);
    if (!file_) {
        file_.clear();
        file_.open(filename_, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    } else {
        load_index();
    }
}

document_like::~document_like() {
    save_index();
    file_.close();
}

void document_like::add(std::string_view key, const value_type& value) {
    auto it = data_.find(std::string(key));
    if (it != data_.end()) {
        remove_offset(it->second.second);
        it->second.first = value;
        write_value(value, it->second.second);
    } else {
        size_t offset = get_free_offset();
        data_[std::string(key)] = {value, offset};
        write_value(value, offset);
    }
}

const document_like::value_type& document_like::get(std::string_view key) const noexcept {
    static const value_type default_value = create_empty_value();
    auto it = data_.find(std::string(key));
    if (it != data_.end()) {
        return it->second.first;
    }
    return default_value;
}

void document_like::remove(std::string_view key) noexcept {
    auto it = data_.find(std::string(key));
    if (it != data_.end()) {
        remove_offset(it->second.second);
        data_.erase(it);
    }
}

document_like::value_type document_like::create_empty_value() {
    return no_value{};
}

void document_like::load_index() {
    size_t index_size;
    file_.read(reinterpret_cast<char*>(&index_size), sizeof(index_size));

    for (size_t i = 0; i < index_size; ++i) {
        std::string key;
        size_t key_size;
        file_.read(reinterpret_cast<char*>(&key_size), sizeof(key_size));
        key.resize(key_size);
        file_.read(&key[0], key_size);

        size_t offset;
        file_.read(reinterpret_cast<char*>(&offset), sizeof(offset));

        value_type value = read_value_at_offset(offset);
        data_[key] = {value, offset};
    }

    size_t free_blocks_count;
    file_.read(reinterpret_cast<char*>(&free_blocks_count), sizeof(free_blocks_count));

    for (size_t i = 0; i < free_blocks_count; ++i) {
        FreeBlock block;
        file_.read(reinterpret_cast<char*>(&block.offset), sizeof(block.offset));
        file_.read(reinterpret_cast<char*>(&block.size), sizeof(block.size));
        free_blocks_.push_back(block);
    }
}

void document_like::save_index() {
    // Запись индекса в начало файла
    file_.seekp(0, std::ios::beg);
    size_t index_size = data_.size();
    file_.write(reinterpret_cast<const char*>(&index_size), sizeof(index_size));

    for (const auto& [key, pair] : data_) {
        size_t key_size = key.size();
        file_.write(reinterpret_cast<const char*>(&key_size), sizeof(key_size));
        file_.write(key.data(), key_size);

        size_t offset = pair.second;
        file_.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }

    // Запись списка свободных блоков
    size_t free_blocks_count = free_blocks_.size();
    file_.write(reinterpret_cast<const char*>(&free_blocks_count), sizeof(free_blocks_count));

    for (const auto& block : free_blocks_) {
        file_.write(reinterpret_cast<const char*>(&block.offset), sizeof(block.offset));
        file_.write(reinterpret_cast<const char*>(&block.size), sizeof(block.size));
    }
}

size_t document_like::get_free_offset() {
    if (!free_blocks_.empty()) {
        auto block = free_blocks_.back();
        free_blocks_.pop_back();
        return block.offset;
    }
    // Возвращаем текущий конец файла как новое смещение
    file_.seekp(0, std::ios::end);
    return file_.tellp();
}

void document_like::remove_offset(size_t offset) {
    // Получаем размер удаляемого значения
    int type_id;
    file_.seekg(offset, std::ios::beg);
    file_.read(reinterpret_cast<char*>(&type_id), sizeof(type_id));

    size_t value_size = 0;
    switch (type_id) {
        case 0: // int
            value_size = sizeof(int);
            break;
        case 1: // bool
            value_size = sizeof(bool);
            break;
        case 2: // std::string
            {
                size_t str_size;
                file_.read(reinterpret_cast<char*>(&str_size), sizeof(str_size));
                value_size = sizeof(type_id) + sizeof(str_size) + str_size;
            }
            break;
        case 3: // std::vector<int>
            {
                size_t vec_size;
                file_.read(reinterpret_cast<char*>(&vec_size), sizeof(vec_size));
                value_size = sizeof(type_id) + sizeof(vec_size) + vec_size * sizeof(int);
            }
            break;
        case 4: // no_value
            value_size = sizeof(type_id);
            break;
        default:
            throw std::runtime_error("Unknown type id");
    }

    // Добавляем этот блок в список свободных блоков
    free_blocks_.push_back({offset, value_size});
}

void document_like::write_value(const value_type& value, size_t offset) {
    file_.seekp(offset, std::ios::beg);
    std::visit([this](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        int type_id = get_type_id<T>();
        file_.write(reinterpret_cast<const char*>(&type_id), sizeof(type_id));

        if constexpr (std::is_same_v<T, int>) {
            file_.write(reinterpret_cast<const char*>(&arg), sizeof(arg));
        } else if constexpr (std::is_same_v<T, bool>) {
            bool temp = arg;
            file_.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
        } else if constexpr (std::is_same_v<T, std::string>) {
            size_t str_size = arg.size();
            file_.write(reinterpret_cast<const char*>(&str_size), sizeof(str_size));
            file_.write(arg.data(), str_size);
        } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            size_t vec_size = arg.size();
            file_.write(reinterpret_cast<const char*>(&vec_size), sizeof(vec_size));
            file_.write(reinterpret_cast<const char*>(arg.data()), vec_size * sizeof(int));
        } else if constexpr (std::is_same_v<T, no_value>) {
            // Ничего не записываем
        }
    }, value);
}

document_like::value_type document_like::read_value_at_offset(size_t offset) {
    file_.seekg(offset, std::ios::beg);
    int type_id;
    file_.read(reinterpret_cast<char*>(&type_id), sizeof(type_id));

    switch (type_id) {
        case 0: { // int
            int value;
            file_.read(reinterpret_cast<char*>(&value), sizeof(value));
            return value;
        }
        case 1: { // bool
            bool value;
            file_.read(reinterpret_cast<char*>(&value), sizeof(value));
            return value;
        }
        case 2: { // std::string
            size_t str_size;
            file_.read(reinterpret_cast<char*>(&str_size), sizeof(str_size));
            std::string value(str_size, '\0');
            file_.read(&value[0], str_size);
            return value;
        }
        case 3: { // std::vector<int>
            size_t vec_size;
            file_.read(reinterpret_cast<char*>(&vec_size), sizeof(vec_size));
            std::vector<int> value(vec_size);
            file_.read(reinterpret_cast<char*>(value.data()), vec_size * sizeof(int));
            return value;
        }
        case 4: { // no_value
            return no_value{};
        }
        default:
            throw std::runtime_error("Unknown type id");
    }
}

template<typename T>
int document_like::get_type_id() {
    if constexpr (std::is_same_v<T, int>) {
        return 0;
    } else if constexpr (std::is_same_v<T, bool>) {
        return 1;
    } else if constexpr (std::is_same_v<T, std::string>) {
        return 2;
    } else if constexpr (std::is_same_v<T, std::vector<int>>) {
        return 3;
    } else if constexpr (std::is_same_v<T, no_value>) {
        return 4;
    } else {
        static_assert(sizeof(T) == 0, "Unsupported type");
    }
}