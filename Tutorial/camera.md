# How to Control the Camera

> This guide explains all camera modes and effects available in the engine.
> Camera functions work from any callback — items, buildings, NPCs, objects, dimensions.

---

## Include

```cpp
#include "cam_api.h"
```

---

## Camera modes

The camera has five modes. Switch between them at any time.

### Follow player (default)

```cpp
setCameraFollowPlayer();
```

The camera follows the player automatically. This is the default when the game starts.

---

### Fixed — freeze the camera at a position

```cpp
setCameraFixed(worldX, worldY);
```

The camera stops at the given world-space position. The player can still move freely.

```cpp
// Freeze where the camera currently is
setCameraFixed(camera.target.x, camera.target.y);
```

---

### Free — manually positioned

```cpp
setCameraFree();
```

The camera stays where it is and stops tracking anything. Use `setCameraFixed` or `cameraMoveToSmooth` to reposition it from code.

---

### Follow NPC

```cpp
setCameraFollowNpc("farmer_0");
```

The camera follows a specific NPC. Pass the NPC's instance ID (e.g. `"farmer_0"`).

---

### Cinematic — smooth animated movement

```cpp
cameraMoveToSmooth(worldX, worldY, duration);
```

The camera glides to the target position over `duration` seconds using a smooth-step curve.

```cpp
cameraMoveToSmooth(500.0f, 300.0f, 2.5f); // move to (500, 300) in 2.5 seconds
```

Jump to a position instantly:

```cpp
cameraMoveToInstant(worldX, worldY);
```

---

## Effects

### Camera shake

```cpp
cameraShake(intensity, duration);
```

Shakes the camera for `duration` seconds. The shake fades out over time.

```cpp
cameraShake(8.0f, 0.4f);  // strong short shake (e.g. explosion)
cameraShake(3.0f, 1.0f);  // gentle long shake (e.g. earthquake)
```

---

## Fine-tuning

### Smooth follow (lerp)

```cpp
setCameraLerp(0.1f);
```

Controls how quickly the camera catches up to its target.

| Value | Behavior |
|---|---|
| `1.0` | Instant — camera snaps to target every frame (default) |
| `0.1` | Smooth — camera slowly drifts toward the target |
| `0.0` | Camera never moves |

---

### Deadzone

```cpp
setCameraDeadzone(20.0f);
```

The player can move up to 20 pixels from the camera center without the camera moving. The camera only starts following once the player leaves that radius.

---

### Offset

```cpp
setCameraOffset(0.0f, -40.0f);
```

Shifts the camera focus point. Useful to show more of the screen in a particular direction.

```cpp
setCameraOffset(0.0f, -40.0f); // look slightly ahead of the player (upward shift)
```

---

### Bounds — keep camera inside an area

```cpp
setCameraBounds(0, 0, 3200, 3200); // camera stays within this rectangle
clearCameraBounds();               // remove the restriction
```

Coordinates are in world space (pixels). The camera target will never leave this rectangle.

---

## Mode reference

| Function | Description |
|---|---|
| `setCameraFollowPlayer()` | Camera follows the player (default) |
| `setCameraFollowNpc(instanceId)` | Camera follows a specific NPC |
| `setCameraFree()` | Camera stays where it is, tracks nothing |
| `setCameraFixed(x, y)` | Camera locked to a world position |
| `cameraMoveToSmooth(x, y, duration)` | Smooth animated camera movement |
| `cameraMoveToInstant(x, y)` | Instant camera jump |

## Effect reference

| Function | Description |
|---|---|
| `cameraShake(intensity, duration)` | Screen shake that fades over time |
| `setCameraLerp(value)` | Follow smoothness (1.0 = instant, 0.1 = soft) |
| `setCameraDeadzone(radius)` | Pixels the player moves before camera follows |
| `setCameraOffset(x, y)` | Shift the camera focus point |
| `setCameraBounds(x, y, w, h)` | Restrict camera to a rectangular area |
| `clearCameraBounds()` | Remove camera bounds restriction |

---

## Full example — Item that cycles through three camera modes

```cpp
#include "item api.h"
#include "cam_api.h"

namespace { static int mode = 0; }

ITEM_BEGIN("kameraItem", kameraItem)

    void onHand() {
        if (mode == 0) setPendingTooltip("[Click] Freeze camera");
        if (mode == 1) setPendingTooltip("[Click] Cinematic pan");
        if (mode == 2) setPendingTooltip("[Click] Back to player");
    }

    void onClick() {
        if (!leftClickPressed()) return;

        if (mode == 0) {
            // Freeze the camera at its current position
            setCameraFixed(camera.target.x, camera.target.y);
            mode = 1;

        } else if (mode == 1) {
            // Glide to a point near the player over 3 seconds
            Vector2 pos = g_player->Get_position();
            cameraMoveToSmooth(pos.x + 100.0f, pos.y - 80.0f, 3.0f);
            mode = 2;

        } else {
            // Return to following the player + shake to signal the reset
            setCameraFollowPlayer();
            cameraShake(6.0f, 0.5f);
            mode = 0;
        }
    }

    void onInventory() { }

ITEM_END("kameraItem")
```

---

## Useful in dimensions

When entering a small room it looks better if the camera is fixed at the room center rather than following the player to the edge:

```cpp
#include "dimension_api.h"
#include "cam_api.h"

DIMENSION_BEGIN("house_interior", house_interior)

    void onEnter() {
        // Fix camera at center of the 4×10 room
        setCameraFixed(2 * 32.0f, 5 * 32.0f);
        setCameraLerp(0.08f);
    }

    void onLeave() {
        // Restore smooth player follow
        setCameraFollowPlayer();
        setCameraLerp(1.0f);
    }

    void onUpdate() {
        if (IsKeyPressed(KEY_E)) leaveCurrentDimension();
    }

    void onDraw() { }

DIMENSION_END("house_interior")
```

---

That's it! Include `cam_api.h` and call the functions from any callback.
