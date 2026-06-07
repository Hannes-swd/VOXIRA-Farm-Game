#include "Object.h"
#include "player.h"
#include "item api.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern player*      g_player;
extern std::string  assetPath(const std::string&);

PlacedObject* g_activePlacedObject = nullptr;

static std::string normalizePath(const std::string& raw) {
    for (const std::string& sep : {"assets/", "assets\\"}) {
        size_t pos = raw.rfind(sep);
        if (pos != std::string::npos) {
            std::string rest = raw.substr(pos + sep.size());
            for (char& c : rest) if (c == '\\') c = '/';
            return rest;
        }
    }
    std::string r = raw;
    for (char& c : r) if (c == '\\') c = '/';
    return r;
}

ObjectManager::~ObjectManager() {
    for (auto& [id, t] : templates) t.unloadTexture();
}

void ObjectManager::registerTemplate(const std::string& id, ObjectTemplate tmpl) {
    templates[id] = std::move(tmpl);
}

void ObjectManager::registerCallbacks(const std::string& id,
                                      std::function<void(PlacedObject&)> fnInteract,
                                      std::function<void(PlacedObject&)> fnUpdate,
                                      std::function<void(PlacedObject&)> fnDestroy)
{
    callbackRegistry[id] = {fnInteract, fnUpdate, fnDestroy};
    auto it = templates.find(id);
    if (it != templates.end()) {
        it->second.onInteract = fnInteract;
        it->second.onUpdate   = fnUpdate;
        it->second.onDestroy  = fnDestroy;
    }
}

void ObjectManager::loadTemplates(const std::string& jsonPath) {
    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        std::cerr << "[ObjectManager] Nicht gefunden: " << jsonPath << std::endl;
        return;
    }
    json data;
    try { f >> data; } catch (const std::exception& e) {
        std::cerr << "[ObjectManager] JSON Fehler: " << e.what() << std::endl;
        return;
    }
    for (auto& [id, d] : data.items()) {
        ObjectTemplate t;
        t.id             = id;
        t.name           = d.value("name",             id);
        t.width          = d.value("width",             1);
        t.height         = d.value("height",            1);
        t.solid          = d.value("solid",             false);
        t.interactable   = d.value("interactable",      true);
        t.interactRadius = d.value("interact_radius",   40.0f);
        t.layer          = d.value("layer", std::string("below_player"));

        std::string rawTex = d.value("texture", "");
        if (!rawTex.empty()) {
            t.texturPath = assetPath(normalizePath(rawTex));
            t.loadTexture();
        }

        // Callbacks die vor dem JSON-Laden registriert wurden einbinden
        auto cbIt = callbackRegistry.find(id);
        if (cbIt != callbackRegistry.end()) {
            t.onInteract = cbIt->second.onInteract;
            t.onUpdate   = cbIt->second.onUpdate;
            t.onDestroy  = cbIt->second.onDestroy;
        }

        std::cout << "[ObjectManager] Template: " << id << std::endl;
        templates[id] = std::move(t);
    }
}

PlacedObject* ObjectManager::place(const std::string& objectId, int tileX, int tileY) {
    PlacedObject obj;
    obj.objectId   = objectId;
    obj.instanceId = objectId + "_" + std::to_string(tileX)
                   + "_" + std::to_string(tileY)
                   + "_" + std::to_string(nextId++);
    obj.tileX = tileX;
    obj.tileY = tileY;
    placed.push_back(obj);
    return &placed.back();
}

void ObjectManager::remove(const std::string& instanceId) {
    for (auto& obj : placed) {
        if (obj.instanceId == instanceId) {
            auto it = templates.find(obj.objectId);
            if (it != templates.end() && it->second.onDestroy)
                it->second.onDestroy(obj);
            obj.pendingRemoval = true;
            return;
        }
    }
}

ObjectTemplate* ObjectManager::getTemplate(const std::string& id) {
    auto it = templates.find(id);
    return it != templates.end() ? &it->second : nullptr;
}

