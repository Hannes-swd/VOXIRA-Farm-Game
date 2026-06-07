#include "Items.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Asset path helper (defined in main.cpp)
extern std::string assetPath(const std::string& relativ);

// Normalizes a texture path: absolute path from old device → relative path
// Gleiche Logik wie in ground.cpp
static std::string normalizePath(const std::string& rawPath) {
    for (const std::string& sep : { "assets/", "assets\\" }) {
        size_t pos = rawPath.rfind(sep);
        if (pos != std::string::npos) {
            std::string rest = rawPath.substr(pos + sep.size());
            for (char& c : rest) if (c == '\\') c = '/';
            return rest;
        }
    }
    std::string result = rawPath;
    for (char& c : result) if (c == '\\') c = '/';
    return result;
}

// Verbindet einen JSON-Funktionsnamen mit der Registry
static void bindFunc(std::function<void()>& target,
                     const json& data,
                     const std::string& jsonKey)
{
    if (!data.contains(jsonKey)) return;
    std::string funcName = data[jsonKey].get<std::string>();
    if (funcName.empty()) return;

    auto fn = g_itemManager.findFunc(funcName);
    if (fn) {
        target = fn;
        std::cout << "[ItemManager] Callback gebunden: " << jsonKey
                  << " -> " << funcName << std::endl;
    } else {
        std::cerr << "[ItemManager] WARNUNG: Funktion '" << funcName
                  << "' (fuer " << jsonKey << ") nicht in der Registry!" << std::endl;
    }
}

static void bindKlick(Item& item, const json& data, const std::string& jsonKey)
{
    if (!data.contains(jsonKey)) return;
    std::string funcName = data[jsonKey].get<std::string>();
    if (funcName.empty()) return;

    auto fn = g_itemManager.findFunc(funcName);
    if (fn) {
        item.onClick    = fn;
        item.clickKey = g_itemManager.findKey(funcName);
        std::cout << "[ItemManager] onClick gebunden: " << funcName
                  << "  Taste=" << item.clickKey << std::endl;
    } else {
        std::cerr << "[ItemManager] WARNUNG: Funktion '" << funcName
                  << "' (fuer " << jsonKey << ") nicht in der Registry!" << std::endl;
    }
}

// Laedt alle Items aus einer einzelnen JSON-Datei
static void loadItemsFromJson(const std::string& path,
                              std::unordered_map<std::string, std::unique_ptr<Item>>& items)
{
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[ItemManager] Kann nicht oeffnen: " << path << std::endl;
        return;
    }

    json data;
    try {
        f >> data;
    } catch (const std::exception& e) {
        std::cerr << "[ItemManager] JSON Fehler in " << path << ": " << e.what() << std::endl;
        return;
    }

    for (auto& [id, itemData] : data.items()) {
        auto item  = std::make_unique<Item>();
        item->id   = id;
        item->name = itemData.value("name", id);

        // Texturpfad normalisieren (absolut → relativ)
        std::string rawPath = itemData.value("textur", "");
        if (!rawPath.empty()) {
            std::string relPath  = normalizePath(rawPath);
            item->texturPfad     = assetPath(relPath);
        }

        item->filePath = path;
        item->loadTexture();

        bindKlick(*item,           itemData, "onClick");
        bindFunc(item->onInventory, itemData, "inInventar");
        bindFunc(item->onHand,     itemData, "inHand");
        bindFunc(item->onUpdate,   itemData, "onUpdate");

        // ── Auto-Binding fuer ITEM_BEGIN/ITEM_END Callbacks ───────────────────
        // Modder-Dateien registrieren Funktionen als "<id>_onHand" usw.
        // Falls JSON keinen expliziten Eintrag hat, hier automatisch binden.
        if (!item->onHand) {
            auto fn = g_itemManager.findFunc(id + "_onHand");
            if (fn) {
                item->onHand = fn;
                std::cout << "[ItemManager] Auto-Bind: " << id << "_onHand" << std::endl;
            }
        }
        if (!item->onClick) {
            auto fn = g_itemManager.findFunc(id + "_onKlick");
            if (fn) {
                item->onClick = fn;
                std::cout << "[ItemManager] Auto-Bind: " << id << "_onKlick" << std::endl;
            }
        }
        if (!item->onInventory) {
            auto fn = g_itemManager.findFunc(id + "_onInventory");
            if (fn) {
                item->onInventory = fn;
                std::cout << "[ItemManager] Auto-Bind: " << id << "_onInventory" << std::endl;
            }
        }

        items[id] = std::move(item);
        std::cout << "[ItemManager] Item geladen: " << id << std::endl;
    }
}

void ItemManager::scanAndLoadItems() {
    namespace fs = std::filesystem;

    const std::string neuPfad = assetPath("json/items/item.json");
    if (fs::exists(neuPfad)) {
        std::cout << "[ItemManager] Lade: " << neuPfad << std::endl;
        loadItemsFromJson(neuPfad, items);
    }

    const std::string altPfad = assetPath("json/item.json");
    if (fs::exists(altPfad)) {
        std::cout << "[ItemManager] Lade (Fallback): " << altPfad << std::endl;
        loadItemsFromJson(altPfad, items);
    }

    const std::string ordner = assetPath("json/items/");
    if (fs::exists(ordner)) {
        for (const auto& entry : fs::recursive_directory_iterator(ordner)) {
            if (entry.path().extension() == ".json") {
                std::string p = entry.path().string();
                if (p == neuPfad) continue;
                std::cout << "[ItemManager] Zusatz-JSON: " << p << std::endl;
                loadItemsFromJson(p, items);
            }
        }
    }

    std::cout << "[ItemManager] Gesamt: " << items.size() << " Items geladen." << std::endl;
}

void ItemManager::loadItemFromFile(const std::string& filePath) {
    std::cout << "[ItemManager] Neue Item-Datei: " << filePath << std::endl;
    loadItemsFromJson(filePath, items);
}