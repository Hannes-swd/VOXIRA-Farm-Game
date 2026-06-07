#include "NPC.h"
#include "npc_pathfinding.h"
#include "clothing.h"
#include "map.h"
#include "ground.h"
#include "Dimension.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern Map     world;
extern player* g_player;
extern std::string assetPath(const std::string& relativ);

// ── Hilfsfunktion: Textur-Pfad normalisieren ─────────────────────────────────
static std::string npcNormalizePath(const std::string& raw) {
    for (const std::string& sep : { "assets/", "assets\\" }) {
        size_t pos = raw.rfind(sep);
        if (pos != std::string::npos) {
            std::string rest = raw.substr(pos + sep.size());
            for (char& c : rest) if (c == '\\') c = '/';
            return rest;
        }
    }
    std::string result = raw;
    for (char& c : result) if (c == '\\') c = '/';
    return result;
}

// ── NPCManager::~NPCManager ───────────────────────────────────────────────────
NPCManager::~NPCManager() {
    for (auto& [id, tmpl] : templates)
        tmpl.unloadTexture();
}

// ── Template registrieren ─────────────────────────────────────────────────────
void NPCManager::registerTemplate(const std::string& id, NPCTemplate tmpl) {
    auto it = templates.find(id);
    if (it != templates.end()) {
        // Callbacks überschreiben, JSON-Daten behalten falls schon geladen
        it->second.onUpdate          = tmpl.onUpdate;
        it->second.onProximity       = tmpl.onProximity;
        it->second.onLeaveProximity  = tmpl.onLeaveProximity;
        it->second.onSpawn           = tmpl.onSpawn;
        if (!tmpl.id.empty()) it->second.id = tmpl.id;
    } else {
        templates[id] = std::move(tmpl);
    }
    std::cout << "[NPCManager] Template registriert: " << id << std::endl;
}

// ── JSON-Templates laden ──────────────────────────────────────────────────────
void NPCManager::loadTemplates() {
    namespace fs = std::filesystem;
    const std::string ordner = assetPath("json/npcs/");
    if (!fs::exists(ordner)) {
        std::cout << "[NPCManager] json/npcs/ nicht gefunden – übersprungen." << std::endl;
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(ordner)) {
        if (entry.path().extension() != ".json") continue;

        std::ifstream f(entry.path().string());
        if (!f.is_open()) continue;

        json data;
        try { f >> data; }
        catch (const std::exception& e) {
            std::cerr << "[NPCManager] JSON Fehler in " << entry.path() << ": " << e.what() << std::endl;
            continue;
        }

        for (auto& [id, nd] : data.items()) {
            NPCTemplate& tmpl = templates[id];
            tmpl.id   = id;
            tmpl.name = nd.value("name", id);

            tmpl.bodyId  = nd.value("bodyId",  tmpl.bodyId);
            tmpl.shirtId = nd.value("shirtId", tmpl.shirtId);
            tmpl.pantsId = nd.value("pantsId", tmpl.pantsId);
            tmpl.hairId  = nd.value("hairId",  tmpl.hairId);

            std::string rawTex = nd.value("textur", "");
            if (!rawTex.empty()) {
                tmpl.texturPath = assetPath(npcNormalizePath(rawTex));
                tmpl.loadTexture();
            }

            tmpl.speed            = nd.value("speed",            tmpl.speed);
            tmpl.wanderRadius     = nd.value("wanderRadius",     tmpl.wanderRadius);
            tmpl.proximityRadius  = nd.value("proximityRadius",  tmpl.proximityRadius);
            tmpl.movementEnabled  = nd.value("movementEnabled",  tmpl.movementEnabled);

            if (nd.contains("collision")) {
                auto& col = nd["collision"];
                tmpl.collideWithTiles  = col.value("withTiles",  tmpl.collideWithTiles);
                tmpl.collideWithNpcs   = col.value("withNpcs",   tmpl.collideWithNpcs);
                tmpl.collideWithPlayer = col.value("withPlayer", tmpl.collideWithPlayer);
            }

            std::cout << "[NPCManager] Template geladen: " << id << std::endl;
        }
    }
}

