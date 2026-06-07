#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  npc_api.h  –  Das einzige Header das ein NPC-Modder braucht
//
//  Verwendung in npcs/Farmer.cpp:
//
//      #include "npc_api.h"
//
//      NPC_BEGIN("farmer", Farmer)
//
//          NPC_VAR(int, level, 1)
//          NPC_VAR(bool, hostile, false)
//
//          void onSpawn() {
//              NPC_SET(level, 1);   // Standardwert setzen
//          }
//
//          void onUpdate() {
//              // Jeden Frame – npc.posX, npc.posY, npc.state ...
//          }
//
//          void onProximity() {
//              setPendingTooltip("[E] Sprechen");
//              if (IsKeyPressed(KEY_E)) openUI("farmer_dialog");
//          }
//
//          void onLeaveProximity() { }
//
//      NPC_END("farmer")
//
//  WICHTIG: NPC_BEGIN zweites Argument = gültiger C++-Bezeichner (kein String).
//  WICHTIG: Alle 4 Callbacks müssen definiert werden (leerer Body ist OK).
// ═══════════════════════════════════════════════════════════════════════════════

#include "NPC.h"
#include "item api.h"
#include "ui_api.h"
#include <string>
#include <functional>
#include <any>
#include <type_traits>

// ── NPC-Einstellungen (für npc_settings.cpp) ──────────────────────────────────
// NPCSettings & npcSettings() sind in NPC.h definiert.

#define NPC_SETTINGS_BEGIN \
    namespace { \
        static struct _NpcSettingsInit { \
            _NpcSettingsInit() {

#define PATHFINDING_ENABLED(v)     npcSettings().pathfindingEnabled  = (v);
#define WANDER_INTERVAL(mn, mx)    npcSettings().wanderIntervalMin   = (mn); \
                                   npcSettings().wanderIntervalMax   = (mx);
#define MAX_PATHFIND_PER_FRAME(v)  npcSettings().maxPathfindPerFrame = (v);

#define NPC_SETTINGS_END \
        } \
    } _npcSettingsInst; \
}

// ── Typed customVar Accessor ──────────────────────────────────────────────────

template<typename T>
T npc_get_var(PlacedNPC& npc, const char* key, const T& defaultVal = T{}) {
    auto it = npc.customVars.find(key);
    if (it == npc.customVars.end()) return defaultVal;
    try { return std::any_cast<T>(it->second); }
    catch (...) { return defaultVal; }
}

template<typename T>
void npc_set_var(PlacedNPC& npc, const char* key, T val) {
    npc.customVars[key] = std::move(val);
}

// ── NPC-Referenz Hilfsfunktionen ──────────────────────────────────────────────

inline PlacedNPC* npcGetRef(PlacedNPC& npc, const char* name) {
    std::string key = std::string(name) + "_ref";
    auto it = npc.customVars.find(key);
    if (it == npc.customVars.end()) return nullptr;
    try {
        std::string id = std::any_cast<std::string>(it->second);
        if (id.empty()) return nullptr;
        return g_npcManager.getNPC(id);
    } catch (...) { return nullptr; }
}

inline void npcSetRef(PlacedNPC& npc, const char* name, const std::string& instanceId) {
    npc.customVars[std::string(name) + "_ref"] = instanceId;
}

// ── Bewegungs-API ─────────────────────────────────────────────────────────────

inline void npcGoTo(PlacedNPC& npc, float x, float y) {
    npc.targetX = x;
    npc.targetY = y;
    npc.path.clear();
    npc.pathIndex = 0;
    npc.state = "go_to";
}

inline void npcSetWander(PlacedNPC& npc, bool enable) {
    if (enable) {
        npc.state = "wander";
        npc.wanderTimer = 0.0f;
    } else {
        npc.state = "idle";
        npc.path.clear();
        npc.pathIndex = 0;
    }
}

inline void npcSetMovement(PlacedNPC& npc, bool enable) {
    npc.movementEnabled = enable;
    if (!enable) {
        npc.path.clear();
        npc.pathIndex = 0;
        npc.state = "idle";
    }
}

