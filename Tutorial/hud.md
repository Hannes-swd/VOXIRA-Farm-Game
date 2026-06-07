# How to Create HUD Elements

> A **HUD element** is a permanent on-screen overlay — always visible, no JSON needed.
> Unlike popups, HUD elements are always drawn and need no `openUI()` call.

---

## Step 1 — Create a file in `hud/`

Create a new `.cpp` file in the [`hud/`](../hud/) folder.  
The engine finds it automatically — no registration needed.

---

## Button

A button has two callbacks:
- `HUD_ON_PRESS` — fires **once** the moment the button is clicked
- `HUD_ON_HOLD`  — fires **every frame** as long as the button is held down

```cpp
#include "hud_api.h"

HUD_BUTTON_BEGIN(attack_button)
    HUD_POS(10, 10)
    HUD_SIZE(80, 40)
    HUD_TEXT("Angriff")

    HUD_ON_PRESS([]{
        // einmal beim Klick
    })

    HUD_ON_HOLD([]{
        // jeden Frame solange gedrückt
    })
HUD_BUTTON_END
```

Button mit Textur statt Text:

```cpp
HUD_BUTTON_BEGIN(sword_button)
    HUD_POS(10, 10)
    HUD_SIZE(64, 64)
    HUD_TEXTURE("assets/textures/sword_icon.png")

    HUD_ON_PRESS([]{
        // ...
    })
HUD_BUTTON_END
```

> Both callbacks are optional — leave out `HUD_ON_PRESS` or `HUD_ON_HOLD` if not needed.

---

## Image

Displays a texture at a fixed screen position.

```cpp
#include "hud_api.h"

HUD_IMAGE_BEGIN(player_portrait)
    HUD_POS(10, 60)
    HUD_SIZE(64, 64)
    HUD_TEXTURE("assets/textures/portrait.png")
HUD_IMAGE_END
```

---

## Label

Displays a variable value. The lambda is called every frame.

```cpp
#include "hud_api.h"

HUD_LABEL_BEGIN(hp_display)
    HUD_POS(80, 60)
    HUD_VALUE([]{ return "HP: " + std::to_string(g_player->getHp()); })
HUD_LABEL_END
```

Any `std::string` can be returned — combine multiple values freely:

```cpp
HUD_LABEL_BEGIN(coords)
    HUD_POS(10, 680)
    HUD_VALUE([]{
        Vector2 pos = g_player->Get_position();
        return "X:" + std::to_string((int)pos.x) + " Y:" + std::to_string((int)pos.y);
    })
HUD_LABEL_END
```

---

## Macro reference

| Macro | Applies to | Description |
|---|---|---|
| `HUD_POS(x, y)` | Button, Image, Label | Screen position in pixels |
| `HUD_SIZE(w, h)` | Button, Image | Width and height in pixels |
| `HUD_TEXT("...")` | Button | Text drawn on the button |
| `HUD_TEXTURE("path")` | Button, Image | Texture path — replaces text on buttons |
| `HUD_ON_PRESS(lambda)` | Button | Called once when clicked |
| `HUD_ON_HOLD(lambda)` | Button | Called every frame while held |
| `HUD_VALUE(lambda)` | Label | Lambda returning `std::string` |

---

## Multiple elements in one file

All three types can coexist in a single file:

```cpp
#include "hud_api.h"

HUD_IMAGE_BEGIN(portrait)
    HUD_POS(10, 10)
    HUD_SIZE(64, 64)
    HUD_TEXTURE("assets/textures/portrait.png")
HUD_IMAGE_END

HUD_LABEL_BEGIN(hp)
    HUD_POS(80, 20)
    HUD_VALUE([]{ return "HP: " + std::to_string(g_player->getHp()); })
HUD_LABEL_END

HUD_BUTTON_BEGIN(heal_btn)
    HUD_POS(80, 45)
    HUD_SIZE(60, 24)
    HUD_TEXT("Heilen")
    HUD_ON_PRESS([]{
        g_player->heal(10);
    })
HUD_BUTTON_END
```
