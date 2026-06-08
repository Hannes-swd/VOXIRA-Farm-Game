#pragma once
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  WorldGen  –  Optionales Terrain-Generierungssystem für Modder
//
//  Standard: deaktiviert.  Aktivierung + Konfiguration komplett per JSON –
//  kein C++-Code nötig.
//
//  Zwei unabhängige Höhen-Quellen:
//    1. Biome-Karte  → welches Biom     (noise_scale / heightmap)
//    2. Textur-Karte → Tile-Variation innerhalb des Bioms
//                      (tex_noise_scale / texture_heightmap)
// ─────────────────────────────────────────────────────────────────────────────

// Objekt das in einem Biom zufällig platziert wird.
struct BiomeObject {
    std::string id;      // muss in objects.json existieren
    float       chance;  // 0.0 – 1.0 (z.B. 0.10 = 10 % pro Tile)
};

// Tile-Variante innerhalb eines Bioms, gesteuert durch die Textur-Karte.
struct BiomeTile {
    std::string tile;           // muss in ground.json existieren
    float       heightMin = 0.0f;
    float       heightMax = 1.0f;
};

// Ein Biom weist einem Biome-Höhenbereich Tiles und optionale Objekte zu.
struct BiomeRule {
    std::string name;
    float       heightMin = 0.0f;
    float       heightMax = 1.0f;
    std::string tile;                    // Standard-Tile (Fallback)
    std::vector<BiomeTile>   tiles;      // Textur-Varianten per Textur-Karte
    std::vector<BiomeObject> objects;    // zufällige Objekte (Bäume, Felsen…)
};

enum class GenMode { None, Noise, Heightmap };

struct WorldGen {
    // ── Biome-Karte ───────────────────────────────────────────────────────────
    GenMode mode        = GenMode::None;
    float   hmScale     = 1.0f;
    float   noiseScale  = 0.03f;
    int     octaves     = 4;
    float   persistence = 0.5f;
    int     seed        = 0;

    std::string defaultTile = "gras";
    std::vector<BiomeRule> biomes;

    std::vector<float> heightmap;   // Biome-Heightmap-Pixel (0..1)
    int hmWidth = 0, hmHeight = 0;

    // ── Textur-Karte (Variation innerhalb eines Bioms) ────────────────────────
    // Wenn texture_heightmap gesetzt: PNG wird genutzt
    // Sonst: automatisch Noise mit tex_noise_scale (kein extra Bild nötig)
    float texNoiseScale  = 0.08f;
    int   texOctaves     = 2;
    float texPersistence = 0.5f;

    std::vector<float> texHeightmap;  // Textur-Heightmap-Pixel (0..1)
    int thWidth = 0, thHeight = 0;

    // ── API ───────────────────────────────────────────────────────────────────
    bool load(const std::string& configPath);
    bool isEnabled() const { return mode != GenMode::None; }

    std::string generateTile(int worldX, int worldY) const;
    std::string generateObjectAt(int worldX, int worldY) const;

private:
    float            getBiomeHeight  (int worldX, int worldY) const;
    float            getTextureHeight(int worldX, int worldY) const;
    const BiomeRule* getBiome        (float height) const;
};

extern WorldGen g_worldGen;
