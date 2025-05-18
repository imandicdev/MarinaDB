#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <string>
#include "CommandType.h"
#include "CommandMap.h"
#include "CommandHandlers.h"
#include "Database.h"

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
        for (const auto& col : columns)
            std::cout << col.name << "\t";
        std::cout << "\n";
        // Print each record
        for (const auto& rec : records) {
            for (const auto& col : columns) {
                const auto& val = rec.at(col.name);
                if (col.type == DataType::Integer)
                    std::cout << std::get<int>(val) << "\t";
                else if (col.type == DataType::Float)
                    std::cout << std::get<float>(val) << "\t";
                else
                    std::cout << std::get<std::string>(val) << "\t";
            }
            std::cout << "\n";
        }
    });

    dispatcher.registerHandler(CommandType::Help, [&](const std::vector<std::string>&) {
        std::cout <<
            "Supported commands:\n"
            "  create <file>                                 Create a new (empty) database\n"
            "  load <file>                                   Load existing database\n"
            "  create_table <table> <col>:<type> ...         Create table/schema (types: int, float, string)\n"
            "  insert <table> <col>=<val> ...                Insert record into table\n"
            "  select <table>                                Display all records from table\n"
            "  help                                          Show this message\n"
            "  exit                                          Quit MarinaDB CLI\n";
    });

    // --- Main Loop ---
    std::cout << "MarinaDB CLI v0.2. Type 'help' for commands.\n";
    std::string line;
    while (std::cout << "marina> ", std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmdWord;
        iss >> cmdWord;
        std::vector<std::string> args;
        for (std::string arg; iss >> arg;) args.push_back(arg);
        std::ranges::transform(cmdWord, cmdWord.begin(), ::tolower);
        CommandType cmd = parseCommand(cmdWord);
        if (cmd == CommandType::Exit) break;
        dispatcher.dispatch(cmd, args);
    }
    std::cout << "Goodbye!\n";
}