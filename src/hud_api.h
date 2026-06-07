#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  hud_api.h  –  Das einzige Header das ein HUD-Modder braucht
//
//  Verwendung in hud/my_hud.cpp:
//
//      #include "hud_api.h"
//
//      HUD_BUTTON_BEGIN(attack_btn)
//          HUD_POS(10, 10)
//          HUD_SIZE(80, 40)
//          HUD_TEXT("Angriff")
//          HUD_ON_PRESS([]{ /* einmal beim Drücken */ })
//          HUD_ON_HOLD([]{ /* jeden Frame solange gehalten */ })
//      HUD_BUTTON_END
//
//      HUD_IMAGE_BEGIN(portrait)
//          HUD_POS(10, 60)
//          HUD_SIZE(64, 64)
//          HUD_TEXTURE("assets/portrait.png")
//      HUD_IMAGE_END
//
//      HUD_LABEL_BEGIN(hp_text)
//          HUD_POS(80, 60)
//          HUD_VALUE([]{ return "HP: " + std::to_string(g_player->getHp()); })
//      HUD_LABEL_END
//
//  Kein JSON, kein openUI(), alles direkt im cpp.
//  Die Datei wird automatisch gefunden und registriert.
// ═══════════════════════════════════════════════════════════════════════════════

#include "HUD.h"
#include "item api.h"
#include "particle_api.h"
#include "sound_api.h"
#include <string>
#include <functional>

// ── Positions- und Größen-Makros ──────────────────────────────────────────────
#define HUD_POS(x_, y_)    g_hudManager.setPos((float)(x_), (float)(y_));
#define HUD_SIZE(w_, h_)   g_hudManager.setSize((float)(w_), (float)(h_));
#define HUD_TEXT(t_)       g_hudManager.setText(t_);
#define HUD_TEXTURE(p_)    g_hudManager.setTexturePath(p_);
#define HUD_ON_PRESS(fn_)  g_hudManager.setOnPress(fn_);
#define HUD_ON_HOLD(fn_)   g_hudManager.setOnHold(fn_);
#define HUD_VALUE(fn_)     g_hudManager.setGetValue(fn_);

// ── Auto-Registrar ────────────────────────────────────────────────────────────
struct _HudAutoReg {
    _HudAutoReg(HudElementType t, std::function<void()> builder) {
        g_hudManager.beginElement(t);
        builder();
        g_hudManager.endElement();
    }
};

// ── HUD_BUTTON_BEGIN / HUD_BUTTON_END ─────────────────────────────────────────
#define HUD_BUTTON_BEGIN(tag)                                      \
    namespace _hud_btn_##tag {                                     \
        static void _build();                                      \
        static _HudAutoReg _reg(HudElementType::Button, _build);   \
        static void _build() {

#define HUD_BUTTON_END \
        }              \
    }

// ── HUD_IMAGE_BEGIN / HUD_IMAGE_END ──────────────────────────────────────────
#define HUD_IMAGE_BEGIN(tag)                                       \
    namespace _hud_img_##tag {                                     \
        static void _build();                                      \
        static _HudAutoReg _reg(HudElementType::Image, _build);    \
        static void _build() {

#define HUD_IMAGE_END \
        }             \
    }

// ── HUD_LABEL_BEGIN / HUD_LABEL_END ──────────────────────────────────────────
#define HUD_LABEL_BEGIN(tag)                                       \
    namespace _hud_lbl_##tag {                                     \
        static void _build();                                      \
        static _HudAutoReg _reg(HudElementType::Label, _build);    \
        static void _build() {

#define HUD_LABEL_END \
        }             \
    }
