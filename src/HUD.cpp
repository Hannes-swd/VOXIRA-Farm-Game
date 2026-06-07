#include "HUD.h"

void HudManager::beginElement(HudElementType t) {
    _current = HudElement{};
    _current.type = t;
}

void HudManager::setPos(float x, float y)             { _current.x = x; _current.y = y; }
void HudManager::setSize(float w, float h)            { _current.w = w; _current.h = h; }
void HudManager::setText(const std::string& t)        { _current.text = t; }
void HudManager::setTexturePath(const std::string& p) { _current.texturePath = p; }
void HudManager::setOnPress(std::function<void()> fn) { _current.onPress = fn; }
void HudManager::setOnHold(std::function<void()> fn)  { _current.onHold = fn; }
void HudManager::setGetValue(std::function<std::string()> fn) { _current.getValue = fn; }

void HudManager::endElement() {
    elements.push_back(std::move(_current));
}

void HudManager::update() {
    Vector2 mouse     = GetMousePosition();
    bool    pressed   = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool    held      = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    for (auto& el : elements) {
        if (el.type != HudElementType::Button) continue;
        Rectangle rect = { el.x, el.y, el.w, el.h };
        bool over = CheckCollisionPointRec(mouse, rect);
        if (over && pressed && el.onPress) el.onPress();
        if (over && held    && el.onHold)  el.onHold();
    }
}

void HudManager::draw() {
    for (auto& el : elements) {
        // Lazy-load: Textur erst laden wenn das Fenster offen ist
        if (!el.texturePath.empty() && !el.texLoaded) {
            el.texture   = LoadTexture(el.texturePath.c_str());
            el.texLoaded = true;
        }

        if (el.type == HudElementType::Button) {
            Rectangle rect = { el.x, el.y, el.w, el.h };
            bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
            Color bg = hovered ? Color{80, 80, 80, 220} : Color{50, 50, 50, 200};
            DrawRectangleRec(rect, bg);
            DrawRectangleLinesEx(rect, 1.0f, {200, 200, 200, 255});

            if (el.texLoaded && el.texture.id != 0) {
                Rectangle src = { 0, 0, (float)el.texture.width, (float)el.texture.height };
                DrawTexturePro(el.texture, src, rect, {0, 0}, 0.0f, WHITE);
            } else if (!el.text.empty()) {
                int fs = 14;
                int tw = MeasureText(el.text.c_str(), fs);
                DrawText(el.text.c_str(),
                         (int)(el.x + el.w / 2 - tw / 2),
                         (int)(el.y + el.h / 2 - fs / 2),
                         fs, WHITE);
            }

        } else if (el.type == HudElementType::Image) {
            if (el.texLoaded && el.texture.id != 0) {
                Rectangle src  = { 0, 0, (float)el.texture.width, (float)el.texture.height };
                Rectangle dest = { el.x, el.y, el.w, el.h };
                DrawTexturePro(el.texture, src, dest, {0, 0}, 0.0f, WHITE);
            }

        } else if (el.type == HudElementType::Label) {
            std::string val = el.getValue ? el.getValue() : el.text;
            DrawText(val.c_str(), (int)el.x, (int)el.y, 16, WHITE);
        }
    }
}
