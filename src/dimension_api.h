#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  dimension_api.h  –  Das einzige Header das ein Dimensions-Modder braucht
//
//  Verwendung in src/dimensions/MeineDimension.cpp:
//
//      #include "dimension_api.h"
//      DIMENSION_BEGIN("house_interior", house_interior)
//
//          void onEnter()  {
//              // Wird aufgerufen wenn der Spieler diese Dimension betritt
//          }
//
//          void onLeave()  {
//              // Wird aufgerufen wenn der Spieler die Dimension verlässt
//          }
//
//          void onUpdate() {
//              // Jedes Frame solange der Spieler in dieser Dimension ist
//              if (IsKeyPressed(KEY_E)) leaveCurrentDimension();
//          }
//
//          void onDraw() {
//              // Optionale Extra-Zeichnung (world-space, innerhalb BeginMode2D)
//          }
//
//      DIMENSION_END("house_interior")
//
//  Alle vier Callbacks muessen definiert sein (leerer Body ist OK).
//  Das zweite Argument von DIMENSION_BEGIN muss ein gueltiger C++-Bezeichner sein.
// ═══════════════════════════════════════════════════════════════════════════════

#include "Dimension.h"
#include "item api.h"
#include <string>
#include <functional>

// ── Hilfsfunktionen für Modder ───────────────────────────────────────────────

// In welcher Dimension ist der Spieler? (leerer String = Hauptwelt)
inline std::string getCurrentDimensionId() { return g_dimensionManager.getCurrentId(); }

// Ist der Spieler gerade in einer Dimension?
inline bool isInDimension() { return g_dimensionManager.isInDimension(); }

// Zurück in die Hauptwelt
inline void leaveCurrentDimension() { switchDimension(""); }

// Setzt einen Tile in der aktiven Dimension (ignoriert Out-of-Bounds)
inline void setDimensionTile(int x, int y, const std::string& type) {
    DimensionData* dim = g_dimensionManager.getCurrentDimension();
    if (!dim) return;
    if (x < 0 || y < 0 || x >= dim->width || y >= dim->height) return;
    dim->tiles[std::to_string(x) + "," + std::to_string(y)] = type;
}

// Liest einen Tile aus der aktiven Dimension
inline std::string getDimensionTile(int x, int y) {
    DimensionData* dim = g_dimensionManager.getCurrentDimension();
    if (!dim) return "";
    std::string key = std::to_string(x) + "," + std::to_string(y);
    auto it = dim->tiles.find(key);
    return (it != dim->tiles.end()) ? it->second : dim->defaultTile;
}

// Groesse der aktuellen Dimension
inline int getDimensionWidth()  { auto* d = g_dimensionManager.getCurrentDimension(); return d ? d->width  : 0; }
inline int getDimensionHeight() { auto* d = g_dimensionManager.getCurrentDimension(); return d ? d->height : 0; }

// ── Interner Auto-Registrar ───────────────────────────────────────────────────
struct _DimensionAutoRegistrar {
    _DimensionAutoRegistrar(const char*           dimId,
                            std::function<void()> fnEnter,
                            std::function<void()> fnLeave,
                            std::function<void()> fnUpdate,
                            std::function<void()> fnDraw)
    {
        std::string id(dimId);
        auto& mgr = g_dimensionManager;
        mgr.registerFunction(id + "_onEnter",  fnEnter);
        mgr.registerFunction(id + "_onLeave",  fnLeave);
        mgr.registerFunction(id + "_onUpdate", fnUpdate);
        mgr.registerFunction(id + "_onDraw",   fnDraw);

        DimensionData* dim = mgr.getDimension(id);
        if (dim) {
            dim->onEnter  = fnEnter;
            dim->onLeave  = fnLeave;
            dim->onUpdate = fnUpdate;
            dim->onDraw   = fnDraw;
        }
    }
};

// ── DIMENSION_BEGIN / DIMENSION_END ───────────────────────────────────────────
//
//  dimId = Dimensions-String, z.B. "house_interior"  (muss mit JSON übereinstimmen)
//  tag   = gültiger C++-Bezeichner, z.B. house_interior
//
//  Alle vier Callbacks müssen definiert werden (leerer Body ist OK).

#define DIMENSION_BEGIN(dimId, tag)                               \
    namespace _dim_impl_##tag {                                   \
        static const char* _DIM_ID = dimId;

#define DIMENSION_END(dimId)                                      \
        static _DimensionAutoRegistrar _reg(                      \
            dimId, onEnter, onLeave, onUpdate, onDraw             \
        );                                                        \
    }
