#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "raylib.h"
#include "direction.h"

struct ClothingItem {
    std::string id;
    std::string name;
    std::string spriteFrontalPath;
    std::string spriteSidePath;
    std::string spriteBackPath;
    Texture2D   texFrontal = {0};
    Texture2D   texSide    = {0};
    Texture2D   texBack    = {0};
    int offsetX = 0;
    int offsetY = 0;

    void load();
    void unload();
};

struct BodyData {
    std::string id;
    std::string name;
    std::string spriteFrontalPath;
    std::string spriteSidePath;
    Texture2D   texFrontal = {0};
    Texture2D   texSide    = {0};
    int offsetX = 0;
    int offsetY = 0;

    void load();
    void unload();
};

class ClothingManager {
    std::unordered_map<std::string, std::unique_ptr<BodyData>>     bodies;
    std::unordered_map<std::string, std::unique_ptr<ClothingItem>> shirts;
    std::unordered_map<std::string, std::unique_ptr<ClothingItem>> pants;
    std::unordered_map<std::string, std::unique_ptr<ClothingItem>> hairItems;

public:
    ~ClothingManager();
    void loadAll();

    BodyData*     getBody (const std::string& id);
    ClothingItem* getShirt(const std::string& id);
    ClothingItem* getPants(const std::string& id);
    ClothingItem* getHair (const std::string& id);
};

// Zeichnet Body + Kleidungs-Layer in der richtigen Reihenfolge.
// pos = Spieler-Center; baseY so dass Oberkörper-Mitte bei pos.y liegt.
void drawCharacter(BodyData* body, ClothingItem* shirt, ClothingItem* pants,
                   ClothingItem* hair, direction dir, Vector2 pos);

inline ClothingManager& getClothingManager() {
    static ClothingManager instance;
    return instance;
}
#define g_clothingManager getClothingManager()
