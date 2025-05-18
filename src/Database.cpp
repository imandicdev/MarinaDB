//
// Created by Ilija Mandic on 4/18/2024.
//
#include "Database.h"
#include <stdexcept>

void Database::createTable(const TableSchema& schema) {
    if (tables.contains(schema.name())) {
        throw std::runtime_error("Table already exists: " + schema.name());
    }
    tables[schema.name()] = std::make_unique<Table>(schema);
}

Table* Database::getTable(const std::string& name) {
    const auto it = tables.find(name);
    if (it == tables.end()) return nullptr;
    return it->second.get();
}