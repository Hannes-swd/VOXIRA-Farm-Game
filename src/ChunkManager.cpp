#include "ChunkManager.h"
#include "WorldGen.h"
#include "Object.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

// Korrekte Floor-Division für negative Zahlen (b ist immer CHUNK_SIZE > 0)
static int floorDiv(int a, int b) {
    int q = a / b;
    if (a % b != 0 && a < 0) q--;
    return q;
}

std::string ChunkManager::chunkKey(int cx, int cy) {
    return std::to_string(cx) + "," + std::to_string(cy);
}

void ChunkManager::toChunkCoords(int wx, int wy, int& cx, int& cy, int& lx, int& ly) {
    cx = floorDiv(wx, CHUNK_SIZE);
    cy = floorDiv(wy, CHUNK_SIZE);
    lx = wx - cx * CHUNK_SIZE;
    ly = wy - cy * CHUNK_SIZE;
}

void ChunkManager::init(const std::string& dir, const std::string& defTile) {
    saveDir     = dir;
    defaultTile = defTile;
    if (!dir.empty()) fs::create_directories(dir);
}

std::string ChunkManager::getTile(int wx, int wy) const {
    int cx, cy, lx, ly;
    toChunkCoords(wx, wy, cx, cy, lx, ly);
    auto it = loaded.find(chunkKey(cx, cy));
    if (it == loaded.end()) return defaultTile;
    return it->second.tiles[ly][lx];
}

void ChunkManager::setTile(int wx, int wy, const std::string& type) {
    int cx, cy, lx, ly;
    toChunkCoords(wx, wy, cx, cy, lx, ly);
    std::string key = chunkKey(cx, cy);
    if (!loaded.count(key)) loadChunk(cx, cy);
    Chunk& chunk          = loaded[key];
    chunk.tiles[ly][lx]   = type;
    chunk.dirty           = true;
}

void ChunkManager::loadChunk(int cx, int cy) {
    std::string key = chunkKey(cx, cy);
    if (loaded.count(key)) return;

    Chunk chunk;
    chunk.chunkX = cx;
    chunk.chunkY = cy;
    for (int y = 0; y < CHUNK_SIZE; y++)
        for (int x = 0; x < CHUNK_SIZE; x++)
            chunk.tiles[y][x] = defaultTile;

    std::string path = saveDir + "/chunk_" + std::to_string(cx) + "_" + std::to_string(cy) + ".json";
    std::ifstream f(path);
    if (f.is_open()) {
        try {
            json data;
            f >> data;
            if (data.contains("tiles")) {
                for (auto& [tkey, tval] : data["tiles"].items()) {
                    size_t comma = tkey.find(',');
                    if (comma == std::string::npos) continue;
                    int lx = std::stoi(tkey.substr(0, comma));
                    int ly = std::stoi(tkey.substr(comma + 1));
                    if (lx >= 0 && lx < CHUNK_SIZE && ly >= 0 && ly < CHUNK_SIZE)
                        chunk.tiles[ly][lx] = tval.get<std::string>();
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[Chunk] JSON Fehler in " << path << ": " << e.what() << std::endl;
        }
    }

    // Neuer Chunk (keine Datei vorhanden) → ggf. Terrain generieren
    if (!f.is_open() && g_worldGen.isEnabled()) {
        for (int ly = 0; ly < CHUNK_SIZE; ly++) {
            for (int lx = 0; lx < CHUNK_SIZE; lx++) {
                int wx = cx * CHUNK_SIZE + lx;
                int wy = cy * CHUNK_SIZE + ly;
                chunk.tiles[ly][lx] = g_worldGen.generateTile(wx, wy);
                std::string objId = g_worldGen.generateObjectAt(wx, wy);
                if (!objId.empty()) g_objectManager.place(objId, wx, wy);
            }
        }
        chunk.dirty = true; // generierten Chunk direkt speichern
    }

    loaded[key] = std::move(chunk);
}

void ChunkManager::unloadChunk(int cx, int cy) {
    std::string key = chunkKey(cx, cy);
    auto it = loaded.find(key);
    if (it == loaded.end()) return;
    if (it->second.dirty) saveChunk(it->second);
    loaded.erase(it);
}

void ChunkManager::saveChunk(const Chunk& chunk) const {
    if (saveDir.empty()) return;
    std::string path = saveDir + "/chunk_" + std::to_string(chunk.chunkX) + "_"
                                           + std::to_string(chunk.chunkY) + ".json";
    json data;
    // Nur Tiles die vom Default abweichen speichern
    for (int ly = 0; ly < CHUNK_SIZE; ly++) {
        for (int lx = 0; lx < CHUNK_SIZE; lx++) {
            if (chunk.tiles[ly][lx] != defaultTile) {
                data["tiles"][std::to_string(lx) + "," + std::to_string(ly)] = chunk.tiles[ly][lx];
            }
        }
    }
    std::ofstream f(path);
    if (f.is_open()) {
        f << data.dump(2);
    } else {
        std::cerr << "[Chunk] Kann nicht speichern: " << path << std::endl;
    }
}

void ChunkManager::saveAll() {
    for (auto& [key, chunk] : loaded) {
        if (chunk.dirty) {
            saveChunk(chunk);
            chunk.dirty = false;
        }
    }
}

void ChunkManager::updateAround(int tileX, int tileY, int radius) {
    int centerCX = floorDiv(tileX, CHUNK_SIZE);
    int centerCY = floorDiv(tileY, CHUNK_SIZE);

    // Neue Chunks im Radius laden
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int cx = centerCX + dx;
            int cy = centerCY + dy;
            if (!loaded.count(chunkKey(cx, cy)))
                loadChunk(cx, cy);
        }
    }

    // Chunks außerhalb Radius+1 entladen (Puffer verhindert Thrashing)
    std::vector<std::pair<int, int>> toUnload;
    for (auto& [key, chunk] : loaded) {
        if (std::abs(chunk.chunkX - centerCX) > radius + 1 ||
            std::abs(chunk.chunkY - centerCY) > radius + 1)
            toUnload.push_back({ chunk.chunkX, chunk.chunkY });
    }
    for (auto [cx, cy] : toUnload)
        unloadChunk(cx, cy);
}

void ChunkManager::importFromFlat(const std::unordered_map<std::string, std::string>& flatTiles) {
    std::unordered_map<std::string, Chunk> temp;

    for (auto& [tileKey, tileType] : flatTiles) {
        size_t comma = tileKey.find(',');
        if (comma == std::string::npos) continue;
        int wx = std::stoi(tileKey.substr(0, comma));
        int wy = std::stoi(tileKey.substr(comma + 1));

        int cx, cy, lx, ly;
        toChunkCoords(wx, wy, cx, cy, lx, ly);
        std::string key = chunkKey(cx, cy);

        if (!temp.count(key)) {
            Chunk c;
            c.chunkX = cx;
            c.chunkY = cy;
            for (int y = 0; y < CHUNK_SIZE; y++)
                for (int x = 0; x < CHUNK_SIZE; x++)
                    c.tiles[y][x] = defaultTile;
            temp[key] = c;
        }
        temp[key].tiles[ly][lx] = tileType;
    }

    for (auto& [key, chunk] : temp)
        saveChunk(chunk);

    std::cout << "[ChunkManager] Migration: " << temp.size()
              << " Chunks aus " << flatTiles.size() << " Tiles erstellt." << std::endl;
}
