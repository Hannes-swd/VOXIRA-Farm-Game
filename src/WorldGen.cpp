#include "WorldGen.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <nlohmann/json.hpp>
#include "raylib.h"

using json = nlohmann::json;

WorldGen g_worldGen;

extern std::string assetPath(const std::string& relativ);

// ── Value Noise 2D mit Fractal-Oktaven ───────────────────────────────────────

static uint32_t hashCoord(int x, int y, int seed) {
    uint32_t h = (uint32_t)x * 374761393u
               ^ (uint32_t)y * 668265263u
               ^ (uint32_t)seed * 1274126177u;
    h ^= h >> 13;
    h *= 1274126177u;
    h ^= h >> 16;
    return h;
}

static float valueNoise(float x, float y, int seed) {
    int   ix = (int)floorf(x), iy = (int)floorf(y);
    float fx = x - ix,         fy = y - iy;
    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uy = fy * fy * (3.0f - 2.0f * fy);

    float v00 = (hashCoord(ix,   iy,   seed) & 0xFFFF) / 65535.0f;
    float v10 = (hashCoord(ix+1, iy,   seed) & 0xFFFF) / 65535.0f;
    float v01 = (hashCoord(ix,   iy+1, seed) & 0xFFFF) / 65535.0f;
    float v11 = (hashCoord(ix+1, iy+1, seed) & 0xFFFF) / 65535.0f;

    return v00
         + (v10 - v00) * ux
         + (v01 - v00) * uy
         + (v00 - v10 - v01 + v11) * ux * uy;
}

