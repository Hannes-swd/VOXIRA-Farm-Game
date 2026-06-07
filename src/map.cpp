#include <fstream>
#include <iostream>
#include <filesystem>
#include "map.h"
#include "ground.h"
#include "Cam.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

void Map::setTile(int x, int y, const std::string& typ) {
    chunkMgr.setTile(x, y, typ);
}

std::string Map::getTile(int x, int y) const {
    return chunkMgr.getTile(x, y);
}

void Map::init(const GroundDatabase& ground) {
    groundDatabase = &ground;

    if (!ground.allGroundTypes.empty()) {
        defaultType          = ground.allGroundTypes.begin().key();
        chunkMgr.defaultTile = defaultType;
        std::cout << "Default Bodentyp: " << defaultType << std::endl;
    }
}

void Map::save(const std::string& datei) {
    // Chunks auf Disk schreiben
    chunkMgr.saveAll();

    // Welt-Metadaten + Gebäude speichern (keine tiles-Sektion mehr)
    fs::path p(datei);
    if (p.has_parent_path()) fs::create_directories(p.parent_path());

    json ausgabe;
    ausgabe["name"]        = "Meine Welt";
    ausgabe["default_typ"] = defaultType;

    for (const auto& [key, pb] : buildings) {
        ausgabe["buildings"][key]["buildingId"]  = pb.buildingId;
        ausgabe["buildings"][key]["x"]           = pb.x;
        ausgabe["buildings"][key]["y"]           = pb.y;
        ausgabe["buildings"][key]["width"]       = pb.width;
        ausgabe["buildings"][key]["height"]      = pb.height;
        ausgabe["buildings"][key]["instanceId"]  = pb.instanceId;
        ausgabe["buildings"][key]["state"]       = pb.state;
    }

    std::ofstream f(datei);
    if (f.is_open()) {
        f << ausgabe.dump(4);
        f.flush();
        std::cout << "[Map] Gespeichert: " << datei << std::endl;
    } else {
        std::cerr << "[Map] Fehler: Kann nicht speichern: " << datei << std::endl;
    }
}

void Map::load(const std::string& datei) {
    std::ifstream f(datei);
    if (!f.is_open()) {
        std::cerr << "[Map] Fehler: Kann nicht laden: " << datei << std::endl;
        return;
    }

    json input;
    try { f >> input; }
    catch (const std::exception& e) {
        std::cerr << "[Map] JSON Fehler: " << e.what() << std::endl;
        return;
    }

    buildings.clear();

    if (input.contains("default_typ"))
        defaultType = input["default_typ"];

    // Chunk-Ordner neben world.json anlegen
    fs::path worldPath(datei);
    std::string chunkDir = (worldPath.parent_path() / "chunks").string();
    chunkMgr.init(chunkDir, defaultType);

    // Migration: altes Format mit "tiles"-Sektion → Chunks
    if (input.contains("tiles") && !input["tiles"].empty()) {
        std::unordered_map<std::string, std::string> flatTiles;
        for (auto& [key, val] : input["tiles"].items())
            flatTiles[key] = val.get<std::string>();
        chunkMgr.importFromFlat(flatTiles);
        std::cout << "[Map] Migration abgeschlossen – Chunks gespeichert." << std::endl;
    }

    if (input.contains("buildings")) {
        for (auto& [key, bData] : input["buildings"].items()) {
            PlacedBuilding pb;
            pb.buildingId = bData.value("buildingId", "");
            pb.x          = bData.value("x",          0);
            pb.y          = bData.value("y",          0);
            pb.width      = bData.value("width",      1);
            pb.height     = bData.value("height",     1);
            pb.instanceId = bData.value("instanceId", "");
            pb.state      = bData.value("state",      "");
            buildings[key] = pb;
        }
    }

    std::cout << "[Map] Geladen: " << datei << std::endl;
}

void Map::placeBuilding(const PlacedBuilding& pb) {
    std::string key = std::to_string(pb.x) + "," + std::to_string(pb.y);
    buildings[key] = pb;
}

PlacedBuilding* Map::getBuildingAt(int x, int y) {
    for (auto& [key, pb] : buildings) {
        if (x >= pb.x && x < pb.x + pb.width &&
            y >= pb.y && y < pb.y + pb.height)
            return &pb;
    }
    return nullptr;
}

void Map::clear() {
    chunkMgr = ChunkManager();
    buildings.clear();
}

int Map::getSize() const {
    return (int)chunkMgr.getLoadedChunks().size() * CHUNK_SIZE * CHUNK_SIZE;
}

void Map::updateChunks(int tileX, int tileY, int radius) {
    chunkMgr.updateAround(tileX, tileY, radius);
}

void draw_ground(const Map& world, const GroundDatabase& ground, int tileSize) {
    // Sichtbaren Tile-Bereich aus der Kamera berechnen
    float halfW = GetScreenWidth()  / 2.0f / camera.zoom;
    float halfH = GetScreenHeight() / 2.0f / camera.zoom;
    int minTX = (int)floorf((camera.target.x - halfW) / tileSize) - 1;
    int minTY = (int)floorf((camera.target.y - halfH) / tileSize) - 1;
    int maxTX = (int)ceilf( (camera.target.x + halfW) / tileSize) + 1;
    int maxTY = (int)ceilf( (camera.target.y + halfH) / tileSize) + 1;

    for (const auto& [key, chunk] : world.chunkMgr.getLoadedChunks()) {
        int originX = chunk.chunkX * CHUNK_SIZE;
        int originY = chunk.chunkY * CHUNK_SIZE;

        for (int ly = 0; ly < CHUNK_SIZE; ly++) {
            int wy = originY + ly;
            if (wy < minTY || wy > maxTY) continue;

            for (int lx = 0; lx < CHUNK_SIZE; lx++) {
                int wx = originX + lx;
                if (wx < minTX || wx > maxTX) continue;

                const std::string& typ = chunk.tiles[ly][lx];
                auto it = ground.texturen.find(typ);
                if (it != ground.texturen.end() && it->second.id != 0) {
                    DrawTexture(it->second, wx * tileSize, wy * tileSize, WHITE);
                } else {
                    DrawRectangle(wx * tileSize, wy * tileSize, tileSize, tileSize, VIOLET);
                }
            }
        }
    }
}
