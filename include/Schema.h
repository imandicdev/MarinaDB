//
// Created by Ilija Mandic on 4/18/2024.
//

#pragma once
#include <string>
#include <vector>
#include <variant>

enum class DataType { Integer, String, Float };

struct Column {
    std::string name;
    DataType type;
    // Add more here: nullable, default value, etc.
};

class TableSchema {
public:
    TableSchema(std::string tableName, std::vector<Column> columns)
        : tableName(std::move(tableName)), columns(std::move(columns)) {}

    [[nodiscard]] const std::string& name() const { return tableName; }
    [[nodiscard]] const std::vector<Column>& getColumns() const { return columns; }

private:
    std::string tableName;
    std::vector<Column> columns;
};
