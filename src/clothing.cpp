#include "clothing.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;
extern std::string assetPath(const std::string& relativ);

// ── Load / Unload ─────────────────────────────────────────────────────────────

void ClothingItem::load() {
    if (!spriteFrontalPath.empty()) texFrontal = LoadTexture(assetPath(spriteFrontalPath).c_str());
    if (!spriteSidePath.empty())    texSide    = LoadTexture(assetPath(spriteSidePath).c_str());
    if (!spriteBackPath.empty())    texBack    = LoadTexture(assetPath(spriteBackPath).c_str());
}

void ClothingItem::unload() {
    if (texFrontal.id) { UnloadTexture(texFrontal); texFrontal = {0}; }
    if (texSide.id)    { UnloadTexture(texSide);    texSide    = {0}; }
    if (texBack.id)    { UnloadTexture(texBack);    texBack    = {0}; }
}

void BodyData::load() {
    if (!spriteFrontalPath.empty()) texFrontal = LoadTexture(assetPath(spriteFrontalPath).c_str());
    if (!spriteSidePath.empty())    texSide    = LoadTexture(assetPath(spriteSidePath).c_str());
}

void BodyData::unload() {
    if (texFrontal.id) { UnloadTexture(texFrontal); texFrontal = {0}; }
    if (texSide.id)    { UnloadTexture(texSide);    texSide    = {0}; }
}

// ── ClothingManager ───────────────────────────────────────────────────────────

ClothingManager::~ClothingManager() {
    for (auto& [id, b] : bodies)    b->unload();
    for (auto& [id, s] : shirts)    s->unload();
    for (auto& [id, p] : pants)     p->unload();
    for (auto& [id, h] : hairItems) h->unload();
}

static std::unique_ptr<ClothingItem> parseClothingItem(const std::string& id, const json& j) {
    auto item = std::make_unique<ClothingItem>();
    item->id               = id;
    item->name             = j.value("name", id);
    item->spriteFrontalPath = j.value("spriteFrontal", "");
    item->spriteSidePath   = j.value("spriteSide", "");
    item->spriteBackPath   = j.value("spriteBack", "");
    item->offsetX          = j.value("offsetX", 0);
    item->offsetY          = j.value("offsetY", 0);
    return item;
}

static std::unique_ptr<BodyData> parseBodyData(const std::string& id, const json& j) {
    auto b = std::make_unique<BodyData>();
    b->id               = id;
    b->name             = j.value("name", id);
    b->spriteFrontalPath = j.value("spriteFrontal", "");
    b->spriteSidePath   = j.value("spriteSide", "");
    b->offsetX          = j.value("offsetX", 0);
    b->offsetY          = j.value("offsetY", 0);
    return b;
}

static bool parseJsonFile(const std::string& path, json& out) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    try {
        f >> out;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Clothing] JSON Fehler (" << path << "): " << e.what() << std::endl;
        return false;
    }
}

void ClothingManager::loadAll() {
    json j;

    if (parseJsonFile(assetPath("json/clothing/bodies.json"), j)) {
        for (auto& [id, data] : j.items()) {
            auto b = parseBodyData(id, data);
            b->load();
            std::cout << "[Clothing] Body: " << id << std::endl;
            bodies[id] = std::move(b);
        }
    }

    j.clear();
    if (parseJsonFile(assetPath("json/clothing/shirts.json"), j)) {
        for (auto& [id, data] : j.items()) {
            auto item = parseClothingItem(id, data);
            item->load();
            std::cout << "[Clothing] Shirt: " << id << std::endl;
            shirts[id] = std::move(item);
        }
    }

    j.clear();
    if (parseJsonFile(assetPath("json/clothing/pants.json"), j)) {
        for (auto& [id, data] : j.items()) {
            auto item = parseClothingItem(id, data);
            item->load();
            std::cout << "[Clothing] Pants: " << id << std::endl;
            pants[id] = std::move(item);
        }
    }

    j.clear();
    if (parseJsonFile(assetPath("json/clothing/hair.json"), j)) {
        for (auto& [id, data] : j.items()) {
            auto item = parseClothingItem(id, data);
            item->load();
            std::cout << "[Clothing] Hair: " << id << std::endl;
            hairItems[id] = std::move(item);
        }
    }
}

