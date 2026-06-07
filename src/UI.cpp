#include "UI.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
extern std::string assetPath(const std::string& relativ);

// ── Layout constants ──────────────────────────────────────────────────────────
static const int PAD        = 15;
static const int ELEM_H     = 30;
static const int TEXT_H     = 22;
static const int SEP_H      = 12;
static const int TITLE_H    = 34;
static const int SPACING    = 8;
static const int FONT_NRM   = 16;
static const int FONT_SML   = 14;

// ── Color palette ─────────────────────────────────────────────────────────────
static const Color COL_BTN_NORMAL  = {60,  60,  60,  255};
static const Color COL_BTN_HOVER   = {90,  90,  90,  255};
static const Color COL_BTN_PRESS   = {30,  100, 180, 255};
static const Color COL_INPUT_BG    = {50,  50,  50,  255};
static const Color COL_INPUT_FOCUS = {65,  65,  65,  255};
static const Color COL_ACCENT      = {40,  120, 200, 255};
static const Color COL_TRACK       = {75,  75,  75,  255};
static const Color COL_BORDER      = {100, 100, 100, 255};
static const Color COL_LOG         = {255, 200,  50, 255};

// ── Draw helpers ──────────────────────────────────────────────────────────────

static bool ptInRect(Vector2 p, Rectangle r) {
    return p.x >= r.x && p.x <= r.x + r.width &&
           p.y >= r.y && p.y <= r.y + r.height;
}

static void drawCentered(const char* text, Rectangle r, int fs, Color col) {
    int tw = MeasureText(text, fs);
    DrawText(text,
             (int)(r.x + (r.width  - tw) * 0.5f),
             (int)(r.y + (r.height - fs) * 0.5f),
             fs, col);
}

// ── Layout ────────────────────────────────────────────────────────────────────

std::vector<Rectangle> UIManager::computeRects(UIWindow* win, int sw, int sh) const {
    std::vector<Rectangle> rects;
    int winX    = (sw - win->width)  / 2;
    int winY    = (sh - win->height) / 2;
    int contentW = win->width - 2 * PAD;
    int curY    = winY + PAD + (!win->title.empty() ? TITLE_H : 0);

    for (auto& el : win->elements) {
        int h = ELEM_H;
        switch (el->type) {
            case UIElementType::Text:
            case UIElementType::LiveText:
            case UIElementType::Checkbox:  h = TEXT_H; break;
            case UIElementType::Separator: h = SEP_H;  break;
            case UIElementType::Title:     h = 28;     break;
            default:                       h = ELEM_H; break;
        }
        rects.push_back({(float)(winX + PAD), (float)curY, (float)contentW, (float)h});
        curY += h + SPACING;
    }
    return rects;
}

// ── Load ──────────────────────────────────────────────────────────────────────

void UIManager::load(const std::string& jsonPath) {
    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        std::cerr << "[UIManager] Kann nicht öffnen: " << jsonPath << std::endl;
        return;
    }
    json data;
    try { f >> data; }
    catch (const std::exception& e) {
        std::cerr << "[UIManager] JSON Fehler: " << e.what() << std::endl;
        return;
    }

    for (auto& [id, wData] : data.items()) {
        auto win        = std::make_unique<UIWindow>();
        win->id         = id;
        win->title      = wData.value("title",      "");
        win->width      = wData.value("width",       300);
        win->height     = wData.value("height",      200);
        win->modal      = wData.value("modal",       true);
        win->closeOnEsc = wData.value("closeOnEsc",  true);

        if (wData.contains("backgroundColor") && wData["backgroundColor"].is_array()
                && wData["backgroundColor"].size() >= 3) {
            auto& bg = wData["backgroundColor"];
            win->backgroundColor.r = (unsigned char)bg[0].get<int>();
            win->backgroundColor.g = (unsigned char)bg[1].get<int>();
            win->backgroundColor.b = (unsigned char)bg[2].get<int>();
            win->backgroundColor.a = bg.size() >= 4
                                     ? (unsigned char)bg[3].get<int>() : 220;
        }

        auto it = builderRegistry.find(id);
        if (it != builderRegistry.end()) {
            _buildTarget = win.get();
            it->second();
            _buildTarget = nullptr;
        } else {
            std::cerr << "[UIManager] Kein Builder für: " << id << std::endl;
        }

        windows[id] = std::move(win);
        std::cout << "[UIManager] Popup geladen: " << id << std::endl;
    }
}

