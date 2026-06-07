#include "player.h"
#include "clothing.h"
#include "inventory.h"
#include "Cam.h"
#include "map.h"
#include "ground.h"
#include "Dimension.h"
#include "Buildings.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cmath>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Asset path helper (defined in main.cpp)
extern std::string assetPath(const std::string& relativ);
extern Map world;

// ── Inventory-Methoden ────────────────────────────────────────────────────────

Item* player::getHandItem() const {
    if (currentSlot < (int)inventory.size()) {
        const std::string& id = inventory[currentSlot].itemId;
        if (!id.empty()) return g_itemManager.getItem(id);
    }
    return nullptr;
}

void player::addToInventory(const std::string& itemId, int amount) {
    for (auto& slot : inventory) {
        if (slot.itemId == itemId) {
            slot.amount += amount;
            return;
        }
    }
    inventory.push_back({ itemId, amount });
}

void player::removeFromInventory(const std::string& itemId, int amount) {
    for (auto it = inventory.begin(); it != inventory.end(); ++it) {
        if (it->itemId == itemId) {
            it->amount -= amount;
            if (it->amount <= 0) inventory.erase(it);
            return;
        }
    }
}

bool player::hasItem(const std::string& itemId) const {
    for (const auto& slot : inventory)
        if (slot.itemId == itemId) return true;
    return false;
}

void player::swapSlots(int a, int b) {
    auto& inv = inventory;
    while ((int)inv.size() <= std::max(a, b))
        inv.push_back({"", 0});
    std::swap(inv[a], inv[b]);
    while (!inv.empty() && inv.back().itemId.empty())
        inv.pop_back();
}

// ── Speichern / Laden ─────────────────────────────────────────────────────────

