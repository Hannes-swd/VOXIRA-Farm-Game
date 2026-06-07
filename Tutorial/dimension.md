# How to Create a Dimension

> A **Dimension** is a separate, bounded map the player can enter — for example a house interior, a dungeon, or a cave.
> This guide uses a **house interior** as a practical example.

---

## Step 1 — Register the dimension in `dimensions.json`

Open [`assets/json/Map/dimensions.json`](assets/json/Map/dimensions.json) and add a new entry:

```json
"house_interior": {
    "name": "House Interior",
    "width": 4,
    "height": 10,
    "backgroundColor": [50, 30, 20, 255],
    "defaultTile": "concrete",
    "sharedInterior": false
}
```

| Field | Description |
|---|---|
| `name` | Display name of the dimension |
| `width` | Width in tiles — this is where the size is defined |
| `height` | Height in tiles — this is where the size is defined |
| `backgroundColor` | Background color as `[R, G, B, A]` (0–255) |
| `defaultTile` | Tile type used for cells that have no tile placed yet |
| `sharedInterior` | `true` → all building instances share one dimension · `false` → each placed building gets its own copy (default: `true`) |

> The dimension ID (here `"house_interior"`) is the key used to enter and leave it via `switchDimension()`.

---

## Step 2 — Create the dimension source file

Inside the [`dimensions/`](dimensions/) folder, create a new `.cpp` file (e.g. `house_interior.cpp`).

All four callback functions must always be named exactly as shown:

```cpp
#include "dimension_api.h"

DIMENSION_BEGIN("house_interior", house_interior)

    // Called once when the player enters this dimension.
    void onEnter() {

    }

    // Called once when the player leaves this dimension.
    void onLeave() {

    }

    // Called every frame while the player is inside this dimension.
    void onUpdate() {

    }

    // Called every frame for extra drawing (world-space, inside BeginMode2D).
    void onDraw() {

    }

DIMENSION_END("house_interior")
```

> **Important:** The ID string in `DIMENSION_BEGIN` and `DIMENSION_END` must **exactly match** the key in `dimensions.json` — here `"house_interior"`. The second argument (here `house_interior`) must be a valid C++ identifier with no spaces or special characters.

---

## Step 3 — Enter the dimension from a building

Call `switchDimension("house_interior")` from a building's `onClick()` to send the player inside:

```cpp
// inside House.cpp
void onClick() {
    switchDimension("house_interior");
}
```

To leave the dimension and return to the world, call `leaveCurrentDimension()`:

```cpp
// inside house_interior.cpp
void onUpdate() {
    if (IsKeyPressed(KEY_E)) {
        leaveCurrentDimension();
    }
}
```

---

## How it connects

```
dimensions.json         DIMENSION_BEGIN id        house_interior.cpp
──────────────────      ──────────────────────    ────────────────────────────
"house_interior"    ──► "house_interior"      ──► void onEnter()  { }
                                              ──► void onLeave()  { }
                                              ──► void onUpdate() { }
                                              ──► void onDraw()   { }
```

---

## Player behaviour inside a dimension

- The player **cannot move outside** the dimension bounds (`width` × `height` tiles).
- `setTile(x, y, type)` automatically writes to the **dimension's tile map** while the player is inside — tiles placed outside the bounds are silently ignored.
- Dimension tiles are **saved automatically** every 30 seconds and when the player leaves.
- Each dimension keeps its own tile state independently of the main world.

---

## Callback reference

| Callback | When it runs |
|---|---|
| `onEnter()` | Once when the player enters the dimension |
| `onLeave()` | Once when the player leaves the dimension |
| `onUpdate()` | Every frame while the player is inside |
| `onDraw()` | Every frame, for extra drawing (world-space) |

---

## Functions available inside callbacks

| Function | Description |
|---|---|
| `leaveCurrentDimension()` | Return the player to the main world |
| `isInDimension()` | Returns `true` if the player is currently in any dimension |
| `getCurrentDimensionId()` | Returns the ID of the current dimension (`""` in the main world) |
| `setDimensionTile(x, y, type)` | Place a tile inside the current dimension (bounds-checked) |
| `getDimensionTile(x, y)` | Read the tile type at position `(x, y)` in the current dimension |
| `getDimensionWidth()` | Width of the current dimension in tiles |
| `getDimensionHeight()` | Height of the current dimension in tiles |
| `setTile(x, y, type)` | Same as `setDimensionTile` when inside a dimension |
| `getTileMouse()` | Tile coordinates under the mouse cursor |
| `leftClickPressed()` | `true` on the first frame of a left click |
| `IsMouseOnUi()` | `true` if the mouse is over the hotbar or inventory |
| `switchDimension(id)` | Enter a dimension by ID (use `""` to return to the world) |

---

## Full example — House interior with tile placement and exit key

```cpp
#include "dimension_api.h"

DIMENSION_BEGIN("house_interior", house_interior)

    void onEnter() {
        // runs once when the player steps inside
    }

    void onLeave() {
        // runs once when the player steps outside
    }

    void onUpdate() {
        // Press E to leave the dimension and return to the world
        if (IsKeyPressed(KEY_E)) {
            leaveCurrentDimension();
        }
    }

    void onDraw() {
        // optional: extra UI or world-space drawing
    }

DIMENSION_END("house_interior")
```

Tile placement (e.g. with a Beton item) works the same as in the main world — just use `setTile()` or `setBuildMode(true)` in your item. While the player is inside a dimension, `setTile()` automatically writes to that dimension's tile map and respects the dimension's bounds.

---

## Dimension tile storage

Dimension tiles are stored as separate JSON files under `assets/json/Map/dimensions/`:

```
assets/json/Map/dimensions/
    house_interior.json
    dungeon.json
    ...
```

Each file is created automatically the first time tiles are placed or the player leaves. You do not need to create these files manually.

---

That's it! Once `dimensions.json` and your `.cpp` file are in place, the engine will automatically register and load your dimension.
