#include <iostream>
#include "Database.h"

int main() {
    // 1. Define schema
    TableSchema personSchema(
        "person",
        {
            {"id", DataType::Integer},
            {"name", DataType::String},
            {"salary", DataType::Float}
        }
    );

    // 2. Create DB and insert records
    Database db;
    db.createTable(personSchema);

    Table* person = db.getTable("person");
    if (!person) return 1;

    // Insert records
    person->insert({{"id", 1}, {"name", std::string("Alice")}, {"salary", 1000.0f}});
    person->insert({{"id", 2}, {"name", std::string("Marina")}, {"salary", 2000.0f}});

    // 3. Save to file (binary)
    if (db.saveToFile("testdb.marina"))
        std::cout << "Database saved to testdb.marina\n";
    else
        std::cout << "Failed to save database\n";

    // 4. Load into a new DB from file
    auto db2 = Database::loadFromFile("testdb.marina");
    if (!db2) {
        std::cout << "Failed to load database\n";
        return 1;
    }

    Table* person2 = db2->getTable("person");
    if (!person2) {
        std::cout << "Table not found after load\n";
        return 1;
    }

    // 5. Print loaded records
    std::cout << "Records loaded from file:\n";
    for (const auto& rec : person2->getRecords()) {
        std::cout << "id: " << std::get<int>(rec.at("id"))
                  << ", name: " << std::get<std::string>(rec.at("name"))
                  << ", salary: " << std::get<float>(rec.at("salary")) << "\n";
    }

    return 0;
}