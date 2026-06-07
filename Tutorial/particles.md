# How to Use Particles

> This guide explains how to spawn particle effects — bursts, trails, and continuous emitters.
> Particles are drawn in world space and work from any callback.

---

## Quick burst

The simplest way: fill a `SpawnParams` struct and call `spawnParticles`.

Include `particle_api.h` in your `.cpp` file:

```cpp
#include "particle_api.h"
```

```cpp
SpawnParams p;
p.count    = 20;       // number of particles
p.color    = RED;      // color
p.speed    = 120.0f;   // pixels per second
p.lifetime = 0.8f;     // seconds before disappearing
p.size     = 4.0f;     // radius in pixels
p.gravity  = 200.0f;   // downward pull (pixels/s²)
p.fade     = true;     // fade out as lifetime drops

spawnParticles(worldX, worldY, p);
```

> Positions are in **world space** (pixels). To get the center of a tile:
> ```cpp
> float wx = (tileX + 0.5f) * 32.0f;
> float wy = (tileY + 0.5f) * 32.0f;
> ```
> To spawn at the player:
> ```cpp
> Vector2 pos = g_player->Get_position();
> spawnParticles(pos.x, pos.y, p);
> ```

---

## SpawnParams reference

| Field | Default | Description |
|---|---|---|
| `count` | 10 | Number of particles to spawn |
| `color` | WHITE | Particle color |
| `speed` | 100.0f | Initial speed in pixels/second |
| `lifetime` | 1.0f | How long each particle lives (seconds) |
| `size` | 3.0f | Radius in pixels |
| `spread` | 360.0f | Spread angle in degrees (360 = all directions) |
| `dirAngle` | 0.0f | Base direction in degrees (used when `spread` < 360) |
| `gravity` | 0.0f | Downward acceleration in pixels/s² |
| `fade` | true | Fade out alpha as particle ages |
| `texture` | nullptr | Optional sprite texture — use `getItemTexture("id")` |
| `rotation` | 0.0f | Start rotation in degrees |
| `rotationSpeed` | 0.0f | Degrees per second — each particle gets a random spin direction |

---

## Directional burst

Use `spread` and `dirAngle` to shoot particles in one direction:

```cpp
SpawnParams p;
p.count    = 15;
p.color    = ORANGE;
p.speed    = 200.0f;
p.lifetime = 0.5f;
p.size     = 3.0f;
p.spread   = 60.0f;    // ±30° cone
p.dirAngle = 270.0f;   // upward (0°=right, 90°=down, 270°=up)
p.gravity  = 150.0f;

spawnParticles(wx, wy, p);
```

---

## Particle template (PARTICLE_BEGIN)

For more control — customize each particle individually and add per-frame behavior.

Create a new `.cpp` file anywhere in [`items/`](items/), [`objects/`](objects/), or any scanned folder:

```cpp
#include "particle_api.h"

PARTICLE_BEGIN("funken", funken)

    // Called once when the particle is created — set its properties here.
    void onSpawn() {
        p.color    = ORANGE;
        p.size     = randomFloat(2.0f, 5.0f);
        p.lifetime = randomFloat(0.4f, 0.8f);
        p.maxLife  = p.lifetime;
        p.gravity  = 180.0f;
        p.fade     = true;
    }

    // Called every frame while the particle is alive.
    void onUpdate(float dt) {
        // Example: shrink over time
        p.size = 4.0f * p.lifeRatio();
    }

PARTICLE_END("funken")
```

> **Important:** The ID string in `PARTICLE_BEGIN` and `PARTICLE_END` must match exactly. The second argument must be a valid C++ identifier.

> Inside both callbacks, `p` (a `Particle&`) is directly available.

Spawn a template:

```cpp
spawnParticleTemplate("funken", wx, wy, 20); // spawn 20 particles
```

---

## Continuous emitter

An emitter spawns particles automatically at a fixed rate. Useful for fire, smoke, rain.

```cpp
// Create an emitter (returns a pointer you can keep)
ParticleEmitter* fire = createParticleEmitter("funken", wx, wy, 30.0f); // 30 particles/second

// Move it each frame if needed
fire->x = newX;
fire->y = newY;

// Remove it when done
destroyParticleEmitter(fire);
```

> Emitters use templates defined with `PARTICLE_BEGIN`. Make sure the template is registered before calling `createParticleEmitter`.

---

## Callback reference (PARTICLE_BEGIN)

| Callback | When it runs |
|---|---|
| `onSpawn()` | Once, when the particle is created |
| `onUpdate(float dt)` | Every frame while the particle is alive |

---

## Functions reference

| Function | Description |
|---|---|
| `spawnParticles(x, y, params)` | Spawn a burst using `SpawnParams` |
| `spawnParticleTemplate(id, x, y, count)` | Spawn particles using a registered template |
| `createParticleEmitter(id, x, y, rate)` | Create a continuous emitter (`rate` = particles/second) |
| `destroyParticleEmitter(emitter*)` | Remove a running emitter |
| `randomFloat(min, max)` | Returns a random float between min and max |
| `getItemTexture(itemId)` | Returns a `Texture2D*` from a registered item — use as `p.texture` |

---

## Particle struct fields

Accessible as `p` inside `onSpawn` and `onUpdate`:

| Field | Type | Description |
|---|---|---|
| `p.x`, `p.y` | `float` | World position |
| `p.vx`, `p.vy` | `float` | Velocity (pixels/second) |
| `p.life` | `float` | Remaining lifetime in seconds |
| `p.maxLife` | `float` | Total lifetime at spawn |
| `p.color` | `Color` | RGBA color |
| `p.size` | `float` | Radius in pixels |
| `p.gravity` | `float` | Downward acceleration |
| `p.fade` | `bool` | Whether alpha fades with lifetime |
| `p.texture` | `Texture2D*` | Sprite texture — `nullptr` = draw as circle |
| `p.rotation` | `float` | Current rotation in degrees |
| `p.rotationSpeed` | `float` | Degrees per second spin |
| `p.lifeRatio()` | `float` | Returns `life / maxLife` (1.0 = fresh, 0.0 = dead) |

---

## Full example — Item that shoots a colored burst on click

```cpp
#include "item api.h"
#include "particle_api.h"

ITEM_BEGIN("partikelItem", partikelItem)

    void onHand() {
        setBuildMode(true);
    }

    void onClick() {
        if (!isBuildMode()) return;
        if (leftClickPressed()) {
            Vector2 t = getTileMouse();
            float wx  = (t.x + 0.5f) * 32.0f;
            float wy  = (t.y + 0.5f) * 32.0f;

            SpawnParams p;
            p.count    = 25;
            p.speed    = 130.0f;
            p.lifetime = 1.0f;
            p.size     = 5.0f;
            p.gravity  = 200.0f;
            p.fade     = true;

            p.color = RED;     spawnParticles(wx, wy, p);
            p.color = YELLOW;  spawnParticles(wx, wy, p);
            p.color = SKYBLUE; spawnParticles(wx, wy, p);
        }
    }

    void onInventory() { }

ITEM_END("partikelItem")
```

---

That's it! Add `#include "particle_api.h"` and call `spawnParticles` from any callback.
