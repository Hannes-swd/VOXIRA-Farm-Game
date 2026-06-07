# Voxira Game Engine

A 2D game engine where you build your game by editing files — JSON for data, C++ for logic — and then run it.
There is no visual editor for building the game itself; the engine is the runtime, and **the files are your game**.

> Some parts of this project were developed with the help of AI.

---

## The idea

Most engines have you click through editors and drag things around.
Voxira works differently: you **write your game first, then start it**.

- World layout, biomes, items, objects, sounds, the player — everything lives in JSON or C++ files.
- You edit those files, hit compile, and the engine loads your definitions automatically.
- The goal is that everything you would want to do in 2D should be possible.

There is no limit to what you can build — combat systems, crafting, procedural worlds, custom UI, particles, dimensions — as long as it's 2D, the engine is designed to support it.

---

## Full access when you need it

The engine comes with a ready-made set of API functions you can call from your item and object scripts.
But if you ever need something the API doesn't cover yet, **you have the complete engine source right there**.
Nothing is hidden or locked — open the relevant file, add what you need, and rebuild.

On top of the engine API, **all of raylib is available directly** in every C++ script.
If you know raylib you can draw custom shapes, play sounds at exact positions, do raycasts, or anything else raylib supports — no wrappers, no restrictions.

The engine also has a **built-in GUI system** you can use from scripts, so building in-game menus, panels, and interactive windows is supported out of the box.

---

## How to make a game

1. **Configure** — edit JSON files in `assets/json/` to define tiles, items, objects, sounds, the world.
2. **Script** — add C++ files in `items/`, `objects/`, `player/` etc. to give things behavior.
3. **Build & run** — compile and start the engine. Your game is now running.

You never have to touch the engine source code — the `src/` folder can stay completely untouched.
The engine picks up your files automatically at startup.

---

## What you can do

| Feature | How |
|---|---|
| Tiles & ground | `assets/json/Map/ground.json` |
| Procedural world gen | `assets/json/Map/generation.json` — noise or PNG heightmap |
| Biomes with texture variation | Defined inside `generation.json` |
| Items with custom logic | Register in `item.json`, write `items/YourItem.cpp` |
| World objects (chests, trees, …) | Register in `objects.json`, write `objects/YourObject.cpp` |
| Player extensions | `player/player_ext_vars.h` + `player/player_ext_methods.h` |
| Sounds | `assets/json/sounds.json` |
| Particles | `particle_api.h` — burst params in C++ |
| HUD / UI | `ui/` folder |
| Multiple dimensions | `assets/json/dimension/` |
| Camera | Configurable via JSON |

---

## Project structure

```
assets/
  json/          ← all game data (JSON)
  Images/        ← sprites and textures
items/           ← C++ item behaviour files
objects/         ← C++ object behaviour files
player/          ← player extension headers
Tutorial/        ← full documentation (see below)
```

---

## Tutorial

Everything is documented in the [`Tutorial/`](Tutorial/) folder.
Each feature has its own file:

| File | Topic |
|---|---|
| [`worldgen.md`](Tutorial/worldgen.md) | Procedural world generation |
| [`biomes.md`](Tutorial/biomes.md) | Biome setup and texture variation |
| [`heightmap.md`](Tutorial/heightmap.md) | PNG-based world maps |
| [`items.md`](Tutorial/Items.md) | Creating items with custom logic |
| [`objects.md`](Tutorial/objects.md) | World objects (chests, trees, …) |
| [`player.md`](Tutorial/player.md) | Extending the player |
| [`sounds.md`](Tutorial/sounds.md) | Sound system |
| [`particles.md`](Tutorial/particles.md) | Particle effects |
| [`hud.md`](Tutorial/hud.md) | HUD and UI |
| [`dimension.md`](Tutorial/dimension.md) | Multiple dimensions / areas |
| [`camera.md`](Tutorial/camera.md) | Camera configuration |
| [`house.md`](Tutorial/house.md) | Buildings |
| [`functions.md`](Tutorial/functions.md) | Available API functions |
| [`ui.md`](Tutorial/ui.md) | UI system |

---

## Built with

- [raylib](https://www.raylib.com/) — rendering and input
- C++17
