#include "src/document_like.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::string filename = argv[1];
    document_like doc(filename);

    doc.add("age", 22);
    doc.add("is_student", true);
    doc.add("name", std::string("Ayubjon Umarov"));
    doc.add("grades", std::vector<int>({85, 90, 78}));

    // Получаем значение для ключа "age"
    const auto& age_value = doc.get("age");
    std::cout << "Age: ";
    if (std::holds_alternative<int>(age_value)) {
        std::cout << std::get<int>(age_value) << "\n";
    } else {
        std::cout << std::get<no_value>(age_value) << "\n";
    }

    // Получаем значение для ключа "is_student"
    const auto& is_student_value = doc.get("is_student");
    std::cout << "Is student: ";
    if (std::holds_alternative<bool>(is_student_value)) {
        std::cout << std::get<bool>(is_student_value) << "\n";
    } else {
        std::cout << std::get<no_value>(is_student_value) << "\n";
    }

    // Получаем значение для ключа "name"
    const auto& name_value = doc.get("name");
    std::cout << "Name: ";
    if (std::holds_alternative<std::string>(name_value)) {
        std::cout << std::get<std::string>(name_value) << "\n";
    } else {
        std::cout << std::get<no_value>(name_value) << "\n";
    }

    // Получаем значение для ключа "grades"
    const auto& grades_value = doc.get("grades");
    std::cout << "Grades: ";
    if (std::holds_alternative<std::vector<int>>(grades_value)) {
        const auto& grades = std::get<std::vector<int>>(grades_value);
        for (int grade : grades) {
            std::cout << grade << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << std::get<no_value>(grades_value) << "\n";
    }

    // Проверка на несуществующий ключ
    const auto& unknown_key_value = doc.get("unknown_key");
    std::cout << "Unknown key: ";
    if (std::holds_alternative<no_value>(unknown_key_value)) {
        std::cout << std::get<no_value>(unknown_key_value) << "\n";
    } else {
        std::cout << "Unexpected type\n";
    }

    doc.remove("is_student");

    return 0;
}