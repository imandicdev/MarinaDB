#include <iostream>
#include "Database.h"

int main() {
    // Define schema
    TableSchema personSchema(
        "person",
        {
            {"id", DataType::Integer},
            {"name", DataType::String},
            {"salary", DataType::Float}
        }
    );

    Database db;
    db.createTable(personSchema);

    Table* person = db.getTable("person");
    if (!person) return 1;

    // Insert a record
    Record record = {
        {"id", 1},
        {"name", std::string("Alice")},
        {"salary", 1000.0f}
    };
    person->insert(record);

    // Print all records
    for (const auto& rec : person->getRecords()) {
        std::cout << "id: " << std::get<int>(rec.at("id"))
                  << ", name: " << std::get<std::string>(rec.at("name"))
                  << ", salary: " << std::get<float>(rec.at("salary")) << "\n";
    }

    return 0;
}