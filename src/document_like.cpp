#include "document_like.hpp"

document_like::document_like(const std::string& name) : filename_(name) {
    file_.open(filename_, std::ios::binary | std::ios::in | std::ios::out);
    if (!file_) {
        // Если файл не существует, создаем новый
        file_.clear();
        file_.open(filename_, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    }
}

void document_like::add(std::string_view key, const value_type& value) {
    auto it = data_.find(std::string(key));
    size_t offset = file_.tellp();
    if (it != data_.end()) {
        // Удаляем старое значение из файла
        file_.seekp(it->second, std::ios::beg);
        file_.write(reinterpret_cast<const char*>(&offset), sizeof(offset)); // Записываем новое смещение на место старого
        it->second = offset; // Обновляем смещение в карте
    } else {
        data_[std::string(key)] = offset; // Сохраняем новое смещение
    }

    write_value(value, offset); // Записываем новое значение
}

const document_like::value_type document_like::get(std::string_view key) const {
    auto it = data_.find(std::string(key));
    if (it != data_.end()) {
        return read_value_at_offset(file_, it->second, get_type_id_from_offset(it->second));
    }
    return create_empty_value();
}

void document_like::remove(std::string_view key) noexcept {
    auto it = data_.find(std::string(key));
    if (it != data_.end()) {
        // Помечаем как удаленный, записав специальное значение
        file_.seekp(it->second, std::ios::beg);
        int deleted_type_id = -1; // Специальный тип для удаленных значений
        file_.write(reinterpret_cast<const char*>(&deleted_type_id), sizeof(deleted_type_id));
        data_.erase(it);
    }
}

document_like::value_type document_like::create_empty_value() {
    return no_value{};
}

int document_like::get_type_id_from_offset(size_t offset) const {
    file_.seekg(offset, std::ios::beg);
    int type_id;
    file_.read(reinterpret_cast<char*>(&type_id), sizeof(type_id));
    return type_id;
}

document_like::value_type document_like::read_value_at_offset(std::fstream& file, size_t offset, int type_id) {
    file.seekg(offset + sizeof(int), std::ios::beg); // Пропускаем тип

    switch (type_id) {
        case 0: { // int
            int value;
            file.read(reinterpret_cast<char*>(&value), sizeof(value));
            return value;
        }
        case 1: { // bool
            bool value;
            file.read(reinterpret_cast<char*>(&value), sizeof(value));
            return value;
        }
        case 2: { // std::string
            size_t str_size;
            file.read(reinterpret_cast<char*>(&str_size), sizeof(str_size));
            std::string value(str_size, '\0');
            file.read(&value[0], str_size);
            return value;
        }
        case 3: { // std::vector<int>
            size_t vec_size;
            file.read(reinterpret_cast<char*>(&vec_size), sizeof(vec_size));
            std::vector<int> value(vec_size);
            file.read(reinterpret_cast<char*>(value.data()), vec_size * sizeof(int));
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

void document_like::write_value(const value_type& value, size_t offset) const {
    file_.seekp(offset, std::ios::beg);
    std::visit([this, offset](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        // Записываем тип значения
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
            // Ничего не записываем для no_value
        }
    }, value);
}