// ── NPC spawnen ───────────────────────────────────────────────────────────────
PlacedNPC& NPCManager::spawnNPC(const std::string& npcId, float x, float y) {
    auto tmplIt = templates.find(npcId);

    PlacedNPC npc;
    npc.npcId      = npcId;
    npc.instanceId = npcId + "_" + std::to_string(nextId++);
    npc.dimensionId = g_dimensionManager.getCurrentId();
    npc.posX       = x;
    npc.posY       = y;
    npc.originX    = x;
    npc.originY    = y;

    if (tmplIt != templates.end()) {
        NPCTemplate& tmpl = tmplIt->second;
        npc.bodyId            = tmpl.bodyId;
        npc.shirtId           = tmpl.shirtId;
        npc.pantsId           = tmpl.pantsId;
        npc.hairId            = tmpl.hairId;
        npc.texturPath        = tmpl.texturPath;
        npc.speed             = tmpl.speed;
        npc.wanderRadius      = tmpl.wanderRadius;
        npc.proximityRadius   = tmpl.proximityRadius;
        npc.movementEnabled   = tmpl.movementEnabled;
        npc.collideWithTiles  = tmpl.collideWithTiles;
        npc.collideWithNpcs   = tmpl.collideWithNpcs;
        npc.collideWithPlayer = tmpl.collideWithPlayer;
        npc.onUpdate          = tmpl.onUpdate;
        npc.onProximity       = tmpl.onProximity;
        npc.onLeaveProximity  = tmpl.onLeaveProximity;
        npc.onSpawn           = tmpl.onSpawn;
    } else {
        std::cerr << "[NPCManager] Template nicht gefunden: " << npcId << std::endl;
    }

    placed.push_back(std::move(npc));
    PlacedNPC& ref = placed.back();

    if (ref.onSpawn) ref.onSpawn(ref);

    std::cout << "[NPCManager] NPC gespawnt: " << ref.instanceId << " @ (" << x << "," << y << ")" << std::endl;
    return ref;
}

// ── NPC entfernen (deferred) ──────────────────────────────────────────────────
void NPCManager::removeNPC(const std::string& instanceId) {
    for (auto& npc : placed) {
        if (npc.instanceId == instanceId) {
            npc._pendingRemoval = true;
            return;
        }
    }
}

// ── NPC nachschlagen ──────────────────────────────────────────────────────────
PlacedNPC* NPCManager::getNPC(const std::string& instanceId) {
    for (auto& npc : placed) {
        if (npc.instanceId == instanceId && !npc._pendingRemoval)
            return &npc;
    }
    return nullptr;
}

// ── Tile-Begehbarkeit (Welt + Dimension) ─────────────────────────────────────
static bool npcTileWalkable(int tx, int ty) {
    if (!world.groundDatabase) return true;
    DimensionData* dim = g_dimensionManager.getCurrentDimension();
    if (dim) {
        if (tx < 0 || ty < 0 || tx >= dim->width || ty >= dim->height) return false;
        std::string key = std::to_string(tx) + "," + std::to_string(ty);
        auto it = dim->tiles.find(key);
        const std::string& type = (it != dim->tiles.end()) ? it->second : dim->defaultTile;
        return world.groundDatabase->isWalkable(type);
    }
    return world.groundDatabase->isWalkable(world.getTile(tx, ty));
}

// ── Pathfinding-Wrapper ───────────────────────────────────────────────────────
std::vector<Vector2> NPCManager::findPath(Vector2 from, Vector2 to) {
    if (!world.groundDatabase) return {};
    DimensionData* dim = g_dimensionManager.getCurrentDimension();
    if (dim) {
        // Alle Tiles der Dimension explizit befüllen (A* erwartet existierende Keys)
        Map dimMap;
        dimMap.defaultType           = dim->defaultTile;
        dimMap.chunkMgr.defaultTile  = dim->defaultTile;
        dimMap.groundDatabase        = world.groundDatabase;
        for (int y = 0; y < dim->height; ++y) {
            for (int x = 0; x < dim->width; ++x) {
                std::string key = std::to_string(x) + "," + std::to_string(y);
                auto it = dim->tiles.find(key);
                dimMap.setTile(x, y, (it != dim->tiles.end()) ? it->second : dim->defaultTile);
            }
        }
        return astar_findPath(from, to, 32, dimMap, *world.groundDatabase,
                              npcSettings().maxPathfindPerFrame * 300);
    }
    return astar_findPath(from, to, 32, world, *world.groundDatabase,
                          npcSettings().maxPathfindPerFrame * 300);
}

