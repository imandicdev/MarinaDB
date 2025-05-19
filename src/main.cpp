#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ranges>
#include <unordered_map>
#include <memory>
#include <string>
#include "CommandType.h"
#include "CommandMap.h"
#include "CommandHandlers.h"
#include "Database.h"
// <iomanip> is required ONLY for help message alignment using std::setw below.

// ----------- Utility functions -----------

inline std::unordered_map<std::string, std::string> parseKeyValuePairs(const std::vector<std::string>& args) {
    std::unordered_map<std::string, std::string> kv;
    for (const auto& arg : args) {
        auto eqPos = arg.find('=');
        if (eqPos != std::string::npos) {
            auto key = arg.substr(0, eqPos);
            auto val = arg.substr(eqPos + 1);
            kv[key] = val;
        }
    }
    return kv;
}

inline std::vector<Column> parseColumnDefinitions(const std::vector<std::string>& args) {
    std::vector<Column> columns;
    for (const auto& arg : args) {
        auto pos = arg.find(':');
        if (pos == std::string::npos) continue;
        std::string name = arg.substr(0, pos);
        std::string type = arg.substr(pos + 1);
        DataType dt;
        if      (type == "int")    dt = DataType::Integer;
        else if (type == "float")  dt = DataType::Float;
        else if (type == "string") dt = DataType::String;
        else throw std::runtime_error("Invalid type: " + type);
        columns.push_back({name, dt});
    }
    return columns;
}

// ----------- Main CLI -----------

