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

    std::cout << "Age: ";
    if (std::holds_alternative<int>(doc.get("age"))) {
        std::cout << std::get<int>(doc.get("age")) << "\n";
    } else {
        std::cout << std::get<no_value>(doc.get("age")) << "\n";
    }

    std::cout << "Is student: ";
    if (std::holds_alternative<bool>(doc.get("is_student"))) {
        std::cout << std::get<bool>(doc.get("is_student")) << "\n";
    } else {
        std::cout << std::get<no_value>(doc.get("is_student")) << "\n";
    }

    std::cout << "Name: ";
    if (std::holds_alternative<std::string>(doc.get("name"))) {
        std::cout << std::get<std::string>(doc.get("name")) << "\n";
    } else {
        std::cout << std::get<no_value>(doc.get("name")) << "\n";
    }

    std::cout << "Grades: ";
    if (std::holds_alternative<std::vector<int>>(doc.get("grades"))) {
        for (int grade : std::get<std::vector<int>>(doc.get("grades"))) {
            std::cout << grade << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << std::get<no_value>(doc.get("grades")) << "\n";
    }

    std::cout << "Unknown key: ";
    if (std::holds_alternative<no_value>(doc.get("unknown_key"))) {
        std::cout << std::get<no_value>(doc.get("unknown_key")) << "\n";
    } else {
        std::cout << "Unexpected type\n";
    }

    doc.remove("is_student");

    if (std::holds_alternative<no_value>(doc.get("is_student"))) {
        std::cout << std::get<no_value>(doc.get("is_student")) << "\n";
    } else {
        std::cout << "Unexpected type\n";
    }
    doc.add("is_student", false);

    std::cout << "Is student: ";
    if (std::holds_alternative<bool>(doc.get("is_student"))) {
        std::cout << std::get<bool>(doc.get("is_student")) << "\n";
    } else {
        std::cout << std::get<no_value>(doc.get("is_student")) << "\n";
    }

    // для примера
    doc.remove("is_student");
    doc.remove("is_student");
    doc.remove("is_student");
    return 0;
}