void ObjectManager::update(float /*dt*/, int tileSize) {
    placed.erase(
        std::remove_if(placed.begin(), placed.end(),
            [](const PlacedObject& o){ return o.pendingRemoval; }),
        placed.end());

    bool ePressed = IsKeyPressed(KEY_E);
    float nearDist = 1e9f;
    PlacedObject* nearObj = nullptr;

    for (auto& obj : placed) {
        auto it = templates.find(obj.objectId);
        if (it == templates.end()) continue;
        ObjectTemplate& tmpl = it->second;

        if (tmpl.onUpdate) {
            g_activePlacedObject = &obj;
            tmpl.onUpdate(obj);
            g_activePlacedObject = nullptr;
        }

        if (!tmpl.interactable || !g_player) continue;
        float cx   = (obj.tileX + tmpl.width  * 0.5f) * (float)tileSize;
        float cy   = (obj.tileY + tmpl.height * 0.5f) * (float)tileSize;
        Vector2 pp = g_player->Get_position();
        float dx   = pp.x - cx;
        float dy   = pp.y - cy;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist < tmpl.interactRadius && dist < nearDist) {
            nearDist = dist;
            nearObj  = &obj;
        }
    }

    if (nearObj && ePressed) {
        auto it = templates.find(nearObj->objectId);
        if (it != templates.end() && it->second.onInteract) {
            g_activePlacedObject = nearObj;
            it->second.onInteract(*nearObj);
            g_activePlacedObject = nullptr;
        }
    }
}

void ObjectManager::draw(int tileSize, bool abovePlayer) {
    for (auto& obj : placed) {
        if (obj.pendingRemoval) continue;
        auto it = templates.find(obj.objectId);
        if (it == templates.end()) continue;
        ObjectTemplate& tmpl = it->second;

        bool above = (tmpl.layer == "above_player");
        if (above != abovePlayer) continue;

        int px = obj.tileX * tileSize;
        int py = obj.tileY * tileSize;
        int pw = tmpl.width  * tileSize;
        int ph = tmpl.height * tileSize;

        if (tmpl.textur.id != 0) {
            Rectangle src = {0, 0, (float)tmpl.textur.width, (float)tmpl.textur.height};
            Rectangle dst = {(float)px, (float)py, (float)pw, (float)ph};
            DrawTexturePro(tmpl.textur, src, dst, {0,0}, 0.0f, WHITE);
        } else {
            DrawRectangle(px, py, pw, ph, BROWN);
            DrawRectangleLines(px, py, pw, ph, DARKBROWN);
        }
    }
}

void ObjectManager::save(const std::string& path) const {
    json arr = json::array();
    for (auto& obj : placed) {
        if (obj.pendingRemoval) continue;
        arr.push_back({
            {"objectId",   obj.objectId},
            {"instanceId", obj.instanceId},
            {"tileX",      obj.tileX},
            {"tileY",      obj.tileY}
        });
    }
    std::ofstream f(path);
    if (f.is_open()) f << arr.dump(2);
    else std::cerr << "[ObjectManager] Speichern fehlgeschlagen: " << path << std::endl;
}

void ObjectManager::load(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return;
    json data;
    try { f >> data; } catch (...) { return; }
    if (!data.is_array()) return;
    placed.clear();
    for (auto& j : data) {
        PlacedObject obj;
        obj.objectId   = j.value("objectId",   "");
        obj.instanceId = j.value("instanceId", "");
        obj.tileX      = j.value("tileX",       0);
        obj.tileY      = j.value("tileY",       0);
        if (obj.objectId.empty()) continue;
        // Nächste ID aus gespeicherter instanceId ermitteln
        size_t last = obj.instanceId.rfind('_');
        if (last != std::string::npos) {
            try {
                int idx = std::stoi(obj.instanceId.substr(last + 1));
                if (idx >= nextId) nextId = idx + 1;
            } catch (...) {}
        }
        placed.push_back(obj);
    }
}
