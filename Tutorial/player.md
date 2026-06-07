# How to Extend the Player

> This guide uses a **hunger system** as a practical example.

---

## Overview

You can add private variables and public methods directly to the player class — without touching any engine source file.

There are exactly two files you edit, both in the [`player/`](../player/) folder:

| File | What goes in it |
|---|---|
| `player/player_ext_vars.h` | Private member variables |
| `player/player_ext_methods.h` | Public methods |

Both files are `#include`d directly inside the `player` class at compile time, so everything you write there becomes a real member of the class.

---

## Step 1 — Add a variable

Open [`player/player_ext_vars.h`](../player/player_ext_vars.h) and add your variable:

```cpp
int essen = 100;
```

- The variable is **private** — only methods in `player_ext_methods.h` (and the player class itself) can access it.
- You can set a default value directly.

---

## Step 2 — Add methods

Open [`player/player_ext_methods.h`](../player/player_ext_methods.h) and add your methods:

```cpp
int  GetEssen() const      { return essen; }
void GiebEssen(int menge)  { essen = std::min(100, essen + menge); }
void NimmEssen(int menge)  { essen = std::max(0,   essen - menge); }
```

- Methods are **public** — callable from anywhere via `g_player`.
- All variables from `player_ext_vars.h` are directly accessible here.

---

## Step 3 — Use it from other files

In any item, building, or dimension file, call the methods through `g_player`:

```cpp
#include "item_api.h"

ITEM_BEGIN("food", food)

    void onClick() {
        g_player->GiebEssen(20);
    }

    void onHand()      { }
    void onInventory() { }

ITEM_END("food")
```

---

## How it connects

```
player_ext_vars.h          player_ext_methods.h        anywhere
──────────────────         ────────────────────────    ─────────────────────────
int essen = 100;    ──►    int  GetEssen() const { }   g_player->GetEssen()
                    ──►    void GiebEssen(int m) { }   g_player->GiebEssen(20)
                    ──►    void NimmEssen(int m) { }   g_player->NimmEssen(5)
```

The engine compiles both files directly into the `player` class — no registration, no callbacks, no overhead.

---

## Full example — Hunger system

**`player/player_ext_vars.h`**
```cpp
int essen = 100;
```

**`player/player_ext_methods.h`**
```cpp
int  GetEssen() const      { return essen; }
void GiebEssen(int menge)  { essen = std::min(100, essen + menge); }
void NimmEssen(int menge)  { essen = std::max(0,   essen - menge); }
```

**`items/Food.cpp`**
```cpp
#include "item_api.h"

ITEM_BEGIN("food", food)

    void onClick() {
        g_player->GiebEssen(20);
    }

    void onHand()      { }
    void onInventory() { }

ITEM_END("food")
```

---

## Notes

- You can add as many variables and methods as you want.
- Methods can be inline (defined directly) or call each other.
- Use `std::min` / `std::max` to clamp values — include `<algorithm>` at the top of `player_ext_methods.h` if needed.
- Variables are reset to their default value each time the game starts (no automatic save/load).

---

That's it! No engine files need to be changed — just add to the two files in `player/` and rebuild.