// ── Registration ──────────────────────────────────────────────────────────────

void UIManager::registerBuilder(const std::string& id, std::function<void()> builder) {
    builderRegistry[id] = builder;
    auto it = windows.find(id);
    if (it != windows.end() && it->second->elements.empty()) {
        _buildTarget = it->second.get();
        builder();
        _buildTarget = nullptr;
    }
}

void UIManager::addElement(std::unique_ptr<UIElement> el) {
    if (_buildTarget)
        _buildTarget->elements.push_back(std::move(el));
}

// ── Open / Close ──────────────────────────────────────────────────────────────

void UIManager::openUI(const std::string& id) {
    auto it = windows.find(id);
    if (it == windows.end()) {
        std::cerr << "[UIManager] Unbekanntes Popup: " << id << std::endl;
        return;
    }
    activeId              = id;
    _focusedElementId.clear();
    _draggingElementId.clear();
    _logText.clear();
}

void UIManager::closeCurrentUI() {
    activeId.clear();
    _focusedElementId.clear();
    _draggingElementId.clear();
    _logText.clear();
}

UIWindow* UIManager::getActiveWindow() {
    if (activeId.empty()) return nullptr;
    auto it = windows.find(activeId);
    return (it != windows.end()) ? it->second.get() : nullptr;
}

bool UIManager::isModal() const {
    if (activeId.empty()) return false;
    auto it = windows.find(activeId);
    return (it != windows.end()) ? it->second->modal : false;
}

// ── Element access ────────────────────────────────────────────────────────────

UIElement* UIManager::findElement(const std::string& elementId) {
    UIWindow* win = getActiveWindow();
    if (!win) return nullptr;
    for (auto& el : win->elements)
        if (el->id == elementId) return el.get();
    return nullptr;
}

void UIManager::updateLiveText(const std::string& elementId, const std::string& text) {
    UIElement* el = findElement(elementId);
    if (el) el->liveText = text;
}

std::string UIManager::getInputValue(const std::string& id) {
    UIElement* el = findElement(id);
    return el ? el->textValue : "";
}
int UIManager::getNumberValue(const std::string& id) {
    UIElement* el = findElement(id);
    return el ? el->intValue : 0;
}
int UIManager::getSliderValue(const std::string& id) {
    UIElement* el = findElement(id);
    return el ? el->intValue : 0;
}
bool UIManager::getCheckboxValue(const std::string& id) {
    UIElement* el = findElement(id);
    return el ? el->boolValue : false;
}
void UIManager::addTextLine(const std::string& text) {
    _logText = text;
}

// ── Update ────────────────────────────────────────────────────────────────────

