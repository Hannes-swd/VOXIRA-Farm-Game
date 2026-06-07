#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <vector>
#include "raylib.h"

struct Item {
    std::string id;
    std::string name;
    std::string texturPfad;
    Texture2D textur;
    std::string filePath;

    std::function<void()> onClick;
    std::function<void()> onInventory;
    std::function<void()> onHand;
    std::function<void()> onUpdate;

    // Taste die onClick auslöst (Raylib KeyboardKey, z.B. KEY_E)
    // -1 = keine Taste gesetzt
    int clickKey = -1;

    Item() : textur{0} {}

    void loadTexture() {
        if (!texturPfad.empty()) {
            textur = LoadTexture(texturPfad.c_str());
        }
    }

    void unloadTexture() {
        if (textur.id != 0) {
            UnloadTexture(textur);
        }
    }
};

class ItemManager {
private:
    std::unordered_map<std::string, std::unique_ptr<Item>> items;
    std::unordered_map<std::string, std::function<void()>> functionRegistry;

public:
    ~ItemManager() {
        for (auto& [id, item] : items) {
            item->unloadTexture();
        }
    }

    struct FuncEintrag {
        std::function<void()> fn;
        int taste = -1;
    };
    std::unordered_map<std::string, FuncEintrag> functionRegistryEx;

    // Registriert eine Funktion ohne Taste (onHand, onInventory, onUpdate)
    void registerFunction(const std::string& name, std::function<void()> fn) {
        functionRegistry[name] = fn;
    }

    // Registriert eine Funktion MIT Taste (onClick)
    void registerFunctionWithKey(const std::string& name,
                                      std::function<void()> fn,
                                      int taste) {
        functionRegistryEx[name] = { fn, taste };
        functionRegistry[name]   = fn;
    }

    std::function<void()> findFunc(const std::string& name) const {
        auto it = functionRegistry.find(name);
        if (it != functionRegistry.end()) return it->second;
        return nullptr;
    }

    int findKey(const std::string& name) const {
        auto it = functionRegistryEx.find(name);
        if (it != functionRegistryEx.end()) return it->second.taste;
        return -1;
    }

    void registerItem(std::unique_ptr<Item> item) {
        if (item) items[item->id] = std::move(item);
    }

    void scanAndLoadItems();
    void loadItemFromFile(const std::string& filePath);

    Item* getItem(const std::string& id) {
        auto it = items.find(id);
        if (it != items.end()) return it->second.get();
        return nullptr;
    }

    void execute(Item* item, const std::string& aktion) {
        if (!item) return;
        if      (aktion == "onClick"    && item->onClick)    item->onClick();
        else if (aktion == "onInventory" && item->onInventory)  item->onInventory();
        else if (aktion == "onHand"     && item->onHand)      item->onHand();
        else if (aktion == "onUpdate"   && item->onUpdate)    item->onUpdate();
    }

    void zeichne(Item* item, int x, int y) {
        if (item && item->textur.id != 0)
            DrawTexture(item->textur, x, y, WHITE);
    }

    void updateAllItems() {
        for (auto& [id, item] : items)
            if (item->onUpdate) item->onUpdate();
    }
};

// ── Singleton – eine einzige Instanz im gesamten Programm ────────────────────
inline ItemManager& getItemManager() {
    static ItemManager instance;
    return instance;
}

// g_itemManager als Abkürzung – überall nutzbar, kein "extern" nötig
#define g_itemManager getItemManager()