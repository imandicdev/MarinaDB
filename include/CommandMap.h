//
// Created by Ilija Mandic on 4/19/2024.
//
#pragma once
#include "CommandType.h"
#include <unordered_map>

inline const std::unordered_map<std::string, CommandType> CommandMap = {
    {"create",       CommandType::Create},
    {"load",         CommandType::Load},
    {"create_table", CommandType::CreateTable},
    {"insert",       CommandType::Insert},
    {"select",       CommandType::Select},
    {"exit",         CommandType::Exit},
    {"help",         CommandType::Help}
};

inline CommandType parseCommand(const std::string& cmd) {
    auto it = CommandMap.find(cmd);
    return (it != CommandMap.end()) ? it->second : CommandType::Invalid;
}
