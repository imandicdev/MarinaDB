//
// Created by Ilija Mandic on 4/18/2024.
//
#pragma once
#include <string>

enum class CommandType {
    Create,
    Load,
    CreateTable,
    Insert,
    Select,
    Exit,
    Help,
    Invalid
};

inline std::string to_string(CommandType type) {
    switch(type) {
        case CommandType::Create:       return "create";
        case CommandType::Load:         return "load";
        case CommandType::CreateTable:  return "create_table";
        case CommandType::Insert:       return "insert";
        case CommandType::Select:       return "select";
        case CommandType::Exit:         return "exit";
        case CommandType::Help:         return "help";
        default:                        return "invalid";
    }
}