#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  ui_api.h  –  Das einzige Header das ein UI-Modder braucht
//
//  Verwendung in ui/house_entrance.cpp:
//
//      #include "ui_api.h"
//      UI_WINDOW_BEGIN("house_entrance")
//
//          UI_TEXT("Willst du das Gebäude betreten?")
//          UI_BUTTON("Ja",   []() { switchDimension("house_interior"); closeCurrentUI(); })
//          UI_BUTTON("Nein", []() { closeCurrentUI(); })
//
//      UI_WINDOW_END("house_entrance")
//
//  Die ID muss exakt mit dem Key in assets/json/ui/popups.json übereinstimmen.
//  Das UI wird beim Programmstart automatisch registriert.
// ═══════════════════════════════════════════════════════════════════════════════

#include "UI.h"
#include "item api.h"
#include <string>
#include <functional>
#include <memory>
#include <vector>

// ── Fenstersteuerung ──────────────────────────────────────────────────────────

inline void openUI(const std::string& id)  { g_uiManager.openUI(id); }
inline void closeCurrentUI()               { g_uiManager.closeCurrentUI(); }
inline bool isUIOpen()                     { return g_uiManager.isUIOpen(); }
inline std::string getCurrentUIId()        { return g_uiManager.getCurrentUIId(); }

// ── Dynamische Updates ────────────────────────────────────────────────────────

inline void updateUIText(const std::string& elementId, const std::string& newText) {
    g_uiManager.updateLiveText(elementId, newText);
}
inline std::string getUIInputValue(const std::string& id)  { return g_uiManager.getInputValue(id);   }
inline int         getUINumberValue(const std::string& id) { return g_uiManager.getNumberValue(id);  }
inline int         getUISliderValue(const std::string& id) { return g_uiManager.getSliderValue(id);  }
inline bool        getUICheckboxValue(const std::string& id){ return g_uiManager.getCheckboxValue(id);}
inline void        addUITextLine(const std::string& text)  { g_uiManager.addTextLine(text);           }

// ── Interne Hilfsfunktion (von Makros genutzt) ────────────────────────────────

inline void _ui_addElement(std::unique_ptr<UIElement> el) {
    g_uiManager.addElement(std::move(el));
}

// ── Element-Makros ────────────────────────────────────────────────────────────

#define UI_TEXT(text_) \
    { auto _el = std::make_unique<UIElement>(); \
      _el->type  = UIElementType::Text; \
      _el->label = (text_); \
      _ui_addElement(std::move(_el)); }

#define UI_TITLE(text_) \
    { auto _el = std::make_unique<UIElement>(); \
      _el->type  = UIElementType::Title; \
      _el->label = (text_); \
      _ui_addElement(std::move(_el)); }

#define UI_TEXT_LIVE(id_, initial_) \
    { auto _el = std::make_unique<UIElement>(); \
      _el->type     = UIElementType::LiveText; \
      _el->id       = (id_); \
      _el->label    = (initial_); \
      _el->liveText = (initial_); \
      _ui_addElement(std::move(_el)); }

#define UI_SEPARATOR() \
    { auto _el = std::make_unique<UIElement>(); \
      _el->type = UIElementType::Separator; \
      _ui_addElement(std::move(_el)); }

#define UI_BUTTON(label_, callback_) \
    { auto _el = std::make_unique<UIElement>(); \
      _el->type      = UIElementType::Button; \
      _el->label     = (label_); \
      _el->onClickCb = (callback_); \
      _ui_addElement(std::move(_el)); }

#define UI_TEXT_INPUT(id_, max_len_, callback_) \
    { auto _el = std::make_unique<UIElement>(); \
      _el->type           = UIElementType::TextInput; \
      _el->id             = (id_); \
      _el->maxLen         = (max_len_); \
      _el->onTextChangeCb = (callback_); \
      _ui_addElement(std::move(_el)); }

#define UI_NUMBER_INPUT(id_, min_, max_, default_, callback_) \
    { auto _el = std::make_unique<UIElement>(); \
      _el->type          = UIElementType::NumberInput; \
      _el->id            = (id_); \
      _el->intMin        = (min_); \
      _el->intMax        = (max_); \
      _el->intValue      = (default_); \
      _el->onIntChangeCb = (callback_); \
      _ui_addElement(std::move(_el)); }

#define UI_SLIDER(id_, min_, max_, default_, callback_) \
    { auto _el = std::make_unique<UIElement>(); \
      _el->type          = UIElementType::Slider; \
      _el->id            = (id_); \
      _el->intMin        = (min_); \
      _el->intMax        = (max_); \
      _el->intValue      = (default_); \
      _el->onIntChangeCb = (callback_); \
      _ui_addElement(std::move(_el)); }

#define UI_CHECKBOX(label_, default_, callback_) \
    { auto _el = std::make_unique<UIElement>(); \
      _el->type           = UIElementType::Checkbox; \
      _el->label          = (label_); \
      _el->boolValue      = (default_); \
      _el->onBoolChangeCb = (callback_); \
      _ui_addElement(std::move(_el)); }

#define UI_DROPDOWN(id_, options_, default_idx_, callback_) \
    { auto _el = std::make_unique<UIElement>(); \
      _el->type          = UIElementType::Dropdown; \
      _el->id            = (id_); \
      _el->dropOptions   = (options_); \
      _el->selectedIndex = (default_idx_); \
      _el->onIntChangeCb = (callback_); \
      _ui_addElement(std::move(_el)); }

// ── Auto-Registrar ────────────────────────────────────────────────────────────

struct _UIAutoRegistrar {
    _UIAutoRegistrar(const char* id, std::function<void()> builder) {
        g_uiManager.registerBuilder(std::string(id), builder);
    }
};

// ── UI_WINDOW_BEGIN / UI_WINDOW_END ───────────────────────────────────────────
//
//  Verwendet einen anonymen Namespace damit mehrere .cpp-Dateien
//  keine Namenskonflikte erzeugen (gleiche Regel wie bei Items/Buildings).

#define UI_WINDOW_BEGIN(uiId) \
    namespace { \
        static void _ui_builder_fn(); \
        static _UIAutoRegistrar _ui_reg(uiId, _ui_builder_fn); \
        static void _ui_builder_fn() {

#define UI_WINDOW_END(uiId) \
        } \
    }
