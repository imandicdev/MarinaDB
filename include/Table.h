//
// Created by Ilija Mandic on 4/18/2024.
//

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include "Schema.h"

using Value = std::variant<int, float, std::string>;
using Record = std::unordered_map<std::string, Value>;

class Table {
public:
    explicit Table(TableSchema  schema);

    void insert(const Record& record);
    [[nodiscard]] const std::vector<Record>& getRecords() const;
    [[nodiscard]] const TableSchema& schema() const { return tableSchema; }

private:
    TableSchema tableSchema;
    std::vector<Record> records;
};