inline void npcSetWaypoints(PlacedNPC& npc, std::vector<Vector2> waypoints) {
    npc.path      = std::move(waypoints);
    npc.pathIndex = 0;
    npc.state     = "follow_path";
}

inline void npcFollowPlayer(PlacedNPC& npc) {
    npc.state = "follow_player";
    npc.path.clear();
    npc.pathIndex = 0;
}

inline void npcFleePlayer(PlacedNPC& npc) {
    npc.state = "flee_player";
    npc.path.clear();
    npc.pathIndex = 0;
}

inline void npcSetSpeed(PlacedNPC& npc, float speed) {
    npc.speed = speed;
}

// ── Inventar-API ──────────────────────────────────────────────────────────────

inline void npcAddItem(PlacedNPC& npc, const std::string& itemId, int amount = 1) {
    for (auto& slot : npc.inventory) {
        if (slot.itemId == itemId) { slot.amount += amount; return; }
    }
    npc.inventory.push_back({itemId, amount});
}

inline void npcRemoveItem(PlacedNPC& npc, const std::string& itemId, int amount = 1) {
    for (auto it = npc.inventory.begin(); it != npc.inventory.end(); ++it) {
        if (it->itemId == itemId) {
            it->amount -= amount;
            if (it->amount <= 0) npc.inventory.erase(it);
            return;
        }
    }
}

inline bool npcHasItem(const PlacedNPC& npc, const std::string& itemId) {
    for (auto& slot : npc.inventory)
        if (slot.itemId == itemId) return true;
    return false;
}

inline std::vector<InventorySlot>& npcGetInventory(PlacedNPC& npc) {
    return npc.inventory;
}

// ── NPC_BEGIN / NPC_END ───────────────────────────────────────────────────────
//
//  Jeder NPC-Typ bekommt einen eigenen Namespace (npc_<tag>) um LNK2005 zu
//  vermeiden, genau wie bei ITEM_BEGIN / BUILDING_BEGIN.
//
//  _Impl hält eine Referenz auf den PlacedNPC der gerade verarbeitet wird.
//  npc  = die PlacedNPC-Instanz (in allen Callbacks direkt nutzbar).
//
//  NPC_VAR(type, name, def):
//    Deklariert 'type name_def = def' als Member von _Impl.
//    Wird von NPC_GET für Typ-Deduktion genutzt.
//    Initialisierung in onSpawn() via NPC_SET(name, def).
//
//  NPC_GET(name) → npc_get_var(npc, #name, name##_def)
//    Gibt customVars["name"] zurück, oder name_def falls nicht gesetzt.
//
//  NPC_SET(name, val) → npc_set_var(npc, #name, val)
//    Setzt customVars["name"] = val (persistent, wird gespeichert).

#define NPC_BEGIN(id_str, class_name) \
namespace npc_##class_name { \
    static struct _Reg { \
        _Reg() { \
            NPCTemplate t; \
            t.id = (id_str); \
            struct _Impl { \
                PlacedNPC& npc; \
                explicit _Impl(PlacedNPC& n) : npc(n) {}

#define NPC_VAR(type, name, def) \
                type name##_def = (type)(def);

#define NPC_REF(name) \
                std::string name##_ref_def;

#define NPC_GET(name)      npc_get_var(npc, #name, name##_def)
#define NPC_SET(name, val) npc_set_var(npc, #name, (val))

#define NPC_END(id_str) \
            }; \
            t.onUpdate         = [](PlacedNPC& n) { _Impl(n).onUpdate(); }; \
            t.onProximity      = [](PlacedNPC& n) { _Impl(n).onProximity(); }; \
            t.onLeaveProximity = [](PlacedNPC& n) { _Impl(n).onLeaveProximity(); }; \
            t.onSpawn          = [](PlacedNPC& n) { _Impl(n).onSpawn(); }; \
            getNPCManager().registerTemplate((id_str), std::move(t)); \
        } \
    } _r; \
}
