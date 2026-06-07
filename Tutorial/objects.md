# How to Create an Object

> This guide uses a **Chest (Truhe)** as a practical example to walk you through the process.
>
> Objects are things that sit on top of the ground — decoration, furniture, chests, switches, trees, rocks.
> Unlike buildings they have no interior, and unlike items they live in the world, not the inventory.

---

## Step 1 — Register your object in `objects.json`

Open [`assets/json/objects/objects.json`](assets/json/objects/objects.json) and add a new entry:

```json
"truhe": {
    "name": "Truhe",
    "texture": "assets/Images/Map/gras.png",
    "width": 1,
    "height": 1,
    "solid": false,
    "interactable": true,
    "interact_radius": 40,
    "layer": "below_player"
}
```

| Field | Description |
|---|---|
| `name` | Display name |
| `texture` | Path to the sprite |
| `width` / `height` | Size in tiles |
| `solid` | Whether players / NPCs can walk through it |
| `interactable` | Whether pressing **E** near it does anything |
| `interact_radius` | Distance in pixels within which E-press is detected |
| `layer` | `"below_player"` (drawn under the player) or `"above_player"` |

> **Note:** The engine automatically binds all three callbacks from your `.cpp` file as long as the `OBJECT_BEGIN` ID matches the key in `objects.json`.

---

## Step 2 — Create the object source file

Inside the [`objects/`](objects/) folder, create a new `.cpp` file (e.g. `Truhe.cpp`).

All three callback functions must always be named exactly as shown:

```cpp
#include "object_api.h"

OBJECT_BEGIN("truhe", Truhe)

    // Called every frame — use this for tooltips and proximity checks.
    void onUpdate() {

    }

    // Called once when the player presses E within interact_radius.
    void onInteract() {

    }

    // Called once when the object is removed from the world.
    void onDestroy() {

    }

OBJECT_END("truhe")
```

> **Important:** The ID string in `OBJECT_BEGIN` and `OBJECT_END` must **exactly match** the key in `objects.json` — here `"truhe"`. The second argument (here `Truhe`) must be a valid C++ identifier.

> In all three callbacks `obj` (a `PlacedObject&`) is directly available — it holds the object's position and ID.

---

## Step 3 — Create an item that places the object

Objects are placed through items. Inside the [`items/`](items/) folder, create a new `.cpp` file, and register it in `item.json`.

```cpp
#include "item_api.h"
#include "object_api.h"

ITEM_BEGIN("truhenItem", truhenItem)

    void onHand() {
        setBuildMode(true);
    }

    void onClick() {
        if (!isBuildMode()) return;
        if (leftClickPressed()) {
            Vector2 t = getTileMouse();
            placeObject("truhe", (int)t.x, (int)t.y);
            g_player->removeFromInventory("truhenItem", 1);
        }
    }

    void onInventory() { }

ITEM_END("truhenItem")
```

`item.json` entry:

```json
"truhenItem": {
    "name": "Truhe",
    "textur": "assets/Images/Map/gras.png"
}
```

---

## How it connects

```
objects.json       OBJECT_BEGIN id      Truhe.cpp
──────────────     ────────────────     ─────────────────────────
"truhe"        ──► "truhe"          ──► void onUpdate()   { }
                                    ──► void onInteract() { }
                                    ──► void onDestroy()  { }
```

---

## Callback reference

| Callback | When it runs |
|---|---|
| `onUpdate()` | Every frame for every placed instance of this type |
| `onInteract()` | Once when the player presses E within `interact_radius` |
| `onDestroy()` | Once when the object is removed from the world |

---

## Functions available inside callbacks

| Function | Description |
|---|---|
| `isPlayerNear(obj, radius)` | Returns `true` if the player is within `radius` pixels of this object |
| `placeObject(id, tileX, tileY)` | Place an object of the given type at tile coordinates |
| `removeObject(instanceId)` | Remove a placed object by its instance ID |
| `getObjectInstanceId()` | Returns the unique ID of the currently active object |
| `setPendingTooltip(text)` | Show a tooltip near the mouse cursor this frame |
| `playSound(id)` | Play a sound — see [`sounds.md`](sounds.md) |
| `spawnParticles(x, y, params)` | Spawn a particle burst — see [`particles.md`](particles.md) |

---

## Full example — Chest that plays a sound and spawns particles

```cpp
#include "object_api.h"
#include "sound_api.h"
#include "particle_api.h"

OBJECT_BEGIN("truhe", Truhe)

    void onUpdate() {
        // Show tooltip when the player is close enough
        if (isPlayerNear(obj, 40.0f))
            setPendingTooltip("[E] Open chest");
    }

    void onInteract() {
        // World-space center of this tile
        float wx = (obj.tileX + 0.5f) * 32.0f;
        float wy = (obj.tileY + 0.5f) * 32.0f;

        // Gold particle burst
        SpawnParams p;
        p.count    = 20;
        p.color    = GOLD;
        p.speed    = 90.0f;
        p.lifetime = 0.9f;
        p.size     = 4.0f;
        p.gravity  = 250.0f;
        p.fade     = true;
        spawnParticles(wx, wy, p);

        playSound("truhe_open");
    }

    void onDestroy() { }

OBJECT_END("truhe")
```

---

That's it! Drop the `.cpp` file in `objects/`, add the entry to `objects.json`, and the engine will pick it up automatically on the next build.
