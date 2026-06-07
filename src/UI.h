#pragma once
#include "raylib.h"
#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

enum class UIElementType {
    Text, LiveText, Title, Separator,
    Button, TextInput, NumberInput, Slider, Checkbox, Dropdown
};

struct UIElement {
    UIElementType type  = UIElementType::Text;
    std::string   id;
    std::string   label;
    std::string   liveText;

    std::function<void()>                   onClickCb;
    std::function<void(const std::string&)> onTextChangeCb;
    std::function<void(int)>                onIntChangeCb;
    std::function<void(bool)>               onBoolChangeCb;

    std::string textValue;
    int         intValue   = 0;
    int         intMin     = 0;
    int         intMax     = 100;
    bool        boolValue  = false;
    bool        hovered    = false;
    bool        focused    = false;
    int         maxLen     = 64;

    std::vector<std::string> dropOptions;
    int  selectedIndex = 0;
    bool dropOpen      = false;
};

struct UIWindow {
    std::string id;
    std::string title;
    int   width            = 300;
    int   height           = 200;
    bool  modal            = true;
    bool  closeOnEsc       = true;
    Color backgroundColor  = {30, 30, 30, 220};

    std::vector<std::unique_ptr<UIElement>> elements;
};

class UIManager {
    std::unordered_map<std::string, std::unique_ptr<UIWindow>> windows;
    std::unordered_map<std::string, std::function<void()>>     builderRegistry;
    std::string  activeId;
    UIWindow*    _buildTarget          = nullptr;
    std::string  _focusedElementId;
    std::string  _draggingElementId;
    std::string  _logText;

public:
    void load(const std::string& jsonPath);

    void registerBuilder(const std::string& id, std::function<void()> builder);
    void addElement(std::unique_ptr<UIElement> el);

    void openUI(const std::string& id);
    void closeCurrentUI();
    bool               isUIOpen()      const { return !activeId.empty(); }
    const std::string& getCurrentUIId() const { return activeId; }
    bool               isModal()       const;

    UIElement*  findElement(const std::string& elementId);
    void        updateLiveText(const std::string& elementId, const std::string& text);
    std::string getInputValue(const std::string& id);
    int         getNumberValue(const std::string& id);
    int         getSliderValue(const std::string& id);
    bool        getCheckboxValue(const std::string& id);
    void        addTextLine(const std::string& text);

    void update();
    void draw();

private:
    UIWindow* getActiveWindow();
    std::vector<Rectangle> computeRects(UIWindow* win, int screenW, int screenH) const;
};

inline UIManager& getUIManager() {
    static UIManager instance;
    return instance;
}
#define g_uiManager getUIManager()
