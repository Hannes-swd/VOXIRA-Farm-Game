#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  object_api.h  –  Das einzige Header das ein Objekt-Modder braucht
//
//  Verwendung in objects/Truhe.cpp:
//
//      #include "object_api.h"
//      OBJECT_BEGIN("truhe", Truhe)
//
//          void onInteract() {
//              setPendingTooltip("Truhe geöffnet!");
//          }
//
//          void onUpdate() {
//              if (isPlayerNear(obj, 40.0f))
//                  setPendingTooltip("[E] Truhe öffnen");
//          }
//
//          void onDestroy() { }
//
//      OBJECT_END("truhe")
//
//  Alle drei Callbacks müssen definiert werden (leerer Body ist OK).
//  placeObject("truhe", tileX, tileY) zum Platzieren aus einem Item heraus.
// ═══════════════════════════════════════════════════════════════════════════════

#include "item api.h"
#include "Object.h"
#include <string>
#include <functional>
#include <cmath>

extern PlacedObject* g_activePlacedObject;

// ── Objekt platzieren / entfernen ─────────────────────────────────────────────

inline void placeObject(const std::string& id, int tileX, int tileY) {
    g_objectManager.place(id, tileX, tileY);
}

inline void removeObject(const std::string& instanceId) {
    g_objectManager.remove(instanceId);
}

inline std::string getObjectInstanceId() {
    return g_activePlacedObject ? g_activePlacedObject->instanceId : "";
}

// ── Nähe-Prüfung (nutzbar in onUpdate) ───────────────────────────────────────

inline bool isPlayerNear(const PlacedObject& obj, float radius = 40.0f, int tileSize = 32) {
    if (!g_player) return false;
    ObjectTemplate* tmpl = g_objectManager.getTemplate(obj.objectId);
    float w  = tmpl ? (float)tmpl->width  : 1.0f;
    float h  = tmpl ? (float)tmpl->height : 1.0f;
    float cx = (obj.tileX + w * 0.5f) * (float)tileSize;
    float cy = (obj.tileY + h * 0.5f) * (float)tileSize;
    Vector2 pp = g_player->Get_position();
    float dx = pp.x - cx;
    float dy = pp.y - cy;
    return (dx*dx + dy*dy) <= (radius * radius);
}

// ── Auto-Registrar ────────────────────────────────────────────────────────────

struct _ObjectAutoRegistrar {
    _ObjectAutoRegistrar(const char*                    objId,
                         std::function<void(PlacedObject&)> fnInteract,
                         std::function<void(PlacedObject&)> fnUpdate,
                         std::function<void(PlacedObject&)> fnDestroy)
    {
        g_objectManager.registerCallbacks(objId, fnInteract, fnUpdate, fnDestroy);
    }
};

// ── OBJECT_BEGIN / OBJECT_END ─────────────────────────────────────────────────
//
//  tag = gültiger C++-Bezeichner, z.B. Truhe, Fels, Schalter
//  id  = Object-String, z.B. "truhe", "fels"
//
//  In allen drei Callbacks ist 'obj' (PlacedObject&) direkt verfügbar.

#define OBJECT_BEGIN(objId, tag)                                     \
    namespace _object_impl_##tag {                                   \
        static struct _Reg {                                         \
            _Reg() {                                                 \
                struct _Impl {                                       \
                    PlacedObject& obj;                               \
                    explicit _Impl(PlacedObject& o) : obj(o) {}

#define OBJECT_END(objId)                                            \
                };                                                   \
                static _ObjectAutoRegistrar _r(                      \
                    objId,                                           \
                    [](PlacedObject& o){ _Impl(o).onInteract(); },   \
                    [](PlacedObject& o){ _Impl(o).onUpdate(); },     \
                    [](PlacedObject& o){ _Impl(o).onDestroy(); }     \
                );                                                   \
            }                                                        \
        } _reg;                                                      \
    }
