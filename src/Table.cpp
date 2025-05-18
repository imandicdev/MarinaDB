//
// Created by Ilija Mandic on 4/18/2024.
//

#include "Table.h"
#include <stdexcept>
#include <utility>

Table::Table(TableSchema  schema)
    : tableSchema(std::move(schema)) {}

void Table::insert(const Record& record) {
    // Validate schema (simple check: keys and types)
    for (const auto& col : tableSchema.getColumns()) {
        auto it = record.find(col.name);
        if (it == record.end()) {
            throw std::runtime_error("Missing column: " + col.name);
        }
        // Type-checking (very basic, you can extend)
        if ((col.type == DataType::Integer && !std::holds_alternative<int>(it->second)) ||
            (col.type == DataType::Float   && !std::holds_alternative<float>(it->second)) ||
            (col.type == DataType::String  && !std::holds_alternative<std::string>(it->second))) {
            throw std::runtime_error("Type mismatch for column: " + col.name);
            }
    }
    records.push_back(record);
}

const std::vector<Record>& Table::getRecords() const {
    return records;
}