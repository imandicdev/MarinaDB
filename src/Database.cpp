//
// Created by Ilija Mandic on 4/18/2024.
//
#include "BinaryIO.h"
#include "Database.h"
#include <stdexcept>
#include <fstream>
#include <iostream>

#include "Table.h"
void Database::createTable(const TableSchema& schema) {
    if (tables.contains(schema.name())) {
        throw std::runtime_error("Table already exists: " + schema.name());
    }
    tables[schema.name()] = std::make_unique<Table>(schema);
}

Table* Database::getTable(const std::string& name) {
    const auto it = tables.find(name);
    if (it == tables.end())
        throw std::runtime_error("Table '" + name + "' does not exist.");
    return it->second.get();
}

bool Database::saveToFile(const std::filesystem::path& path) const {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return false;

    // Write magic and version
    ofs.write("MARI", 4);
    write_uint8(ofs, 1);

    // Table count
    write_uint32(ofs, static_cast<uint32_t>(tables.size()));

    for (const auto& [name, tablePtr] : tables) {
        // Table name
        write_string(ofs, tablePtr->schema().name());
        // Columns
        const auto& columns = tablePtr->schema().getColumns();
        write_uint16(ofs, static_cast<uint16_t>(columns.size()));
        for (const auto& col : columns) {
            write_string(ofs, col.name);
            write_uint8(ofs, static_cast<uint8_t>(col.type));
        }
        // Records
        const auto& records = tablePtr->getRecords();
        write_uint32(ofs, static_cast<uint32_t>(records.size()));
        for (const auto& record : records) {
            for (const auto& col : columns) {
                const auto& value = record.at(col.name);
                if (col.type == DataType::Integer) {
                    write_uint32(ofs, static_cast<uint32_t>(std::get<int>(value)));
                } else if (col.type == DataType::Float) {
                    float f = std::get<float>(value);
                    ofs.write(reinterpret_cast<char*>(&f), sizeof(float));
                } else {
                    write_string(ofs, std::get<std::string>(value));
                }
            }
        }
    }
    return true;
}

std::unique_ptr<Database> Database::loadFromFile(const std::filesystem::path& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open file for reading: " + path.string());

    char magic[4];
    ifs.read(magic, 4);
    if (std::string(magic, 4) != "MARI") {
        std::cerr << "Not a MarinaDB database file!\n";
        return nullptr;
    }
    uint8_t version = read_uint8(ifs);
    if (version != 1) {
        std::cerr << "Unsupported MarinaDB file version!\n";
        return nullptr;
    }
    auto db = std::make_unique<Database>();
    uint32_t table_count = read_uint32(ifs);
    for (uint32_t t = 0; t < table_count; ++t) {
        std::string table_name = read_string(ifs);
        uint16_t column_count = read_uint16(ifs);
        std::vector<Column> columns;
        for (uint16_t c = 0; c < column_count; ++c) {
            std::string col_name = read_string(ifs);
            DataType col_type = static_cast<DataType>(read_uint8(ifs));
            columns.push_back({col_name, col_type});
        }
        TableSchema schema(table_name, columns);
        db->createTable(schema);
        Table* table = db->getTable(table_name);

        uint32_t record_count = read_uint32(ifs);
        for (uint32_t r = 0; r < record_count; ++r) {
            Record rec;
            for (const auto& col : columns) {
                if (col.type == DataType::Integer) {
                    rec[col.name] = static_cast<int>(read_uint32(ifs));
                } else if (col.type == DataType::Float) {
                    float f;
                    ifs.read(reinterpret_cast<char*>(&f), sizeof(float));
                    rec[col.name] = f;
                } else {
                    rec[col.name] = read_string(ifs);
                }
            }
            table->insert(rec);
        }
    }
    return db;
}