#include <iostream>
#include <string>
#include <vector>
#include "document_like.hpp"

int main() {
    document_like doc;

    doc.add("age", 22);
    doc.add("is_student", true);
    doc.add("name", std::string("Ayubjon Umarov"));
    doc.add("grades", std::vector<int>({85, 90, 78}));

    doc.write_to_disk("document.dat");

    document_like new_doc;
    new_doc.read_from_disk("document.dat");

    std::cout << "Age: " << new_doc.get<int>("age") << std::endl;
    std::cout << "Is Student: " << new_doc.get<bool>("is_student") << std::endl;
    std::cout << "Name: " << new_doc.get<std::string>("name") << std::endl;
    auto grades = new_doc.get<std::vector<int>>("grades");
    std::cout << "Grades: ";
    for (auto grade : grades) {
        std::cout << grade << " ";
    }
    std::cout << std::endl;

    return 0;
}