int main() {
    std::unique_ptr<Database> db;
    CommandDispatcher dispatcher;

    dispatcher.registerHandler(CommandType::Create, [&](const std::vector<std::string>& args) {
        if (args.empty()) { std::cout << "Usage: create <filename>\n"; return; }
        db = std::make_unique<Database>();
        db->saveToFile(args[0]);
        std::cout << "Empty database created and saved to " << args[0] << "\n";
    });

    dispatcher.registerHandler(CommandType::Load, [&](const std::vector<std::string>& args) {
        if (args.empty()) { std::cout << "Usage: load <filename>\n"; return; }
        db = Database::loadFromFile(args[0]);
        std::cout << (db ? "Loaded DB from " : "Failed to load ") << args[0] << "\n";
    });

    dispatcher.registerHandler(CommandType::CreateTable, [&](const std::vector<std::string>& args) {
        if (!db) { std::cout << "No database loaded.\n"; return; }
        if (args.size() < 2) {
            std::cout << "Usage: create_table <table> <col1>:<type> <col2>:<type> ...\n";
            std::cout << "Types: int, float, string\n";
            return;
        }
        try {
            const std::string& tableName = args[0];
            auto columns = parseColumnDefinitions({args.begin() + 1, args.end()});
            TableSchema schema(tableName, columns);
            db->createTable(schema);
            std::cout << "Table '" << tableName << "' created.\n";
        } catch (const std::exception& ex) {
            std::cout << "Error: " << ex.what() << "\n";
        }
    });

    dispatcher.registerHandler(CommandType::Insert, [&](const std::vector<std::string>& args) {
        if (!db) { std::cout << "No database loaded.\n"; return; }
        if (args.size() < 2) {
            std::cout << "Usage: insert <table> <col1>=<val1> <col2>=<val2> ...\n";
            return;
        }
        const std::string& tableName = args[0];
        Table* table = db->getTable(tableName);
        if (!table) {
            std::cout << "Table '" << tableName << "' does not exist.\n";
            return;
        }
        auto kv = parseKeyValuePairs({args.begin() + 1, args.end()});
        Record record;
        const auto& columns = table->schema().getColumns();
        for (const auto& col : columns) {
            auto it = kv.find(col.name);
            if (it == kv.end()) {
                std::cout << "Missing value for column: " << col.name << "\n";
                return;
            }
            if (col.type == DataType::Integer)      record[col.name] = std::stoi(it->second);
            else if (col.type == DataType::Float)   record[col.name] = std::stof(it->second);
            else                                    record[col.name] = it->second;
        }
        table->insert(record);
        std::cout << "Inserted record into '" << tableName << "'.\n";
    });

    dispatcher.registerHandler(CommandType::Select, [&](const std::vector<std::string>& args) {
        if (!db) { std::cout << "No database loaded.\n"; return; }
        if (args.empty()) { std::cout << "Usage: select <table>\n"; return; }
        const std::string& tableName = args[0];
        Table* table = db->getTable(tableName);
        if (!table) {
            std::cout << "Table '" << tableName << "' does not exist.\n";
            return;
        }
        const auto& records = table->getRecords();
        const auto& columns = table->schema().getColumns();
        // Print header
        std::ranges::for_each(columns, [](const Column& col) { std::cout << col.name << "\t"; });
        std::cout << "\n";
        std::ranges::for_each(records, [&](const Record& rec) {
            std::ranges::for_each(columns, [&](const Column& col) {
                const auto& val = rec.at(col.name);
                if (col.type == DataType::Integer)
                    std::cout << std::get<int>(val) << "\t";
                else if (col.type == DataType::Float)
                    std::cout << std::get<float>(val) << "\t";
                else
                    std::cout << std::get<std::string>(val) << "\t";
            });
            std::cout << "\n";
        });
    });

    // --- select_where: select <table> where <col>=<val> ---
    dispatcher.registerHandler(CommandType::SelectWhere, [&](const std::vector<std::string>& args) {
        if (!db) { std::cout << "No database loaded.\n"; return; }
        if (args.size() < 4 || args[1] != "where") {
            std::cout << "Usage: select <table> where <col>=<val>\n";
            return;
        }
        const std::string& tableName = args[0];
        Table* table = db->getTable(tableName);
        if (!table) {
            std::cout << "Table '" << tableName << "' does not exist.\n";
            return;
        }
        const auto& columns = table->schema().getColumns();
        std::string column, valueString;
        auto eqPos = args[2].find('=');
        if(eqPos != std::string::npos) {
            column = args[2].substr(0, eqPos);
            valueString = args[2].substr(eqPos+1);
        } else if (args.size() > 3 && args[3].find('=') != std::string::npos) {
            column = args[2];
            valueString = args[3].substr(args[3].find('=') + 1);
        } else {
            column = args[2];
            valueString = args.size() > 3 ? args[3] : "";
        }
        auto colIt = std::ranges::find_if(columns, [&](const Column& c){ return c.name == column; });
        if (colIt == columns.end()) {
            std::cout << "Column '" << column << "' not found in schema.\n";
            return;
        }
        Value key;
        try {
            if (colIt->type == DataType::Integer)
                key = std::stoi(valueString);
            else if (colIt->type == DataType::Float)
                key = std::stof(valueString);
            else
                key = valueString;
        } catch (const std::exception& ex) {
            std::cout << "Value parse error: " << ex.what() << "\n";
            return;
        }
        bool usedIndex = false;
        if (colIt == columns.begin() && table->isIndexed()) {
            const Record* rec = table->findByKey(key);
            if (rec) {
                for (const auto& col : columns)
                    std::cout << col.name << "\t";
                std::cout << "\n";
                for (const auto& col : columns) {
                    const auto& val = rec->at(col.name);
                    if (col.type == DataType::Integer)
                        std::cout << std::get<int>(val) << "\t";
                    else if (col.type == DataType::Float)
                        std::cout << std::get<float>(val) << "\t";
                    else
                        std::cout << std::get<std::string>(val) << "\t";
                }
                std::cout << "\n";
                usedIndex = true;
            }
        }
        if (!usedIndex) {
            bool found = false;
            auto recIt = std::ranges::find_if(
                table->getRecords(),
                [&](const Record& rec) {
                    auto it = rec.find(column);
                    return it != rec.end() && it->second == key;
                });
            if (recIt != table->getRecords().end()) {
                std::ranges::for_each(columns, [](const Column& col) { std::cout << col.name << "\t"; });
                std::cout << "\n";
                std::ranges::for_each(columns, [&](const Column& col) {
                    const auto& val = recIt->at(col.name);
                    if (col.type == DataType::Integer)
                        std::cout << std::get<int>(val) << "\t";
                    else if (col.type == DataType::Float)
                        std::cout << std::get<float>(val) << "\t";
                    else
                        std::cout << std::get<std::string>(val) << "\t";
                });
                std::cout << "\n";
                found = true;
            }
            if (!found) std::cout << "No record found with " << column << "=" << valueString << "\n";
        }
    });
        // Startup message only; suppress command list for minimal prompt.
        // std::cout << "Supported commands: ..." << std::endl; // now shown only via 'help'

    dispatcher.registerHandler(CommandType::Help, [&](const std::vector<std::string>&) {
#include <iomanip>
        std::vector<std::pair<std::string, std::string>> help_entries = {
            {"create <file>", "Create a new (empty) database"},
            {"load <file>", "Load existing database"},
            {"create_table <table> <col>:<type> ...", "Create table/schema (types: int, float, string)"},
            {"insert <table> <col>=<val> ...", "Insert record into table"},
            {"select <table>", "Display all records from table"},
            {"select <table> where <column>=<value>", "Find and print a record by key (fast if indexed, else linear)"},
            {"help", "Show this message"},
            {"exit", "Quit MarinaDB CLI"}
        };
        std::cout << "Supported commands:\n";
        for (const auto& entry : help_entries) {
            std::cout << "  " << std::left << std::setw(40) << entry.first << entry.second << "\n";
        }
    });


    // --- Main Loop ---
    std::cout << "MarinaDB CLI v0.2. Type 'help' for commands.\n";
    std::cout << "(C) 2024-2025 Ilija Mandic. All rights reserved.\n";
    std::string line;
    try {
        while (std::cout << "marina> ", std::getline(std::cin, line)) {
            try {
                std::istringstream iss(line);
                std::string cmdWord;
                iss >> cmdWord;
                std::vector<std::string> args;
                for (std::string arg; iss >> arg;) args.push_back(arg);
                std::ranges::transform(cmdWord, cmdWord.begin(), ::tolower);
                CommandType cmd = parseCommand(cmdWord, args);
                if (cmd == CommandType::Exit) break;
                dispatcher.dispatch(cmd, args);
            } catch (const std::exception& ex) {
                std::cout << "[Command Error] " << ex.what() << std::endl;
            }
        }
    } catch (const std::exception& ex) {
        std::cout << "[Fatal Error] " << ex.what() << std::endl;
    }
    std::cout << "Goodbye!\n";
}
