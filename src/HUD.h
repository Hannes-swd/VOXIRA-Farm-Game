#pragma once
#include "raylib.h"
#include <string>
#include <functional>
#include <vector>

enum class HudElementType { Button, Image, Label };

struct HudElement {
    HudElementType type = HudElementType::Label;
    float x = 0, y = 0, w = 100, h = 40;

    std::string text;
    std::string texturePath;
    Texture2D   texture   = {0};
    bool        texLoaded = false;

    std::function<void()>        onPress;
    std::function<void()>        onHold;
    std::function<std::string()> getValue;
};

class HudManager {
    std::vector<HudElement> elements;
    HudElement              _current;

public:
    void beginElement(HudElementType t);
    void setPos(float x, float y);
    void setSize(float w, float h);
    void setText(const std::string& t);
    void setTexturePath(const std::string& path);
    void setOnPress(std::function<void()> fn);
    void setOnHold(std::function<void()> fn);
    void setGetValue(std::function<std::string()> fn);
    void endElement();

    void update();
    void draw();
};

inline HudManager& getHudManager() {
    static HudManager instance;
    return instance;
}
#define g_hudManager getHudManager()
