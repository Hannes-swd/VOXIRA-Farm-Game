# How to Modify the Game

> This guide uses a **Grass Item** as a practical example to walk you through the process.

---

## Step 1 — Register your item in `item.json`

Open [`assets/json/items/item.json`](assets/json/items/item.json) and add a new entry using the following template:

```json
"grasItem": {
    "name": "gras",
    "textur": "assets/Images/Map/gras.png"
}
```

| Field | Description |
|---|---|
| `name` | Display name of the item |
| `textur` | Path to the item's sprite/image |

> **Note:** You do **not** need to specify callback function names in `item.json`. The engine automatically finds and binds `onHand`, `onClick`, and `onInventory` from your `.cpp` file as long as the `ITEM_BEGIN` ID matches the key in `item.json`.

---

## Step 2 — Create the item source file

Inside the [`items/`](items/) folder, create a new `.cpp` file for your item (e.g. `Gras.cpp`).

The three callback functions must always be named exactly `onHand`, `onClick`, and `onInventory`:

```cpp
#include "item_api.h"

ITEM_BEGIN("grasItem", grasItem)

    // Called every tick while the item is held.
    void onHand() {

    }

    // Called when the player clicks with this item selected.
    void onClick() {

    }

    // Called every tick while the item is in the inventory.
    void onInventory() {

    }

ITEM_END("grasItem")
```

> **Important:** The ID string in `ITEM_BEGIN` and `ITEM_END` must **exactly match** the key in `item.json` — here `"grasItem"`. The second argument of `ITEM_BEGIN` (here `grasItem`) must be a valid C++ identifier with no spaces or special characters.

> **Note:** `onClick()` automatically skips execution when the mouse is over the UI — you don't need to check for that manually.

---

## How it connects

```
item.json          ITEM_BEGIN id       Gras.cpp
──────────────     ───────────────     ──────────────────────
"grasItem"     ──► "grasItem"      ──► void onHand()     { }
                                   ──► void onClick()    { }
                                   ──► void onInventory(){ }
```

The engine reads the item ID from `item.json` and at startup automatically binds `onHand`, `onClick`, and `onInventory` from the matching `ITEM_BEGIN` block in your `.cpp` file.

---

## Full example — Grass placement item

```cpp
#include "item_api.h"

ITEM_BEGIN("grasItem", grasItem)

    void onHand() {
        setBuildMode(true);  // show placement grid while held
    }

    void onClick() {
        if (!isBuildMode()) return;
        if (leftClickPressed()) {
            Vector2 t = getTileMouse();
            setTile((int)t.x, (int)t.y, "gras");
            g_player->removeFromInventory("grasItem", 1);
        }
    }

    void onInventory() {
        // nothing
    }

ITEM_END("grasItem")
```

---

That's it! Once both files are in place, the engine will automatically register and load your new item.