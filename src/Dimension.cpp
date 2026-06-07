#include "Dimension.h"
#include "ground.h"
#include "player.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern player*      g_player;
extern std::string  assetPath(const std::string& relativ);

static const int TILE_SIZE_DIM = 32;

// ── load ──────────────────────────────────────────────────────────────────────

void DimensionManager::load(const std::string& jsonPath) {
    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        std::cerr << "[DimensionManager] Kann nicht oeffnen: " << jsonPath << std::endl;
        return;
    }
    json data;
    try { f >> data; }
    catch (const std::exception& e) {
        std::cerr << "[DimensionManager] JSON Fehler: " << e.what() << std::endl;
        return;
    }

    for (auto& [id, dData] : data.items()) {
        DimensionData dim;
        dim.id          = id;
        dim.name        = dData.value("name", id);
        dim.width          = dData.value("width",  4);
        dim.height         = dData.value("height", 10);
        dim.defaultTile    = dData.value("defaultTile", "gras");
        dim.sharedInterior = dData.value("sharedInterior", true);

        if (dData.contains("backgroundColor") && dData["backgroundColor"].is_array()
                && dData["backgroundColor"].size() >= 3) {
            dim.backgroundColor.r = (unsigned char)dData["backgroundColor"][0].get<int>();
            dim.backgroundColor.g = (unsigned char)dData["backgroundColor"][1].get<int>();
            dim.backgroundColor.b = (unsigned char)dData["backgroundColor"][2].get<int>();
            dim.backgroundColor.a = dData["backgroundColor"].size() >= 4
                                    ? (unsigned char)dData["backgroundColor"][3].get<int>() : 255;
        }

        auto tryBind = [&](std::function<void()>& target, const std::string& key) {
            auto fn = findFunc(key);
            if (fn) target = fn;
        };
        tryBind(dim.onEnter,  id + "_onEnter");
        tryBind(dim.onLeave,  id + "_onLeave");
        tryBind(dim.onUpdate, id + "_onUpdate");
        tryBind(dim.onDraw,   id + "_onDraw");

        dimensions[id] = std::move(dim);
        std::cout << "[DimensionManager] Dimension geladen: " << id << std::endl;
    }
}

// ── Tiles laden / speichern ───────────────────────────────────────────────────

void DimensionManager::loadDimensionTiles(const std::string& id, const std::string& basePath) {
    DimensionData* dim = getDimension(id);
    if (!dim) return;

    std::string path = basePath + id + ".json";
    std::ifstream f(path);
    if (!f.is_open()) return;

    json data;
    try { f >> data; } catch (...) { return; }

    dim->tiles.clear();
    if (data.contains("tiles")) {
        for (auto& [key, val] : data["tiles"].items())
            dim->tiles[key] = val;
    }
    std::cout << "[DimensionManager] Tiles geladen: " << id
              << " (" << dim->tiles.size() << " Tiles)" << std::endl;
}

void DimensionManager::saveDimensionTiles(const std::string& id, const std::string& basePath) const {
    const DimensionData* dim = getDimension(id);
    if (!dim || dim->tiles.empty()) return;

    namespace fs = std::filesystem;
    fs::create_directories(fs::path(basePath));

    json out;
    out["id"] = id;
    for (const auto& [key, val] : dim->tiles)
        out["tiles"][key] = val;

    std::ofstream f(basePath + id + ".json");
    if (f.is_open()) {
        f << out.dump(4);
        std::cout << "[DimensionManager] Tiles gespeichert: " << id << std::endl;
    }
}

void DimensionManager::cloneTemplate(const std::string& templateId, const std::string& newId) {
    if (dimensions.count(newId)) return;
    DimensionData* tmpl = getDimension(templateId);
    if (!tmpl) return;
    DimensionData clone = *tmpl;
    clone.id = newId;
    clone.tiles.clear();
    dimensions[newId] = std::move(clone);
}

void DimensionManager::saveAll(const std::string& basePath) const {
    for (const auto& [id, dim] : dimensions)
        saveDimensionTiles(id, basePath);
}

// ── Getter ────────────────────────────────────────────────────────────────────

DimensionData* DimensionManager::getDimension(const std::string& id) {
    auto it = dimensions.find(id);
    return (it != dimensions.end()) ? &it->second : nullptr;
}

const DimensionData* DimensionManager::getDimension(const std::string& id) const {
    auto it = dimensions.find(id);
    return (it != dimensions.end()) ? &it->second : nullptr;
}

DimensionData* DimensionManager::getCurrentDimension() {
    return getDimension(currentId);
}

// ── switchTo ─────────────────────────────────────────────────────────────────

void DimensionManager::switchTo(const std::string& id) {
    if (!g_player) return;

    // Aktuelle Dimension verlassen
    if (!currentId.empty()) {
        DimensionData* cur = getDimension(currentId);
        if (cur && cur->onLeave) cur->onLeave();
        saveDimensionTiles(currentId, assetPath("json/Map/dimensions/"));
    }

    if (id.empty()) {
        g_player->Set_position((int)savedWorldX, (int)savedWorldY);
        currentId.clear();
        return;
    }

    DimensionData* next = getDimension(id);
    if (!next) {
        std::cerr << "[DimensionManager] Unbekannte Dimension: " << id << std::endl;
        return;
    }

    Vector2 worldPos = g_player->Get_position();
    savedWorldX = worldPos.x;
    savedWorldY = worldPos.y;

    loadDimensionTiles(id, assetPath("json/Map/dimensions/"));

    int cx = (next->width  / 2) * TILE_SIZE_DIM;
    int cy = (next->height / 2) * TILE_SIZE_DIM;
    g_player->Set_position(cx, cy);

    currentId = id;
    if (next->onEnter) next->onEnter();
}

void switchDimension(const std::string& id) {
    g_dimensionManager.switchTo(id);
}

// ── update ────────────────────────────────────────────────────────────────────

void DimensionManager::update() {
    if (currentId.empty()) return;
    DimensionData* dim = getCurrentDimension();
    if (dim && dim->onUpdate) dim->onUpdate();
}

// ── registerFunction / findFunc ───────────────────────────────────────────────

void DimensionManager::registerFunction(const std::string& name, std::function<void()> fn) {
    functionRegistry[name] = fn;
}

std::function<void()> DimensionManager::findFunc(const std::string& name) const {
    auto it = functionRegistry.find(name);
    return (it != functionRegistry.end()) ? it->second : nullptr;
}

// ── draw_dimension ────────────────────────────────────────────────────────────

void draw_dimension(const DimensionData& dim, const GroundDatabase& ground, int tileSize) {
    for (int y = 0; y < dim.height; ++y) {
        for (int x = 0; x < dim.width; ++x) {
            std::string key      = std::to_string(x) + "," + std::to_string(y);
            const std::string* tileType = &dim.defaultTile;

            auto it = dim.tiles.find(key);
            if (it != dim.tiles.end()) tileType = &it->second;

            auto texIt = ground.texturen.find(*tileType);
            if (texIt != ground.texturen.end() && texIt->second.id != 0) {
                DrawTexture(texIt->second, x * tileSize, y * tileSize, WHITE);
            } else {
                DrawRectangle(x * tileSize, y * tileSize, tileSize, tileSize, VIOLET);
            }
        }
    }
}