// ── Proximity-Update ──────────────────────────────────────────────────────────
void NPCManager::updateProximity(PlacedNPC& npc) {
    if (!g_player) return;
    Vector2 ppos = g_player->Get_position();
    float dx = ppos.x - npc.posX;
    float dy = ppos.y - npc.posY;
    float dist = sqrtf(dx * dx + dy * dy);
    bool inRange = dist <= npc.proximityRadius;

    if (inRange) {
        _lastProximityNPC = &npc;
        if (npc.onProximity) npc.onProximity(npc);
        npc._wasInProximity = true;
    } else if (npc._wasInProximity) {
        if (npc.onLeaveProximity) npc.onLeaveProximity(npc);
        npc._wasInProximity = false;
    }
}

// ── Bewegungs-Update ──────────────────────────────────────────────────────────
void NPCManager::updateMovement(PlacedNPC& npc, float dt) {
    if (!npc.movementEnabled) return;
    if (npc.state == "idle" || npc.state == "custom") return;

    const int TILE = 32;
    const NPCSettings& cfg = npcSettings();

    // Neues Ziel bestimmen
    if (npc.state == "wander") {
        npc.wanderTimer -= dt;
        if (npc.wanderTimer <= 0.0f && npc.path.empty()) {
            if (cfg.pathfindingEnabled && _pathfindThisFrame < cfg.maxPathfindPerFrame) {
                float angle = ((float)rand() / RAND_MAX) * 6.2832f;
                float dist  = ((float)rand() / RAND_MAX) * npc.wanderRadius;
                float tx = npc.originX + cosf(angle) * dist;
                float ty = npc.originY + sinf(angle) * dist;
                npc.path = findPath({npc.posX, npc.posY}, {tx, ty});
                npc.pathIndex = 0;
                ++_pathfindThisFrame;
            }
            float range = cfg.wanderIntervalMax - cfg.wanderIntervalMin;
            npc.wanderTimer = cfg.wanderIntervalMin + ((float)rand() / RAND_MAX) * range;
        }
    } else if (npc.state == "follow_player") {
        if (g_player && npc.path.empty() &&
            cfg.pathfindingEnabled && _pathfindThisFrame < cfg.maxPathfindPerFrame) {
            Vector2 ppos = g_player->Get_position();
            npc.path = findPath({npc.posX, npc.posY}, ppos);
            npc.pathIndex = 0;
            ++_pathfindThisFrame;
        }
    } else if (npc.state == "flee_player") {
        if (g_player && npc.path.empty() &&
            cfg.pathfindingEnabled && _pathfindThisFrame < cfg.maxPathfindPerFrame) {
            Vector2 ppos = g_player->Get_position();
            float dx = npc.posX - ppos.x;
            float dy = npc.posY - ppos.y;
            float len = sqrtf(dx * dx + dy * dy);
            if (len > 0.1f) {
                float fleeTx = npc.posX + (dx / len) * npc.wanderRadius;
                float fleeTy = npc.posY + (dy / len) * npc.wanderRadius;
                npc.path = findPath({npc.posX, npc.posY}, {fleeTx, fleeTy});
                npc.pathIndex = 0;
                ++_pathfindThisFrame;
            }
        }
    } else if (npc.state == "follow_path") {
        // Waypoints sind bereits in npc.path gesetzt (loopen)
        if (npc.path.empty()) return;
        if (npc.pathIndex >= (int)npc.path.size()) npc.pathIndex = 0;
    }
    // "go_to": Pfad wurde von npcGoTo gesetzt, einfach folgen

    // Pfad abfahren
    if (npc.path.empty() || npc.pathIndex >= (int)npc.path.size()) return;

    Vector2 target = npc.path[npc.pathIndex];
    float dx = target.x - npc.posX;
    float dy = target.y - npc.posY;
    float dist = sqrtf(dx * dx + dy * dy);
    float step = npc.speed * dt;

    if (dist <= step) {
        npc.posX = target.x;
        npc.posY = target.y;
        ++npc.pathIndex;
        if (npc.pathIndex >= (int)npc.path.size()) {
            npc.path.clear();
            npc.pathIndex = 0;
            if (npc.state == "go_to") npc.state = "idle";
        }
    } else {
        float nx = dx / dist;
        float ny = dy / dist;
        float newX = npc.posX + nx * step;
        float newY = npc.posY + ny * step;

        // Tile-Kollision (Welt & Dimension)
        if (npc.collideWithTiles) {
            int tx = (int)floorf(newX / TILE);
            int ty = (int)floorf(newY / TILE);
            if (!npcTileWalkable(tx, ty)) {
                // Versuche X-Achse alleine
                if (npcTileWalkable((int)floorf((npc.posX + nx * step) / TILE),
                                    (int)floorf(npc.posY / TILE)))
                    newX = npc.posX + nx * step;
                else
                    newX = npc.posX;
                // Versuche Y-Achse alleine
                if (npcTileWalkable((int)floorf(npc.posX / TILE),
                                    (int)floorf((npc.posY + ny * step) / TILE)))
                    newY = npc.posY + ny * step;
                else
                    newY = npc.posY;
            }
        }

        // Richtung aktualisieren
        if (fabsf(dx) >= fabsf(dy))
            npc.lastDir = (dx > 0.0f) ? Right : Left;
        else
            npc.lastDir = (dy > 0.0f) ? Down : Up;

        npc.posX = newX;
        npc.posY = newY;
    }
}

