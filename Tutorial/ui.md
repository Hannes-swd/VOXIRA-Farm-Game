# How to Create a UI Popup

> A **Popup** is a modal dialog window the player sees on screen — for example an entry prompt, a shop, or a settings menu.
> This guide uses a **house entrance prompt** as a practical example.

---

## Step 1 — Register the popup in `popups.json`

Open [`assets/json/ui/popups.json`](assets/json/ui/popups.json) and add a new entry:

```json
"house_entrance": {
    "title": "",
    "width": 300,
    "height": 160,
    "modal": true,
    "closeOnEsc": true,
    "backgroundColor": [30, 30, 30, 220]
}
```

| Field | Description |
|---|---|
| `title` | Window title shown at the top — leave empty for no title |
| `width` | Window width in pixels |
| `height` | Window height in pixels |
| `modal` | `true` → blocks all world input while open |
| `closeOnEsc` | `true` → ESC closes the window automatically |
| `backgroundColor` | Background color as `[R, G, B, A]` (0–255) |

---

## Step 2 — Create the popup source file

Inside the [`ui/`](ui/) folder, create a new `.cpp` file (e.g. `house_entrance.cpp`).

Unlike items, buildings, and dimensions — there are **no callback methods** here. Instead you describe the window layout using macros:

```cpp
#include "ui_api.h"

UI_WINDOW_BEGIN("house_entrance")

    UI_TEXT("Willst du das Gebäude betreten?")
    UI_SEPARATOR()

    UI_BUTTON("Ja", []() {
        switchDimension("house_interior");
        closeCurrentUI();
    })

    UI_BUTTON("Nein", []() {
        closeCurrentUI();
    })

UI_WINDOW_END("house_entrance")
```

> **Important:** The ID in `UI_WINDOW_BEGIN` and `UI_WINDOW_END` must **exactly match** the key in `popups.json` — here `"house_entrance"`.

---

## Step 3 — Open the popup from anywhere

Call `openUI("id")` from any existing callback:

```cpp
// In buildings/House.cpp
void onClick() {
    openUI("house_entrance");
}
```

```cpp
// In items/ShopItem.cpp
void onClick() {
    openUI("blacksmith_shop");
}
```

```cpp
// In dimensions/house_interior.cpp
void onUpdate() {
    if (IsKeyPressed(KEY_E)) {
        openUI("exit_menu");
    }
}
```

---

## How it connects

```
popups.json            UI_WINDOW_BEGIN id      house_entrance.cpp
────────────────       ──────────────────      ─────────────────────────────
"house_entrance"   ──► "house_entrance"    ──► UI_TEXT(...)
width: 300                                 ──► UI_SEPARATOR()
height: 160                                ──► UI_BUTTON("Ja",   [](){...})
modal: true                                ──► UI_BUTTON("Nein", [](){...})
```

---

## UI element reference

| Macro | Description |
|---|---|
| `UI_TEXT("text")` | Static text line |
| `UI_TEXT_LIVE("id", "initial text")` | Text line that can be updated at runtime |
| `UI_TITLE("text")` | Centered bold title inside the window |
| `UI_SEPARATOR()` | Horizontal divider line |
| `UI_BUTTON("label", [](){})` | Clickable button with lambda callback |
| `UI_TEXT_INPUT("id", maxLen, [](string v){})` | Text input field |
| `UI_NUMBER_INPUT("id", min, max, default, [](int v){})` | Integer field with +/- buttons |
| `UI_SLIDER("id", min, max, default, [](int v){})` | Drag slider |
| `UI_CHECKBOX("label", default, [](bool v){})` | Toggle checkbox |
| `UI_DROPDOWN("id", options, defaultIdx, [](int v){})` | Dropdown list |

---

## Functions available in callbacks

| Function | Description |
|---|---|
| `closeCurrentUI()` | Close the current popup |
| `openUI("id")` | Open a different popup (closes the current one) |
| `isUIOpen()` | `true` if any popup is currently open |
| `getCurrentUIId()` | ID of the currently open popup |
| `updateUIText("id", "text")` | Update a `UI_TEXT_LIVE` element at runtime |
| `getUIInputValue("id")` | Current value of a `UI_TEXT_INPUT` |
| `getUINumberValue("id")` | Current value of a `UI_NUMBER_INPUT` |
| `getUISliderValue("id")` | Current value of a `UI_SLIDER` |
| `getUICheckboxValue("id")` | Current value of a `UI_CHECKBOX` |
| `addUITextLine("text")` | Show a feedback message at the bottom of the window |
| `switchDimension("id")` | Teleport the player into a dimension |
| `g_player` | Player instance (inventory, position, etc.) |

---

## Full example — Shop with feedback

`popups.json`:
```json
"blacksmith_shop": {
    "title": "Schmiede",
    "width": 400,
    "height": 280,
    "modal": true,
    "closeOnEsc": true,
    "backgroundColor": [25, 20, 15, 230]
}
```

`ui/blacksmith_shop.cpp`:
```cpp
#include "ui_api.h"

UI_WINDOW_BEGIN("blacksmith_shop")

    UI_TEXT("Was möchtest du kaufen?")
    UI_SEPARATOR()

    UI_BUTTON("Schwert kaufen (10 Gold)", []() {
        if (g_player->hasItem("gold", 10)) {
            g_player->removeFromInventory("gold", 10);
            g_player->addToInventory("iron_sword", 1);
            addUITextLine("Du hast ein Schwert gekauft!");
        } else {
            addUITextLine("Nicht genug Gold!");
        }
    })

    UI_BUTTON("Schließen", []() {
        closeCurrentUI();
    })

UI_WINDOW_END("blacksmith_shop")
```

Open from a building:
```cpp
void onClick() {
    openUI("blacksmith_shop");
}
```

---

That's it! Once `popups.json` and your `.cpp` file are in place, the engine registers and loads your popup automatically.
