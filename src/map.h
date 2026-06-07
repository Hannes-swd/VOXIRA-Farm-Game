#pragma once
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include "ChunkManager.h"

struct GroundDatabase;

struct PlacedBuilding {
    std::string buildingId;
    int x = 0, y = 0;
    int width = 1, height = 1;
    std::string instanceId;
    std::string state;
};

struct Map {
    ChunkManager chunkMgr;
    std::unordered_map<std::string, PlacedBuilding> buildings;
    const GroundDatabase* groundDatabase = nullptr;
    std::string defaultType = "gras";

    void init(const GroundDatabase& ground);
    void setTile(int x, int y, const std::string& typ);
    std::string getTile(int x, int y) const;
    void placeBuilding(const PlacedBuilding& pb);
    PlacedBuilding* getBuildingAt(int x, int y);
    void save(const std::string& datei);
    void load(const std::string& datei);
    void clear();
    int  getSize() const;

    // Jeden Frame aufrufen – lädt/entlädt Chunks rund um den Spieler
    void updateChunks(int tileX, int tileY, int radius = 2);
};

void draw_ground(const Map& world, const GroundDatabase& ground, int tileSize);
