#pragma once
// ─────────────────────────────────────────────────────────────────────────────
//  Dimension.h  –  Dimensions-System
//
//  Jede Dimension ist eine eigene, begrenzte Karte mit fester Groesse.
//  Der Spieler wechselt per switchDimension("id") hinein und verlässt sie
//  mit switchDimension("") (leerer String = Hauptwelt).
//
//  Für Modder: dimension_api.h verwenden (DIMENSION_BEGIN / DIMENSION_END).
// ─────────────────────────────────────────────────────────────────────────────
#include <string>
#include <functional>
#include <unordered_map>
#include "raylib.h"

struct GroundDatabase;

// ── Datenstruktur einer Dimension ────────────────────────────────────────────

struct DimensionData {
    std::string id;
    std::string name;
    int   width            = 4;
    int   height           = 10;
    Color backgroundColor  = { 50, 30, 20, 255 };
    std::string defaultTile = "gras";
    bool sharedInterior = true;
    std::unordered_map<std::string, std::string> tiles;

    std::function<void()> onEnter;   // Spieler betritt die Dimension
    std::function<void()> onLeave;   // Spieler verlässt die Dimension
    std::function<void()> onUpdate;  // Jedes Frame während Spieler drinnen ist
    std::function<void()> onDraw;    // Extra-Zeichnung (world-space, innerhalb BeginMode2D)
};

// ── DimensionManager (Singleton) ─────────────────────────────────────────────

class DimensionManager {
    std::unordered_map<std::string, DimensionData>         dimensions;
    std::unordered_map<std::string, std::function<void()>> functionRegistry;
    std::string currentId;
    float       savedWorldX = 0.0f;
    float       savedWorldY = 0.0f;

public:
    void load(const std::string& jsonPath);
    void loadDimensionTiles(const std::string& id, const std::string& basePath);
    void saveDimensionTiles(const std::string& id, const std::string& basePath) const;
    void saveAll(const std::string& basePath) const;
    void cloneTemplate(const std::string& templateId, const std::string& newId);

    DimensionData*       getDimension(const std::string& id);
    const DimensionData* getDimension(const std::string& id) const;
    DimensionData*       getCurrentDimension();

    bool               isInDimension() const { return !currentId.empty(); }
    const std::string& getCurrentId()  const { return currentId; }
    float              getSavedWorldX() const { return savedWorldX; }
    float              getSavedWorldY() const { return savedWorldY; }

    void switchTo(const std::string& id);
    void update();

    void registerFunction(const std::string& name, std::function<void()> fn);
    std::function<void()> findFunc(const std::string& name) const;
};

inline DimensionManager& getDimensionManager() {
    static DimensionManager instance;
    return instance;
}
#define g_dimensionManager getDimensionManager()

// Globale Funktion – verwendbar aus building_api.h und dimension_api.h
void switchDimension(const std::string& id);

// Render der Dimension-Tiles (ersetzt draw_ground wenn in einer Dimension)
void draw_dimension(const DimensionData& dim, const GroundDatabase& ground, int tileSize);
