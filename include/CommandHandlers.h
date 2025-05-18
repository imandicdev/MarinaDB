//
// Created by Ilija Mandic on 4/19/2024.
//

#pragma once
#include "CommandType.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

using CommandHandler = std::function<void(const std::vector<std::string>& args)>;

class CommandDispatcher {
    std::unordered_map<CommandType, CommandHandler> handlers;
public:
    void registerHandler(CommandType type, CommandHandler handler) {
        handlers[type] = std::move(handler);
    }
    void dispatch(CommandType type, const std::vector<std::string>& args) const {
        auto it = handlers.find(type);
        if (it != handlers.end())
            it->second(args);
        else
            std::cout << "Unknown command or not implemented yet.\n";
    }
};
