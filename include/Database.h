//
// Created by Ilija Mandic on 4/18/2024.
//

#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "Table.h"
#include <filesystem>

class Database {
public:
    void createTable(const TableSchema& schema);
    Table* getTable(const std::string& name);
    bool saveToFile(const std::filesystem::path& path) const;
    static std::unique_ptr<Database> loadFromFile(const std::filesystem::path& path);

private:
    std::unordered_map<std::string, std::unique_ptr<Table>> tables;
};