void loadPlayer(player& p) {
    std::string path = assetPath("json/player/player.json");
    std::ifstream f(path);

    if (f.is_open()) {
        std::cout << "[loadPlayer] Lade: " << path << std::endl;
        try {
            json data;
            f >> data;
            p.Set_position(data.value("posX", 0), data.value("posY", 0));
            p.Change_Name(data.value("name", std::string("Spieler")));
            p.setCurrentSlot(data.value("currentSlot", 0));
            p.setBuildMode(data.value("buildMode", false));
            p.setBodyId (data.value("bodyId",  std::string("human_base")));
            p.setShirtId(data.value("shirtId", std::string("")));
            p.setPantsId(data.value("pantsId", std::string("")));
            p.setHairId (data.value("hairId",  std::string("")));

            if (data.contains("ownedShirts") && data["ownedShirts"].is_array())
                for (auto& id : data["ownedShirts"]) p.addOwnedShirt(id.get<std::string>());
            if (data.contains("ownedPants") && data["ownedPants"].is_array())
                for (auto& id : data["ownedPants"])  p.addOwnedPants(id.get<std::string>());
            if (data.contains("ownedHair") && data["ownedHair"].is_array())
                for (auto& id : data["ownedHair"])   p.addOwnedHair(id.get<std::string>());

            if (data.contains("inventory") && data["inventory"].is_array()) {
                for (const auto& slot : data["inventory"]) {
                    std::string id  = slot.value("id",     "");
                    int         anz = slot.value("amount", 1);
                    if (!id.empty())
                        p.addToInventory(id, anz);
                    std::cout << "[loadPlayer] Slot: " << id << " x" << anz << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[loadPlayer] JSON Fehler: " << e.what() << " – Fallback auf Standardwerte." << std::endl;
            p.Set_position(0, 0);
            p.Change_Name("Spieler");
        }
    } else {
        std::cout << "[loadPlayer] Nicht gefunden -> Fallback (" << path << ")" << std::endl;
        p.Set_position(0, 0);
        p.Change_Name("Spieler");
    }

    std::cout << "[loadPlayer] Inventar: " << p.getInventory().size() << " slots" << std::endl;
}

void savePlayer(const player& p) {
    json data;
    Vector2 pos           = p.Get_position();
    data["posX"]          = (int)pos.x;
    data["posY"]          = (int)pos.y;
    data["name"]          = p.Get_Name();
    data["currentSlot"] = p.getCurrentSlot();
    data["buildMode"]   = p.isBuildMode();
    data["bodyId"]  = p.getBodyId();
    data["shirtId"] = p.getShirtId();
    data["pantsId"] = p.getPantsId();
    data["hairId"]  = p.getHairId();

    json shirtsArr = json::array();
    for (auto& id : p.getOwnedShirts()) shirtsArr.push_back(id);
    data["ownedShirts"] = shirtsArr;

    json pantsArr = json::array();
    for (auto& id : p.getOwnedPants()) pantsArr.push_back(id);
    data["ownedPants"] = pantsArr;

    json hairArr = json::array();
    for (auto& id : p.getOwnedHair()) hairArr.push_back(id);
    data["ownedHair"] = hairArr;

    json inventoryArray = json::array();
    for (const auto& slot : p.getInventory()) {
        inventoryArray.push_back({
            {"id",     slot.itemId},
            {"amount", slot.amount}
        });
    }
    data["inventory"] = inventoryArray;

    namespace fs = std::filesystem;
    std::string path = assetPath("json/player/player.json");
    fs::create_directories(fs::path(path).parent_path());

    std::ofstream f(path);
    if (f.is_open()) {
        f << data.dump(4);
        std::cout << "[savePlayer] Gespeichert: " << path << std::endl;
    } else {
        std::cerr << "[savePlayer] FEHLER: Kann nicht schreiben: " << path << std::endl;
    }
}

// ── Bewegung ──────────────────────────────────────────────────────────────────

void updatePlayer(player& p) {
    float delta = GetFrameTime();
    const int   TILE_SIZE     = 32;
    const float PLAYER_RADIUS = 10.0f;

    Vector2 pos  = p.Get_position();
    float   newX = pos.x;
    float   newY = pos.y;

    bool movingX = false, movingY = false;
    if (IsKeyDown(KEY_W)) { newY -= p.getSpeed() * delta; movingY = true; p.setLastDir(Up); }
    if (IsKeyDown(KEY_S)) { newY += p.getSpeed() * delta; movingY = true; p.setLastDir(Down); }
    if (IsKeyDown(KEY_A)) { newX -= p.getSpeed() * delta; movingX = true; p.setLastDir(Left); }
    if (IsKeyDown(KEY_D)) { newX += p.getSpeed() * delta; movingX = true; p.setLastDir(Right); }

    DimensionData* dim = g_dimensionManager.getCurrentDimension();

    auto canMoveTo = [&](float x, float y) -> bool {
        if (dim) {
            // In Dimension: Spieler darf Grenzen nicht überschreiten
            float r = PLAYER_RADIUS;
            return x >= r && y >= r
                && x <= dim->width  * TILE_SIZE - r
                && y <= dim->height * TILE_SIZE - r;
        } else {
            // Hauptwelt: Tile muss existieren und begehbar sein
            int tx = (int)floorf(x / TILE_SIZE);
            int ty = (int)floorf(y / TILE_SIZE);
            if (world.groundDatabase && !world.groundDatabase->isWalkable(world.getTile(tx, ty)))
                return false;
            // Gebäude-Kollision: alle Tiles die der Spieler-Kreis überlappt prüfen
            int txMin = (int)floorf((x - PLAYER_RADIUS) / TILE_SIZE);
            int txMax = (int)floorf((x + PLAYER_RADIUS) / TILE_SIZE);
            int tyMin = (int)floorf((y - PLAYER_RADIUS) / TILE_SIZE);
            int tyMax = (int)floorf((y + PLAYER_RADIUS) / TILE_SIZE);
            for (int cy = tyMin; cy <= tyMax; ++cy) {
                for (int cx = txMin; cx <= txMax; ++cx) {
                    PlacedBuilding* pb = world.getBuildingAt(cx, cy);
                    if (pb) {
                        Building* b = g_buildingManager.getBuilding(pb->buildingId);
                        if (b && b->solid) return false;
                    }
                }
            }
            return true;
        }
    };

    if (movingX && canMoveTo(newX, pos.y)) pos.x = newX;
    if (movingY && canMoveTo(pos.x, newY)) pos.y = newY;
    p.setPositionF(pos.x, pos.y);

    // Slot-Auswahl per Tastatur (1-0)
    for (int i = 0; i < 10; i++) {
        if (IsKeyPressed(KEY_ONE + i))
            p.setCurrentSlot(i);
    }

    // Mausrad
    float rad = GetMouseWheelMove();
    if (rad != 0.0f) {
        int newSlot = p.getCurrentSlot() - (int)rad;
        if (newSlot < 0) newSlot = 9;
        if (newSlot > 9) newSlot = 0;
        p.setCurrentSlot(newSlot);
    }

    if (IsKeyPressed(KEY_TAB)) p.toggleInventory();
    if (IsKeyPressed(KEY_B))   p.toggleBuildMode();

    // onHand: jeden Frame fuer das aktive Item
    Item* hand = p.getHandItem();
    if (hand && hand->onHand) hand->onHand();

    // onInventory: jeden Frame fuer alle Items im Inventar
    for (const auto& slot : p.getInventory()) {
        if (slot.itemId.empty()) continue;
        Item* item = g_itemManager.getItem(slot.itemId);
        if (item && item->onInventory) item->onInventory();
    }

    // onClick: kein Ausloesen wenn Maus ueber UI liegt
    if (hand && hand->onClick && !p.IsMouseOnUi()) {
        bool ausloesen = false;

        if (hand->clickKey >= 0)
            ausloesen = IsKeyDown(hand->clickKey);

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            ausloesen = true;

        if (ausloesen)
            hand->onClick();
    }

    Vector2 camPos   = p.Get_position();
    camera.target.x = camPos.x;
    camera.target.y = camPos.y;
}

// ── Zeichnen ──────────────────────────────────────────────────────────────────

void drawPlayer(player& p) {
    Vector2 pos = p.Get_position();

    BodyData*     body  = g_clothingManager.getBody(p.getBodyId());
    ClothingItem* shirt = p.getShirtId().empty() ? nullptr : g_clothingManager.getShirt(p.getShirtId());
    ClothingItem* pants = p.getPantsId().empty() ? nullptr : g_clothingManager.getPants(p.getPantsId());
    ClothingItem* hair  = p.getHairId().empty()  ? nullptr : g_clothingManager.getHair(p.getHairId());

    bool hasSprite = body && (body->texFrontal.id || body->texSide.id);
    if (hasSprite) {
        drawCharacter(body, shirt, pants, hair, p.getLastDir(), pos);
    } else {
        DrawCircle((int)pos.x, (int)pos.y, 10, ORANGE);
    }

    Item* hand = p.getHandItem();
    if (hand && hand->textur.id != 0) {
        const int HAND_SIZE = 14;
        float scaleX = (float)HAND_SIZE / hand->textur.width;
        float scaleY = (float)HAND_SIZE / hand->textur.height;
        float scale  = (scaleX < scaleY) ? scaleX : scaleY;
        Vector2 drawPos = { pos.x + 10, pos.y - 18 };
        DrawTextureEx(hand->textur, drawPos, 0.0f, scale, WHITE);
    }
}