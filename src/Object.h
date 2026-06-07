#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include "raylib.h"

struct PlacedObject {
    std::string objectId;
    std::string instanceId;
    int  tileX          = 0;
    int  tileY          = 0;
    bool pendingRemoval = false;
};

struct ObjectTemplate {
    std::string id;
    std::string name;
    std::string texturPath;
    Texture2D   textur        = {0};
    int         width         = 1;
    int         height        = 1;
    bool        solid         = false;
    bool        interactable  = true;
    float       interactRadius = 40.0f;
    std::string layer         = "below_player"; // "below_player" | "above_player"

    std::function<void(PlacedObject&)> onInteract;
    std::function<void(PlacedObject&)> onUpdate;
    std::function<void(PlacedObject&)> onDestroy;

    void loadTexture()   { if (!texturPath.empty()) textur = LoadTexture(texturPath.c_str()); }
    void unloadTexture() { if (textur.id != 0) { UnloadTexture(textur); textur = {0}; } }
};

struct ObjCallbacks {
    std::function<void(PlacedObject&)> onInteract;
    std::function<void(PlacedObject&)> onUpdate;
    std::function<void(PlacedObject&)> onDestroy;
};

class ObjectManager {
    std::unordered_map<std::string, ObjectTemplate> templates;
    std::unordered_map<std::string, ObjCallbacks>   callbackRegistry;
    std::vector<PlacedObject> placed;
    int nextId = 0;

public:
    ~ObjectManager();

    void registerTemplate(const std::string& id, ObjectTemplate tmpl);
    void registerCallbacks(const std::string& id,
                           std::function<void(PlacedObject&)> fnInteract,
                           std::function<void(PlacedObject&)> fnUpdate,
                           std::function<void(PlacedObject&)> fnDestroy);
    void loadTemplates(const std::string& jsonPath);

    PlacedObject* place(const std::string& objectId, int tileX, int tileY);
    void          remove(const std::string& instanceId);

    void update(float dt, int tileSize);
    void draw(int tileSize, bool abovePlayer = false);

    void save(const std::string& path) const;
    void load(const std::string& path);

    ObjectTemplate* getTemplate(const std::string& id);
    std::vector<PlacedObject>& getAll() { return placed; }
};

inline ObjectManager& getObjectManager() {
    static ObjectManager instance;
    return instance;
}
#define g_objectManager getObjectManager()

extern PlacedObject* g_activePlacedObject;
