#include "Buildings.h"
#include "Mouse tile.h"
#include "player.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>

extern player* g_player;

PlacedBuilding* g_activePlacedBuilding = nullptr;
bool g_buildingJustPlaced = false;

using json = nlohmann::json;

extern std::string assetPath(const std::string& relativ);

static std::string normalizePath(const std::string& rawPath) {
    for (const std::string& sep : { "assets/", "assets\\" }) {
        size_t pos = rawPath.rfind(sep);
        if (pos != std::string::npos) {
            std::string rest = rawPath.substr(pos + sep.size());
            for (char& c : rest) if (c == '\\') c = '/';
            return rest;
        }
    }
    std::string result = rawPath;
    for (char& c : result) if (c == '\\') c = '/';
    return result;
}

static void loadBuildingsFromJson(const std::string& path, BuildingManager& mgr) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[BuildingManager] Kann nicht oeffnen: " << path << std::endl;
        return;
    }

    json data;
    try { f >> data; }
    catch (const std::exception& e) {
        std::cerr << "[BuildingManager] JSON Fehler in " << path << ": " << e.what() << std::endl;
        return;
    }

    for (auto& [id, bData] : data.items()) {
        auto b       = std::make_unique<Building>();
        b->id        = id;
        b->name      = bData.value("name", id);
        b->width     = bData.value("width",  1);
        b->height    = bData.value("height", 1);
        b->solid     = bData.value("solid",  true);

        std::string rawTex = bData.value("textur", "");
        if (!rawTex.empty()) {
            b->texturPath = assetPath(normalizePath(rawTex));
            b->loadTexture();
        }

        // Auto-Bind falls BUILDING_BEGIN/END bereits registriert hat
        auto tryBind = [&](std::function<void()>& target, const std::string& key) {
            auto fn = mgr.findFunc(key);
            if (fn) target = fn;
        };
        tryBind(b->onHover,  id + "_onHover");
        tryBind(b->onClick,  id + "_onClick");
        tryBind(b->onPlace,  id + "_onPlace");
        tryBind(b->onEnter,  id + "_onEnter");
        tryBind(b->onLeave,  id + "_onLeave");
        tryBind(b->onUpdate, id + "_onUpdate");

        std::cout << "[BuildingManager] Building geladen: " << id
                  << " (" << b->width << "x" << b->height << ")" << std::endl;
        mgr.registerBuilding(std::move(b));
    }
}

void BuildingManager::scanAndLoadBuildings() {
    namespace fs = std::filesystem;

    const std::string ordner = assetPath("json/Buildings/");
    if (!fs::exists(ordner)) {
        std::cerr << "[BuildingManager] Ordner nicht gefunden: " << ordner << std::endl;
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(ordner)) {
        if (entry.path().extension() == ".json") {
            std::cout << "[BuildingManager] Lade: " << entry.path().string() << std::endl;
            loadBuildingsFromJson(entry.path().string(), *this);
        }
    }
}

void updateBuildings(Map& world, BuildingManager& mgr, int tileSize) {
    if (g_player && g_player->IsMouseOnUi()) return;

    // Klick der ein Gebäude platziert hat darf nicht gleichzeitig onClick auslösen
    if (g_buildingJustPlaced) {
        g_buildingJustPlaced = false;
        return;
    }

    Vector2 t  = getTileMousePos(tileSize);
    int tileX  = (int)t.x;
    int tileY  = (int)t.y;

    PlacedBuilding* pb = world.getBuildingAt(tileX, tileY);
    if (!pb) return;

    Building* b = mgr.getBuilding(pb->buildingId);
    if (!b) return;

    g_activePlacedBuilding = pb;

    if (b->onHover) b->onHover();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (b->onClick) b->onClick();
    }

    g_activePlacedBuilding = nullptr;
}

void draw_buildings(const Map& world, BuildingManager& mgr, int tileSize) {
    for (const auto& [key, pb] : world.buildings) {
        Building* b = mgr.getBuilding(pb.buildingId);
        if (!b) continue;

        int px = pb.x * tileSize;
        int py = pb.y * tileSize;
        int pw = pb.width  * tileSize;
        int ph = pb.height * tileSize;

        if (b->textur.id != 0) {
            Rectangle src = { 0, 0, (float)b->textur.width, (float)b->textur.height };
            Rectangle dst = { (float)px, (float)py, (float)pw, (float)ph };
            DrawTexturePro(b->textur, src, dst, {0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(px, py, pw, ph, DARKBLUE);
            DrawRectangleLines(px, py, pw, ph, BLUE);
        }
    }
}