// ── Update-Schleife ───────────────────────────────────────────────────────────
void NPCManager::update(float deltaTime) {
    _pathfindThisFrame = 0;
    _lastProximityNPC  = nullptr;

    const std::string currentDim = g_dimensionManager.getCurrentId();

    size_t count = placed.size();
    for (size_t i = 0; i < count; ++i) {
        if (placed[i]._pendingRemoval) continue;
        if (placed[i].dimensionId != currentDim) continue;
        updateMovement(placed[i], deltaTime);
        updateProximity(placed[i]);
        if (placed[i].onUpdate) placed[i].onUpdate(placed[i]);
    }

    // Markierte NPCs entfernen
    placed.erase(
        std::remove_if(placed.begin(), placed.end(),
            [](const PlacedNPC& n) { return n._pendingRemoval; }),
        placed.end());
}

// ── Zeichnen (Y-sortiert) ─────────────────────────────────────────────────────
void NPCManager::draw() {
    const std::string currentDim = g_dimensionManager.getCurrentId();

    std::vector<size_t> indices;
    indices.reserve(placed.size());
    for (size_t i = 0; i < placed.size(); ++i) {
        if (!placed[i]._pendingRemoval && placed[i].dimensionId == currentDim)
            indices.push_back(i);
    }
    std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        return placed[a].posY < placed[b].posY;
    });

    for (size_t i : indices) {
        PlacedNPC& npc = placed[i];
        Vector2 pos = {npc.posX, npc.posY};

        if (!npc.bodyId.empty()) {
            BodyData*     body  = g_clothingManager.getBody(npc.bodyId);
            ClothingItem* shirt = npc.shirtId.empty() ? nullptr : g_clothingManager.getShirt(npc.shirtId);
            ClothingItem* pants = npc.pantsId.empty() ? nullptr : g_clothingManager.getPants(npc.pantsId);
            ClothingItem* hair  = npc.hairId.empty()  ? nullptr : g_clothingManager.getHair(npc.hairId);

            if (body) {
                drawCharacter(body, shirt, pants, hair, npc.lastDir, pos);
            } else {
                DrawCircle((int)pos.x, (int)pos.y, 10, PURPLE);
            }
        } else {
            auto tmplIt = templates.find(npc.npcId);
            if (tmplIt != templates.end() && tmplIt->second.textur.id != 0) {
                Texture2D& tex = tmplIt->second.textur;
                Rectangle src = {0, 0, (float)tex.width, (float)tex.height};
                Rectangle dst = {pos.x - tex.width * 0.5f, pos.y - tex.height * 0.5f,
                                 (float)tex.width, (float)tex.height};
                DrawTexturePro(tex, src, dst, {0, 0}, 0.0f, WHITE);
            } else {
                DrawCircle((int)pos.x, (int)pos.y, 10, PURPLE);
            }
        }
    }
}

// ── Speichern ─────────────────────────────────────────────────────────────────
void NPCManager::save(const std::string& path) {
    json data;

    for (auto& npc : placed) {
        if (npc._pendingRemoval) continue;
        json inst;
        inst["npcId"]       = npc.npcId;
        inst["dimensionId"] = npc.dimensionId;
        inst["posX"]        = npc.posX;
        inst["posY"]        = npc.posY;
        inst["bodyId"]  = npc.bodyId;
        inst["shirtId"] = npc.shirtId;
        inst["pantsId"] = npc.pantsId;
        inst["hairId"]  = npc.hairId;
        inst["state"]   = npc.state;

        json inv = json::array();
        for (auto& slot : npc.inventory)
            inv.push_back({{"id", slot.itemId}, {"amount", slot.amount}});
        inst["inventory"] = inv;

        json vars = json::object();
        for (auto& [key, val] : npc.customVars) {
            try {
                if (val.type() == typeid(int))
                    vars[key] = std::any_cast<int>(val);
                else if (val.type() == typeid(float))
                    vars[key] = std::any_cast<float>(val);
                else if (val.type() == typeid(bool))
                    vars[key] = std::any_cast<bool>(val);
                else if (val.type() == typeid(std::string))
                    vars[key] = std::any_cast<std::string>(val);
            } catch (...) {}
        }
        inst["customVars"] = vars;
        inst["extVars"]    = json::object();

        data[npc.instanceId] = inst;
    }

    namespace fs = std::filesystem;
    fs::create_directories(fs::path(path).parent_path());
    std::ofstream f(path);
    if (f.is_open()) {
        f << data.dump(4);
        std::cout << "[NPCManager] Gespeichert: " << path << std::endl;
    } else {
        std::cerr << "[NPCManager] FEHLER: Kann nicht schreiben: " << path << std::endl;
    }
}

