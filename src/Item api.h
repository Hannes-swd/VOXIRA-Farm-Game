#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  item_api.h  –  Das einzige Header das ein Modder braucht
//
//  Verwendung in items/gras.cpp:
//
//      #include "item_api.h"
//      ITEM_BEGIN("gras", gras)
//
//          void onHand() {
//              setBuildMode(true);
//          }
//
//          void onClick() {
//              Vector2 t = getTileMouse();
//              setTile(t.x, t.y, "gras");
//          }
//
//          void onInventory() {
//              // leer lassen wenn nicht gebraucht
//          }
//
//      ITEM_END("gras")
//
//  WICHTIG: Das zweite Argument von ITEM_BEGIN muss ein gültiger C++-Bezeichner
//  sein (kein String, keine Leerzeichen, keine Bindestriche).
//  Beispiele:
//      ITEM_BEGIN("gras",       gras)
//      ITEM_BEGIN("Beton",      Beton)
//      ITEM_BEGIN("iron_sword", iron_sword)
//
//  Das war's. Kein extern, kein REGISTER_..., kein g_player.
// ═══════════════════════════════════════════════════════════════════════════════

#include "Items.h"
#include "map.h"
#include "player.h"
#include "Mouse tile.h"
#include "Dimension.h"
#include "raylib.h"
#include <string>
#include <functional>

// ── Globale Variablen (einmalig in main.cpp definiert) ───────────────────────
extern Map     world;
extern player* g_player;

// ── Tooltip-System ────────────────────────────────────────────────────────────
// setPendingTooltip() in onHover() aufrufen; main.cpp zeichnet es nach EndMode2D.
#include <string>
namespace _tooltip_impl {
    inline std::string& pendingTooltip() {
        static std::string s;
        return s;
    }
}
inline void setPendingTooltip(const std::string& text) {
    _tooltip_impl::pendingTooltip() = text;
}
// Intern: liest und leert den Tooltip (aufgerufen von main.cpp)
inline std::string consumeTooltip() {
    std::string t = _tooltip_impl::pendingTooltip();
    _tooltip_impl::pendingTooltip().clear();
    return t;
}

// ── Hilfsfunktionen für Modder ───────────────────────────────────────────────

// Setzt einen Tile – in Dimension: auf Dimensionskarte (mit Grenzprüfung), sonst Hauptwelt
inline void setTile(int x, int y, const std::string& typ) {
    if (g_dimensionManager.isInDimension()) {
        DimensionData* dim = g_dimensionManager.getCurrentDimension();
        if (!dim) return;
        if (x < 0 || y < 0 || x >= dim->width || y >= dim->height) return;
        dim->tiles[std::to_string(x) + "," + std::to_string(y)] = typ;
    } else {
        world.setTile(x, y, typ);
    }
}

// Tile position under the mouse
inline Vector2 getTileMouse(int tileGroesse = 32) {
    return getTileMousePos(tileGroesse);
}

// Baumodus (Raster + Vorschau)
inline void  setBuildMode(bool aktiv)  { if (g_player) g_player->setBuildMode(aktiv); }
inline bool  isBuildMode()             { return g_player && g_player->isBuildMode(); }

// Mouse checks
inline bool  IsMouseOnUi()            { return g_player && g_player->IsMouseOnUi(); }
inline bool  leftClick()             { return IsMouseButtonDown(MOUSE_BUTTON_LEFT);  }
inline bool  rightClick()            { return IsMouseButtonDown(MOUSE_BUTTON_RIGHT); }
inline bool  leftClickPressed()      { return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);  }

// ── Interner Auto-Registrar ───────────────────────────────────────────────────
struct _ItemAutoRegistrar {
    _ItemAutoRegistrar(const char* itemId,
                       std::function<void()> fnHand,
                       std::function<void()> fnKlick,
                       std::function<void()> fnInventar)
    {
        std::string id(itemId);

        // Wrapper für onClick: automatisch UI-Check eingebaut
        auto clickWrapper = [fnKlick]() {
            if (!g_player || g_player->IsMouseOnUi()) return;
            fnKlick();
        };

        // In der Registry save (für JSON-Binding)
        getItemManager().registerFunction(id + "_onHand",     fnHand);
        getItemManager().registerFunction(id + "_onKlick",    clickWrapper);
        getItemManager().registerFunction(id + "_onInventory", fnInventar);

        // Sofort ans Item binden falls es bereits aus JSON geladen wurde
        Item* item = getItemManager().getItem(id);
        if (item) {
            item->onHand     = fnHand;
            item->onClick    = clickWrapper;
            item->onInventory = fnInventar;
        }
    }
};

// ── ITEM_BEGIN / ITEM_END ─────────────────────────────────────────────────────
//
//  FIX LNK2005: Jedes Item bekommt einen eigenen Namespace (_item_impl_<tag>),
//  damit onHand/onClick/onInventory nicht über Translation Units hinweg
//  kollidieren.
//
//  tag  = gültiger C++-Bezeichner, z.B. gras, Beton, iron_sword
//  id   = Item-String, z.B. "gras", "Beton", "iron_sword"
//
//  Alle drei Funktionen müssen definiert werden (leerer Body ist OK).

#define ITEM_BEGIN(itemId, tag)                                 \
    namespace _item_impl_##tag {                                \
        static const char* _ITEM_ID = itemId;

#define ITEM_END(itemId)                                        \
        static _ItemAutoRegistrar _reg(                         \
            itemId, onHand, onClick, onInventory                 \
        );                                                      \
    } // end namespace