void UIManager::update() {
    UIWindow* win = getActiveWindow();
    if (!win) return;

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    if (win->closeOnEsc && IsKeyPressed(KEY_ESCAPE)) {
        closeCurrentUI();
        return;
    }

    Vector2 mouse   = GetMousePosition();
    bool clicked    = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool released   = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    // Keyboard input for focused text field
    if (!_focusedElementId.empty()) {
        UIElement* focused = findElement(_focusedElementId);
        if (focused && focused->type == UIElementType::TextInput) {
            bool changed = false;
            int ch = GetCharPressed();
            while (ch > 0) {
                if (ch >= 32 && ch < 127 && (int)focused->textValue.size() < focused->maxLen) {
                    focused->textValue += (char)ch;
                    changed = true;
                }
                ch = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !focused->textValue.empty()) {
                focused->textValue.pop_back();
                changed = true;
            }
            if (changed && focused->onTextChangeCb)
                focused->onTextChangeCb(focused->textValue);
        }
    }

    // Continue dragging slider
    if (!_draggingElementId.empty()) {
        UIElement* el = findElement(_draggingElementId);
        if (el && el->type == UIElementType::Slider) {
            auto rects = computeRects(win, sw, sh);
            for (size_t i = 0; i < win->elements.size(); ++i) {
                if (win->elements[i].get() == el) {
                    auto& r = rects[i];
                    float t = (mouse.x - r.x) / r.width;
                    t = std::max(0.0f, std::min(1.0f, t));
                    int newVal = el->intMin + (int)(t * (el->intMax - el->intMin) + 0.5f);
                    if (newVal != el->intValue) {
                        el->intValue = newVal;
                        if (el->onIntChangeCb) el->onIntChangeCb(el->intValue);
                    }
                    break;
                }
            }
        }
        if (released) _draggingElementId.clear();
    }

    auto rects = computeRects(win, sw, sh);

    for (size_t i = 0; i < win->elements.size() && i < rects.size(); ++i) {
        UIElement* el = win->elements[i].get();
        auto& r       = rects[i];
        el->hovered   = ptInRect(mouse, r);

        switch (el->type) {
            case UIElementType::Button: {
                if (el->hovered && clicked && el->onClickCb) {
                    el->onClickCb();
                    return;
                }
                break;
            }
            case UIElementType::TextInput: {
                if (clicked) {
                    if (el->hovered) {
                        _focusedElementId = el->id;
                        el->focused = true;
                    } else if (el->id == _focusedElementId) {
                        _focusedElementId.clear();
                        el->focused = false;
                    }
                }
                break;
            }
            case UIElementType::Slider: {
                if (el->hovered && clicked) {
                    _draggingElementId = el->id;
                    float t = (mouse.x - r.x) / r.width;
                    t = std::max(0.0f, std::min(1.0f, t));
                    int newVal = el->intMin + (int)(t * (el->intMax - el->intMin) + 0.5f);
                    if (newVal != el->intValue) {
                        el->intValue = newVal;
                        if (el->onIntChangeCb) el->onIntChangeCb(el->intValue);
                    }
                }
                break;
            }
            case UIElementType::NumberInput: {
                if (el->hovered && clicked) {
                    float third = r.width / 3.0f;
                    if (mouse.x < r.x + third && el->intValue > el->intMin) {
                        el->intValue--;
                        if (el->onIntChangeCb) el->onIntChangeCb(el->intValue);
                    } else if (mouse.x > r.x + 2.0f * third && el->intValue < el->intMax) {
                        el->intValue++;
                        if (el->onIntChangeCb) el->onIntChangeCb(el->intValue);
                    }
                }
                break;
            }
            case UIElementType::Checkbox: {
                if (el->hovered && clicked) {
                    el->boolValue = !el->boolValue;
                    if (el->onBoolChangeCb) el->onBoolChangeCb(el->boolValue);
                }
                break;
            }
            case UIElementType::Dropdown: {
                if (clicked) {
                    if (el->hovered) {
                        el->dropOpen = !el->dropOpen;
                    } else if (el->dropOpen) {
                        bool picked = false;
                        for (int j = 0; j < (int)el->dropOptions.size(); ++j) {
                            Rectangle optR = {r.x, r.y + r.height + j * (float)ELEM_H,
                                              r.width, (float)ELEM_H};
                            if (ptInRect(mouse, optR)) {
                                el->selectedIndex = j;
                                el->dropOpen = false;
                                if (el->onIntChangeCb) el->onIntChangeCb(el->selectedIndex);
                                picked = true;
                                break;
                            }
                        }
                        if (!picked) el->dropOpen = false;
                    }
                }
                break;
            }
            default: break;
        }
    }
}

// ── Draw ──────────────────────────────────────────────────────────────────────

