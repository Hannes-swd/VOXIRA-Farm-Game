// ground.h
#pragma once
#include "raylib.h"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>

using json = nlohmann::json;

struct GroundDatabase {
    json allGroundTypes;
    std::unordered_map<std::string, Texture2D> texturen;
    
    void load(const std::string& datei);
    bool exists(const std::string& typ) const;
    const json& get(const std::string& typ) const;
    bool isWalkable(const std::string& typ) const;
    void loadTextures();
    void unloadTextures();
};