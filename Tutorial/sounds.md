# How to Use Sounds

> This guide explains how to add sound effects and music to your game.
> Sounds work everywhere — in items, buildings, NPCs, objects, and dimensions.

---

## Method 1 — Drop a file into the sounds folder (recommended)

Place any audio file in [`assets/sounds/`](assets/sounds/).
The engine scans this folder automatically at startup.
The **filename without the extension** becomes the sound ID.

```
assets/
  sounds/
    hit.wav          →  playSound("hit")
    footstep.ogg     →  playSound("footstep")
    explosion.mp3    →  playSound("explosion")
```

**Supported formats:** `.wav`  `.ogg`  `.mp3`  `.flac`

> Subfolders inside `sounds/` are also scanned.

---

## Method 2 — Drop a file into the music folder

Place a music file in [`assets/music/`](assets/music/).
The filename without extension becomes the music ID.

```
assets/
  music/
    main_theme.ogg   →  playMusic("main_theme")
    dungeon.mp3      →  playMusic("dungeon")
```

**Supported formats:** `.ogg`  `.mp3`  `.wav`  `.flac`  `.xm`  `.mod`

> Music is **streamed** from disk (suitable for large files).
> Sounds are **loaded into memory** (suitable for short effects).

---

## Method 3 — Procedural beep (no file needed)

For quick test tones without any audio file, define a beep in [`assets/json/sounds.json`](assets/json/sounds.json):

```json
{
    "sounds": {
        "ding":  { "type": "beep", "freq": 880.0, "duration": 0.1 },
        "open":  { "type": "beep", "freq": 523.0, "duration": 0.15 }
    },
    "music": {}
}
```

| Field | Description |
|---|---|
| `freq` | Frequency in Hz (440 = A4, 523 = C5, 880 = A5) |
| `duration` | Length in seconds |

---

## Playing sounds in code

Include `sound_api.h` in your `.cpp` file:

```cpp
#include "sound_api.h"
```

Then call any of these functions from inside callbacks:

```cpp
playSound("hit");              // play a sound effect once
stopSound("hit");              // stop a currently playing sound
setSoundVolume("hit", 0.5f);   // set volume (0.0 – 1.0)

playMusic("main_theme");       // start looping background music
stopMusic();                   // stop current music
setMusicVolume(0.8f);          // set music volume (0.0 – 1.0)
```

---

## Where to use sounds

Sounds can be called from **any** callback in any system:

```cpp
// In an Item
void onClick() {
    playSound("place");
}

// In a Building
void onEnter() {
    playMusic("interior");
}

// In an NPC
void onProximity() {
    playSound("greet");
}

// In an Object
void onInteract() {
    playSound("chest_open");
}

// In a Dimension
void onEnter() {
    playMusic("dungeon");
}
void onLeave() {
    stopMusic();
}
```

---

## Functions reference

| Function | Description |
|---|---|
| `playSound(id)` | Play a sound effect once |
| `stopSound(id)` | Stop a sound effect |
| `setSoundVolume(id, volume)` | Set volume for a sound (0.0 – 1.0) |
| `playMusic(id)` | Start looping music (stops any currently playing music first) |
| `stopMusic()` | Stop the current music |
| `setMusicVolume(volume)` | Set the music volume (0.0 – 1.0) |

---

## Full example — House that plays music on enter / leave

```cpp
#include "building_api.h"
#include "sound_api.h"

BUILDING_BEGIN("House", House)

    void onHover()  { setPendingTooltip("[Click] Enter"); }
    void onClick()  { enterInterior("house_interior"); }
    void onPlace()  { }
    void onUpdate() { }

    void onEnter() {
        playMusic("indoor");
    }

    void onLeave() {
        stopMusic();
        playMusic("main_theme");
    }

BUILDING_END("House")
```

---

That's it! Drop your audio files in `assets/sounds/` or `assets/music/` and they are available immediately — no JSON changes needed.