void UIManager::draw() {
    UIWindow* win = getActiveWindow();
    if (!win) return;

    int sw   = GetScreenWidth();
    int sh   = GetScreenHeight();
    int winX = (sw - win->width)  / 2;
    int winY = (sh - win->height) / 2;

    if (win->modal)
        DrawRectangle(0, 0, sw, sh, {0, 0, 0, 110});

    DrawRectangle(winX, winY, win->width, win->height, win->backgroundColor);
    DrawRectangleLines(winX, winY, win->width, win->height, COL_BORDER);

    // Title
    if (!win->title.empty()) {
        int tw = MeasureText(win->title.c_str(), FONT_NRM);
        DrawText(win->title.c_str(),
                 winX + (win->width - tw) / 2,
                 winY + PAD + (TITLE_H - FONT_NRM) / 2 - 6,
                 FONT_NRM, WHITE);
        DrawLine(winX + PAD, winY + TITLE_H,
                 winX + win->width - PAD, winY + TITLE_H, COL_BORDER);
    }

    Vector2 mouse  = GetMousePosition();
    bool    lmbDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    auto    rects   = computeRects(win, sw, sh);

    for (size_t i = 0; i < win->elements.size() && i < rects.size(); ++i) {
        UIElement* el = win->elements[i].get();
        auto& r       = rects[i];

        switch (el->type) {

            case UIElementType::Text: {
                DrawText(el->label.c_str(), (int)r.x, (int)r.y + 3, FONT_SML, WHITE);
                break;
            }
            case UIElementType::LiveText: {
                const std::string& txt = el->liveText.empty() ? el->label : el->liveText;
                DrawText(txt.c_str(), (int)r.x, (int)r.y + 3, FONT_SML, WHITE);
                break;
            }
            case UIElementType::Title: {
                int tw = MeasureText(el->label.c_str(), FONT_NRM);
                DrawText(el->label.c_str(),
                         (int)(r.x + (r.width - tw) / 2), (int)r.y,
                         FONT_NRM, WHITE);
                break;
            }
            case UIElementType::Separator: {
                int lineY = (int)(r.y + r.height / 2);
                DrawLine((int)r.x, lineY, (int)(r.x + r.width), lineY, COL_BORDER);
                break;
            }
            case UIElementType::Button: {
                Color bg = el->hovered
                    ? (lmbDown ? COL_BTN_PRESS : COL_BTN_HOVER)
                    : COL_BTN_NORMAL;
                DrawRectangleRec(r, bg);
                DrawRectangleLinesEx(r, 1, COL_BORDER);
                drawCentered(el->label.c_str(), r, FONT_NRM, WHITE);
                break;
            }
            case UIElementType::TextInput: {
                bool isFocused = (el->id == _focusedElementId);
                DrawRectangleRec(r, isFocused ? COL_INPUT_FOCUS : COL_INPUT_BG);
                DrawRectangleLinesEx(r, 1, isFocused ? COL_ACCENT : COL_BORDER);
                std::string disp = el->textValue;
                if (isFocused && ((int)(GetTime() * 2) % 2 == 0)) disp += "|";
                DrawText(disp.c_str(),
                         (int)r.x + 6,
                         (int)(r.y + (r.height - FONT_NRM) * 0.5f),
                         FONT_NRM, WHITE);
                break;
            }
            case UIElementType::NumberInput: {
                DrawRectangleRec(r, COL_INPUT_BG);
                DrawRectangleLinesEx(r, 1, COL_BORDER);
                float third = r.width / 3.0f;
                Rectangle minR = {r.x,               r.y, third, r.height};
                Rectangle maxR = {r.x + 2.0f * third, r.y, third, r.height};
                Rectangle valR = {r.x + third,        r.y, third, r.height};
                bool minHov = ptInRect(mouse, minR);
                bool maxHov = ptInRect(mouse, maxR);
                DrawRectangleRec(minR, minHov ? COL_BTN_HOVER : COL_BTN_NORMAL);
                DrawRectangleRec(maxR, maxHov ? COL_BTN_HOVER : COL_BTN_NORMAL);
                drawCentered("-", minR, FONT_NRM + 4, WHITE);
                drawCentered("+", maxR, FONT_NRM + 4, WHITE);
                std::string valStr = std::to_string(el->intValue);
                drawCentered(valStr.c_str(), valR, FONT_NRM, WHITE);
                break;
            }
            case UIElementType::Slider: {
                int   trkH  = 8;
                float fillT = (float)(el->intValue - el->intMin)
                            / (float)std::max(1, el->intMax - el->intMin);
                Rectangle trkR  = {r.x, r.y + (r.height - trkH) * 0.5f, r.width, (float)trkH};
                Rectangle fillR = {trkR.x, trkR.y, trkR.width * fillT, trkR.height};
                DrawRectangleRec(trkR,  COL_TRACK);
                DrawRectangleRec(fillR, COL_ACCENT);
                int hx = (int)(r.x + r.width * fillT);
                int hy = (int)(r.y + r.height * 0.5f);
                DrawCircle(hx, hy, 8, COL_ACCENT);
                DrawCircle(hx, hy, 5, el->hovered ? WHITE : LIGHTGRAY);
                break;
            }
            case UIElementType::Checkbox: {
                int boxSz = 18;
                Rectangle boxR = {r.x, r.y + (r.height - boxSz) * 0.5f,
                                  (float)boxSz, (float)boxSz};
                DrawRectangleRec(boxR, el->hovered ? COL_BTN_HOVER : COL_INPUT_BG);
                DrawRectangleLinesEx(boxR, 1, el->boolValue ? COL_ACCENT : COL_BORDER);
                if (el->boolValue)
                    DrawText("x", (int)boxR.x + 3, (int)boxR.y + 1, FONT_NRM, COL_ACCENT);
                DrawText(el->label.c_str(),
                         (int)(r.x + boxSz + 8),
                         (int)(r.y + (r.height - FONT_SML) * 0.5f),
                         FONT_SML, WHITE);
                break;
            }
            case UIElementType::Dropdown: {
                DrawRectangleRec(r, el->hovered ? COL_BTN_HOVER : COL_BTN_NORMAL);
                DrawRectangleLinesEx(r, 1, COL_BORDER);
                std::string selTxt = el->dropOptions.empty()
                    ? "---" : el->dropOptions[el->selectedIndex];
                DrawText(selTxt.c_str(),
                         (int)r.x + 8,
                         (int)(r.y + (r.height - FONT_NRM) * 0.5f),
                         FONT_NRM, WHITE);
                DrawText(el->dropOpen ? "^" : "v",
                         (int)(r.x + r.width - 20),
                         (int)(r.y + (r.height - FONT_NRM) * 0.5f),
                         FONT_NRM, WHITE);
                if (el->dropOpen) {
                    for (int j = 0; j < (int)el->dropOptions.size(); ++j) {
                        Rectangle optR = {r.x, r.y + r.height + j * (float)ELEM_H,
                                          r.width, (float)ELEM_H};
                        bool hov = ptInRect(mouse, optR);
                        DrawRectangleRec(optR, hov ? COL_BTN_HOVER : COL_BTN_NORMAL);
                        DrawRectangleLinesEx(optR, 1, COL_BORDER);
                        DrawText(el->dropOptions[j].c_str(),
                                 (int)optR.x + 8,
                                 (int)(optR.y + (ELEM_H - FONT_NRM) * 0.5f),
                                 FONT_NRM, WHITE);
                    }
                }
                break;
            }
        }
    }

    // Log text (letzte Meldung von addUITextLine)
    if (!_logText.empty()) {
        int tw = MeasureText(_logText.c_str(), FONT_SML);
        DrawText(_logText.c_str(),
                 winX + (win->width - tw) / 2,
                 winY + win->height - PAD - FONT_SML,
                 FONT_SML, COL_LOG);
    }
}