static float fractalNoise(float x, float y, int seed, int octaves, float persistence) {
    float value = 0.0f, amplitude = 1.0f, frequency = 1.0f, total = 0.0f;
    for (int i = 0; i < octaves; i++) {
        value     += valueNoise(x * frequency, y * frequency, seed + i * 7919) * amplitude;
        total     += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    return value / total;
}

// ── Heightmap laden ───────────────────────────────────────────────────────────

static bool loadHeightmapPNG(const std::string& relPath,
                              std::vector<float>& out, int& w, int& h) {
    std::string full = assetPath(relPath);
    Image img = LoadImage(full.c_str());
    if (!img.data) {
        std::cerr << "[WorldGen] Heightmap nicht gefunden: " << full << std::endl;
        return false;
    }
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
    w = img.width;
    h = img.height;
    auto* px = (unsigned char*)img.data;
    out.resize(w * h);
    for (int i = 0; i < w * h; i++) out[i] = px[i] / 255.0f;
    UnloadImage(img);
    std::cout << "[WorldGen] Heightmap geladen: " << full
              << " (" << w << "x" << h << "px)" << std::endl;
    return true;
}

// ── WorldGen::load ────────────────────────────────────────────────────────────

bool WorldGen::load(const std::string& configPath) {
    std::ifstream f(configPath);
    if (!f.is_open()) return false;

    json cfg;
    try { f >> cfg; }
    catch (const std::exception& e) {
        std::cerr << "[WorldGen] JSON Fehler in " << configPath << ": " << e.what() << std::endl;
        return false;
    }

    if (!cfg.value("enabled", false)) { mode = GenMode::None; return false; }

    std::string modeStr = cfg.value("mode", "noise");
    mode = (modeStr == "heightmap") ? GenMode::Heightmap : GenMode::Noise;

    hmScale     = cfg.value("heightmap_scale", 1.0f);
    seed        = cfg.value("seed",           0);
    noiseScale  = cfg.value("noise_scale",  0.03f);
    octaves     = cfg.value("octaves",      4);
    persistence = cfg.value("persistence",  0.5f);
    defaultTile = cfg.value("default_tile", std::string("gras"));

    texNoiseScale  = cfg.value("tex_noise_scale",  0.08f);
    texOctaves     = cfg.value("tex_octaves",      2);
    texPersistence = cfg.value("tex_persistence",  0.5f);

    // ── Biome-Heightmap ───────────────────────────────────────────────────────
    if (mode == GenMode::Heightmap) {
        std::string hmFile = cfg.value("heightmap", "");
        std::string hmRel  = hmFile.empty() ? "" : "Images/Map/heightmaps/" + hmFile;
        if (hmRel.empty() || !loadHeightmapPNG(hmRel, heightmap, hmWidth, hmHeight)) {
            std::cerr << "[WorldGen] Biome-Heightmap fehlt – Fallback auf Noise." << std::endl;
            mode = GenMode::Noise;
        }
    }

    // ── Textur-Heightmap (optional, für beide Modi) ───────────────────────────
    std::string thRel = cfg.value("texture_heightmap", "");
    if (!thRel.empty())
        loadHeightmapPNG(thRel, texHeightmap, thWidth, thHeight);

    // ── Biome laden ───────────────────────────────────────────────────────────
    biomes.clear();
    if (cfg.contains("biomes")) {
        for (auto& b : cfg["biomes"]) {
            BiomeRule rule;
            rule.name      = b.value("name",       "");
            rule.heightMin = b.value("height_min",  0.0f);
            rule.heightMax = b.value("height_max",  1.0f);
            rule.tile      = b.value("tile",        defaultTile);

            // Textur-Varianten innerhalb des Bioms
            if (b.contains("tiles")) {
                for (auto& t : b["tiles"]) {
                    BiomeTile bt;
                    bt.tile      = t.value("tile",       rule.tile);
                    bt.heightMin = t.value("height_min", 0.0f);
                    bt.heightMax = t.value("height_max", 1.0f);
                    rule.tiles.push_back(bt);
                }
            }

            // Objekte
            if (b.contains("objects")) {
                for (auto& o : b["objects"]) {
                    BiomeObject bo;
                    bo.id     = o.value("id",     "");
                    bo.chance = o.value("chance",  0.0f);
                    if (!bo.id.empty() && bo.chance > 0.0f)
                        rule.objects.push_back(bo);
                }
            }

            biomes.push_back(rule);
        }
    }

    std::cout << "[WorldGen] Aktiv – Modus: "
              << (mode == GenMode::Heightmap ? "heightmap" : "noise")
              << " | Textur-Karte: " << (thWidth > 0 ? "PNG" : "Noise")
              << " | " << biomes.size() << " Biome" << std::endl;
    return true;
}

// ── Höhenwerte abrufen ────────────────────────────────────────────────────────

float WorldGen::getBiomeHeight(int wx, int wy) const {
    if (mode == GenMode::Heightmap && hmWidth > 0) {
        int px = ((int)floorf((float)wx / hmScale) % hmWidth  + hmWidth)  % hmWidth;
        int py = ((int)floorf((float)wy / hmScale) % hmHeight + hmHeight) % hmHeight;
        return heightmap[py * hmWidth + px];
    }
    return fractalNoise((float)wx * noiseScale, (float)wy * noiseScale,
                        seed, octaves, persistence);
}

float WorldGen::getTextureHeight(int wx, int wy) const {
    if (thWidth > 0) {
        int px = ((int)floorf((float)wx / hmScale) % thWidth  + thWidth)  % thWidth;
        int py = ((int)floorf((float)wy / hmScale) % thHeight + thHeight) % thHeight;
        return texHeightmap[py * thWidth + px];
    }
    // Kein separates Textur-PNG → Biome-Heightmap nutzen falls vorhanden
    if (hmWidth > 0) {
        int px = ((int)floorf((float)wx / hmScale) % hmWidth  + hmWidth)  % hmWidth;
        int py = ((int)floorf((float)wy / hmScale) % hmHeight + hmHeight) % hmHeight;
        return heightmap[py * hmWidth + px];
    }
    return fractalNoise((float)wx * texNoiseScale, (float)wy * texNoiseScale,
                        seed + 99991, texOctaves, texPersistence);
}

const BiomeRule* WorldGen::getBiome(float height) const {
    for (const auto& b : biomes)
        if (height >= b.heightMin && height < b.heightMax) return &b;
    return nullptr;
}

// ── Tile und Objekt generieren ────────────────────────────────────────────────

std::string WorldGen::generateTile(int wx, int wy) const {
    const BiomeRule* b = getBiome(getBiomeHeight(wx, wy));
    if (!b) return defaultTile;

    if (!b->tiles.empty()) {
        float th = getTextureHeight(wx, wy);
        for (const auto& bt : b->tiles)
            if (th >= bt.heightMin && th < bt.heightMax) return bt.tile;
    }
    return b->tile;
}

std::string WorldGen::generateObjectAt(int wx, int wy) const {
    const BiomeRule* b = getBiome(getBiomeHeight(wx, wy));
    if (!b || b->objects.empty()) return "";

    uint32_t r     = hashCoord(wx, wy, seed + 999983);
    float    roll  = (r & 0xFFFF) / 65535.0f;
    float    cumul = 0.0f;
    for (const auto& obj : b->objects) {
        cumul += obj.chance;
        if (roll < cumul) return obj.id;
    }
    return "";
}
