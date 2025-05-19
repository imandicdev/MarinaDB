//
// Created by Ilija Mandic on 4/18/2024.
//

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include "Schema.h"
#include "BPlusTree.h"

using Value = std::variant<int, float, std::string>;
using Record = std::unordered_map<std::string, Value>;

// Table now supports indexing (production-grade):
// - Supports optional BPlusTree index on first column if int or string.
// - Transparent, diagnostics-friendly, type safe.
// - For additional key types/compound keys, extend key logic and index member.

class Table {
public:
    explicit Table(TableSchema  schema);

    void insert(const Record& record);
    [[nodiscard]] const std::vector<Record>& getRecords() const;
    [[nodiscard]] const TableSchema& schema() const { return tableSchema; }

    // Find using key for indexed column (first column, currently only int/string supported).
    // Returns pointer to record if found, nullptr otherwise.
    const Record* findByKey(const Value& key) const;

    // Returns true if table is indexed.
    bool isIndexed() const { return indexActive; }
    // Index diagnostics
    std::string indexColumn() const { return indexedColumnName; }

private:
    TableSchema tableSchema;
    std::vector<Record> records;

    // Optional index member(s). Only one active (for now).
    std::unique_ptr<BPlusTree<int, Record>> intIndex;
    std::unique_ptr<BPlusTree<std::string, Record>> stringIndex;
    bool indexActive = false;
    std::string indexedColumnName;

    // Checks the schema at construction and sets up suitable index if first col is int or string.
    void setupIndex();
};