// ── Laden ─────────────────────────────────────────────────────────────────────
void NPCManager::load(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cout << "[NPCManager] Keine gespeicherten NPCs: " << path << std::endl;
        return;
    }

    json data;
    try { f >> data; }
    catch (const std::exception& e) {
        std::cerr << "[NPCManager] JSON Fehler: " << e.what() << std::endl;
        return;
    }

    for (auto& [instanceId, inst] : data.items()) {
        std::string npcId = inst.value("npcId", "");
        if (npcId.empty()) continue;

        auto tmplIt = templates.find(npcId);

        PlacedNPC npc;
        npc.npcId       = npcId;
        npc.instanceId  = instanceId;
        npc.dimensionId = inst.value("dimensionId", std::string(""));
        npc.posX        = inst.value("posX", 0.0f);
        npc.posY        = inst.value("posY", 0.0f);
        npc.originX     = npc.posX;
        npc.originY     = npc.posY;
        npc.state       = inst.value("state", std::string("idle"));

        if (tmplIt != templates.end()) {
            NPCTemplate& tmpl    = tmplIt->second;
            npc.bodyId           = tmpl.bodyId;
            npc.shirtId          = tmpl.shirtId;
            npc.pantsId          = tmpl.pantsId;
            npc.hairId           = tmpl.hairId;
            npc.texturPath       = tmpl.texturPath;
            npc.speed            = tmpl.speed;
            npc.wanderRadius     = tmpl.wanderRadius;
            npc.proximityRadius  = tmpl.proximityRadius;
            npc.movementEnabled  = tmpl.movementEnabled;
            npc.collideWithTiles = tmpl.collideWithTiles;
            npc.collideWithNpcs  = tmpl.collideWithNpcs;
            npc.collideWithPlayer = tmpl.collideWithPlayer;
            npc.onUpdate         = tmpl.onUpdate;
            npc.onProximity      = tmpl.onProximity;
            npc.onLeaveProximity = tmpl.onLeaveProximity;
            npc.onSpawn          = tmpl.onSpawn;
        } else {
            std::cerr << "[NPCManager] Template nicht gefunden: " << npcId << std::endl;
        }

        // Instanz-Overrides
        if (inst.contains("bodyId"))  npc.bodyId  = inst.value("bodyId",  npc.bodyId);
        if (inst.contains("shirtId")) npc.shirtId = inst.value("shirtId", npc.shirtId);
        if (inst.contains("pantsId")) npc.pantsId = inst.value("pantsId", npc.pantsId);
        if (inst.contains("hairId"))  npc.hairId  = inst.value("hairId",  npc.hairId);

        // Inventar
        if (inst.contains("inventory") && inst["inventory"].is_array()) {
            for (auto& slot : inst["inventory"]) {
                std::string sid = slot.value("id", "");
                int amt = slot.value("amount", 0);
                if (!sid.empty() && amt > 0)
                    npc.inventory.push_back({sid, amt});
            }
        }

        // CustomVars
        if (inst.contains("customVars") && inst["customVars"].is_object()) {
            for (auto& [key, val] : inst["customVars"].items()) {
                if (val.is_boolean())
                    npc.customVars[key] = val.get<bool>();
                else if (val.is_number_integer())
                    npc.customVars[key] = val.get<int>();
                else if (val.is_number_float())
                    npc.customVars[key] = val.get<float>();
                else if (val.is_string())
                    npc.customVars[key] = val.get<std::string>();
            }
        }

        placed.push_back(std::move(npc));
        std::cout << "[NPCManager] NPC geladen: " << instanceId << std::endl;
    }
}