BodyData*     ClothingManager::getBody (const std::string& id) { auto it = bodies.find(id);    return it != bodies.end()    ? it->second.get() : nullptr; }
ClothingItem* ClothingManager::getShirt(const std::string& id) { auto it = shirts.find(id);    return it != shirts.end()    ? it->second.get() : nullptr; }
ClothingItem* ClothingManager::getPants(const std::string& id) { auto it = pants.find(id);     return it != pants.end()     ? it->second.get() : nullptr; }
ClothingItem* ClothingManager::getHair (const std::string& id) { auto it = hairItems.find(id); return it != hairItems.end() ? it->second.get() : nullptr; }

// ── Drawing ───────────────────────────────────────────────────────────────────

// Body ist 32×96, Kleidung ist 32×32 – auf dem Bildschirm wird alles so
// skaliert dass der Body genau 1 Tile (32px) hoch erscheint.
static const float DISPLAY_TILE = 32.0f;

static void drawTexFlipped(Texture2D tex, float x, float y, float scale) {
    Rectangle src  = { 0.0f, 0.0f, -(float)tex.width, (float)tex.height };
    Rectangle dest = { x, y, tex.width * scale, tex.height * scale };
    DrawTexturePro(tex, src, dest, {0.0f, 0.0f}, 0.0f, WHITE);
}

static void drawBodyLayer(BodyData* body, direction dir, float x, float y, float scale) {
    if (!body) return;
    Vector2 pos = { x, y };
    if (dir == Up || dir == Down) {
        if (body->texFrontal.id) DrawTextureEx(body->texFrontal, pos, 0.0f, scale, WHITE);
    } else if (dir == Right) {
        if (body->texSide.id)    DrawTextureEx(body->texSide,    pos, 0.0f, scale, WHITE);
    } else {
        if (body->texSide.id)    drawTexFlipped(body->texSide, x, y, scale);
    }
}

static void drawItemLayer(ClothingItem* item, direction dir, float x, float y, float scale) {
    if (!item) return;
    Vector2 pos = { x, y };
    if (dir == Down) {
        if (item->texFrontal.id) DrawTextureEx(item->texFrontal, pos, 0.0f, scale, WHITE);
    } else if (dir == Up) {
        Texture2D tex = item->texBack.id ? item->texBack : item->texFrontal;
        if (tex.id) DrawTextureEx(tex, pos, 0.0f, scale, WHITE);
    } else if (dir == Right) {
        if (item->texSide.id)    DrawTextureEx(item->texSide,    pos, 0.0f, scale, WHITE);
    } else {
        if (item->texSide.id)    drawTexFlipped(item->texSide, x, y, scale);
    }
}

void drawCharacter(BodyData* body, ClothingItem* shirt, ClothingItem* pants,
                   ClothingItem* hair, direction dir, Vector2 pos) {
    // Skalierung: Body (32×96) soll auf dem Bildschirm 1 Tile (32px) hoch sein
    int bodyH = (body && body->texFrontal.id) ? body->texFrontal.height : 96;
    int bodyW = (body && body->texFrontal.id) ? body->texFrontal.width  : 32;
    float scale = DISPLAY_TILE / (float)bodyH;

    float screenW = bodyW * scale;
    float screenH = bodyH * scale;  // = 32px

    // Top-left des Body-Sprites, zentriert auf pos
    float baseX = pos.x - screenW * 0.5f;
    float baseY = pos.y - screenH * 0.5f;

    drawBodyLayer(body, dir,
        baseX + (body ? body->offsetX * scale : 0),
        baseY + (body ? body->offsetY * scale : 0), scale);

    // Kleidungs-offsetY ist in Body-Sprite-Pixeln (0=Kopf, 32=Oberkörper, 64=Unterkörper)
    if (shirt) drawItemLayer(shirt, dir, baseX + shirt->offsetX * scale, baseY + shirt->offsetY * scale, scale);
    if (pants) drawItemLayer(pants, dir, baseX + pants->offsetX * scale, baseY + pants->offsetY * scale, scale);
    if (hair)  drawItemLayer(hair,  dir, baseX + hair->offsetX  * scale, baseY + hair->offsetY  * scale, scale);
}
