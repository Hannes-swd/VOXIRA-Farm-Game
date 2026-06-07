#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  NPC.h  –  Kern-Datenstrukturen des NPC-Systems
// ═══════════════════════════════════════════════════════════════════════════════
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <deque>
#include <any>
#include <memory>
#include "raylib.h"
#include "direction.h"
#include "player.h"

struct PlacedNPC {
    std::string npcId;
    std::string instanceId;
    float posX = 0.0f, posY = 0.0f;
    direction lastDir = Down;

    std::string bodyId, shirtId, pantsId, hairId;
    std::string texturPath;

    std::vector<InventorySlot> inventory;

    bool  movementEnabled = true;
    float speed           = 50.0f;
    float wanderRadius    = 96.0f;
    float originX         = 0.0f, originY = 0.0f;
    float targetX         = 0.0f, targetY = 0.0f;

    float proximityRadius = 80.0f;

    bool collideWithTiles  = true;
    bool collideWithNpcs   = false;
    bool collideWithPlayer = false;

    std::string state      = "idle";
    std::string customData;

    std::vector<Vector2> path;
    int   pathIndex   = 0;
    float wanderTimer = 0.0f;

    // Welche Welt/Dimension dieser NPC gehört ("" = Hauptwelt)
    std::string dimensionId;

    // Internal state
    bool _wasInProximity  = false;
    bool _pendingRemoval  = false;

    std::unordered_map<std::string, std::any> customVars;

    // Globale Zusatzvariablen (Modder befüllt npcs/npc_ext_vars.h)
    #include "npc_ext_vars.h"

    std::function<void(PlacedNPC&)> onUpdate;
    std::function<void(PlacedNPC&)> onProximity;
    std::function<void(PlacedNPC&)> onLeaveProximity;
    std::function<void(PlacedNPC&)> onSpawn;
};

struct NPCTemplate {
    std::string id, name;
    std::string bodyId, shirtId, pantsId, hairId;
    std::string texturPath;
    Texture2D   textur        = {0};
    float speed               = 50.0f;
    float wanderRadius        = 96.0f;
    float proximityRadius     = 80.0f;
    bool  movementEnabled     = true;
    bool  collideWithTiles    = true;
    bool  collideWithNpcs     = false;
    bool  collideWithPlayer   = false;

    std::function<void(PlacedNPC&)> onUpdate;
    std::function<void(PlacedNPC&)> onProximity;
    std::function<void(PlacedNPC&)> onLeaveProximity;
    std::function<void(PlacedNPC&)> onSpawn;

    void loadTexture() {
        if (!texturPath.empty() && textur.id == 0)
            textur = LoadTexture(texturPath.c_str());
    }
    void unloadTexture() {
        if (textur.id != 0) { UnloadTexture(textur); textur = {0}; }
    }
};

class NPCManager {
public:
    ~NPCManager();

    void       registerTemplate(const std::string& id, NPCTemplate tmpl);
    PlacedNPC& spawnNPC(const std::string& npcId, float x, float y);
    void       removeNPC(const std::string& instanceId);
    PlacedNPC* getNPC(const std::string& instanceId);
    std::deque<PlacedNPC>& getAll() { return placed; }

    PlacedNPC* getLastProximityNPC() { return _lastProximityNPC; }

    void update(float deltaTime);
    void draw();

    void loadTemplates();
    void save(const std::string& path);
    void load(const std::string& path);

private:
    std::unordered_map<std::string, NPCTemplate> templates;
    std::deque<PlacedNPC> placed;
    int nextId = 0;
    int _pathfindThisFrame = 0;
    PlacedNPC* _lastProximityNPC = nullptr;

    void updateMovement(PlacedNPC& npc, float deltaTime);
    void updateProximity(PlacedNPC& npc);
    std::vector<Vector2> findPath(Vector2 from, Vector2 to);
};

inline NPCManager& getNPCManager() {
    static NPCManager instance;
    return instance;
}
#define g_npcManager getNPCManager()

// ── Engine-Einstellungen (Zugriff über npcSettings()) ────────────────────────
struct NPCSettings {
    bool  pathfindingEnabled  = true;
    float wanderIntervalMin   = 3.0f;
    float wanderIntervalMax   = 6.0f;
    int   maxPathfindPerFrame = 3;
};

inline NPCSettings& npcSettings() {
    static NPCSettings s;
    return s;
}

inline PlacedNPC& spawnNPC(const std::string& id, float x, float y) {
    return getNPCManager().spawnNPC(id, x, y);
}
