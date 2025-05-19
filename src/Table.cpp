//
// Created by Ilija Mandic on 4/18/2024.
//

#include "Table.h"
#include <stdexcept>
#include <utility>

Table::Table(TableSchema schema)
    : tableSchema(std::move(schema))
{
    setupIndex();
}

void Table::setupIndex() {
    // Index only on first column if int or string
    const auto& cols = tableSchema.getColumns();
    if (!cols.empty()) {
        indexedColumnName = cols[0].name;
        switch (cols[0].type) {
        case DataType::Integer:
            intIndex = std::make_unique<BPlusTree<int, Record>>();
            indexActive = true;
            break;
        case DataType::String:
            stringIndex = std::make_unique<BPlusTree<std::string, Record>>();
            indexActive = true;
            break;
        default:
            indexActive = false;
        }
    }
}

void Table::insert(const Record& record) {
    // Validate schema (simple check: keys and types)
    for (const auto& col : tableSchema.getColumns()) {
        auto it = record.find(col.name);
        if (it == record.end()) {
            throw std::runtime_error("Missing column: " + col.name);
        }
        if ((col.type == DataType::Integer && !std::holds_alternative<int>(it->second)) ||
            (col.type == DataType::Float   && !std::holds_alternative<float>(it->second)) ||
            (col.type == DataType::String  && !std::holds_alternative<std::string>(it->second))) {
            throw std::runtime_error("Type mismatch for column: " + col.name);
        }
    }
    records.push_back(record);
    // If indexed, update index
    if (indexActive) {
        auto it = record.find(indexedColumnName);
        if (it != record.end()) {
            if (intIndex && std::holds_alternative<int>(it->second)) {
                intIndex->insert(std::get<int>(it->second), record);
            } else if (stringIndex && std::holds_alternative<std::string>(it->second)) {
                stringIndex->insert(std::get<std::string>(it->second), record);
            }
        }
    }
}

const std::vector<Record>& Table::getRecords() const {
    return records;
}

const Record* Table::findByKey(const Value& key) const {
    if (indexActive) {
        // Find using index, then find pointer in records for safety
        if (intIndex && std::holds_alternative<int>(key)) {
            auto result = intIndex->find(std::get<int>(key));
            if (result) {
                // Search records for address matching value
                for (const auto& record : records) {
                    auto it = record.find(indexedColumnName);
                    if (it != record.end() && it->second == key)
                        return &record;
                }
            }
        } else if (stringIndex && std::holds_alternative<std::string>(key)) {
            auto result = stringIndex->find(std::get<std::string>(key));
            if (result) {
                for (const auto& record : records) {
                    auto it = record.find(indexedColumnName);
                    if (it != record.end() && it->second == key)
                        return &record;
                }
            }
        }
    }
    // Fall back to full scan if not indexed or type mismatch
    for (const auto& record : records) {
        auto it = record.find(indexedColumnName);
        if (it != record.end() && key == it->second)
            return &record;
    }
    return nullptr;
}
