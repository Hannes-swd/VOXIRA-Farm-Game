#pragma once
#include "map.h"
#include "raylib.h"
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>

struct Building {
    std::string id;
    std::string name;
    std::string texturPath;
    Texture2D textur = {0};
    int width  = 1;
    int height = 1;
    bool solid = true;

    std::function<void()> onHover;
    std::function<void()> onClick;
    std::function<void()> onPlace;
    std::function<void()> onEnter;
    std::function<void()> onLeave;
    std::function<void()> onUpdate;

    void loadTexture()   { if (!texturPath.empty()) textur = LoadTexture(texturPath.c_str()); }
    void unloadTexture() { if (textur.id != 0) { UnloadTexture(textur); textur = {0}; } }
};

class BuildingManager {
    std::unordered_map<std::string, std::unique_ptr<Building>> buildings;
    std::unordered_map<std::string, std::function<void()>>     functionRegistry;

public:
    ~BuildingManager() { unloadAll(); }

    void registerFunction(const std::string& name, std::function<void()> fn) {
        functionRegistry[name] = fn;
    }

    std::function<void()> findFunc(const std::string& name) const {
        auto it = functionRegistry.find(name);
        if (it != functionRegistry.end()) return it->second;
        return nullptr;
    }

    void registerBuilding(std::unique_ptr<Building> b) {
        if (b) buildings[b->id] = std::move(b);
    }

    Building* getBuilding(const std::string& id) {
        auto it = buildings.find(id);
        if (it != buildings.end()) return it->second.get();
        return nullptr;
    }

    void unloadAll() {
        for (auto& [id, b] : buildings) b->unloadTexture();
    }

    void scanAndLoadBuildings();
};

inline BuildingManager& getBuildingManager() {
    static BuildingManager instance;
    return instance;
}

#define g_buildingManager getBuildingManager()

void draw_buildings(const Map& world, BuildingManager& mgr, int tileSize);
void updateBuildings(Map& world, BuildingManager& mgr, int tileSize);

// Zeigt auf das PlacedBuilding das gerade hover/click-Callbacks auslöst.
// Wird in updateBuildings gesetzt – in Callbacks via getInstanceId() lesbar.
extern PlacedBuilding* g_activePlacedBuilding;

// Verhindert dass ein Klick der ein Gebäude platziert gleichzeitig dessen onClick auslöst.
extern bool g_buildingJustPlaced;
