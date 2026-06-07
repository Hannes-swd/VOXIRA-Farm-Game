# World Generation

> The world generation system lets modders auto-generate terrain from a single JSON file — no C++ required.
> By default it is **disabled**: the world is empty and you place tiles by hand.
> Enable it and the engine fills every new chunk automatically using your biome rules.

---

## How it works

When the engine loads a chunk for the first time (no saved file exists yet), it checks whether world generation is enabled.
If yes, it runs each tile through your biome rules and places the result.
The chunk is then saved to disk — future loads use the saved file, so player edits are never overwritten.

```
new chunk requested
        │
        ▼
 generation.json enabled?
    yes ──► generate all 16×16 tiles using biome rules
            place objects (trees, rocks, …)
            mark chunk dirty → saved immediately
    no  ──► all tiles = default tile (gras)
```

---

## Enable world generation

Open [`assets/json/Map/generation.json`](../assets/json/Map/generation.json) and set `"enabled"` to `true`:

```json
{
    "enabled": true,
    "mode": "noise",
    "seed": 42,
    ...
}
```

Set it back to `false` at any time to stop generating new chunks (existing chunks are unaffected).

---

## Top-level fields

| Field | Type | Description |
|---|---|---|
| `enabled` | bool | `true` to activate generation, `false` to disable |
| `mode` | string | `"noise"` (procedural) or `"heightmap"` (PNG image) |
| `seed` | int | Changes the entire world layout. Same seed = same world. |
| `noise_scale` | float | Controls feature size. `0.02` = large continents, `0.10` = small patches. |
| `octaves` | int | Detail layers. `1` = smooth blobs, `6` = rough and noisy. |
| `persistence` | float | How much each detail layer contributes. `0.3` = smooth, `0.7` = jagged. |
| `default_tile` | string | Tile used when no biome matches the height value. |

---

## Minimal working example

```json
{
    "enabled": true,
    "mode": "noise",
    "seed": 1,
    "noise_scale": 0.04,
    "octaves": 4,
    "persistence": 0.5,
    "default_tile": "gras",
    "biomes": [
        { "name": "ebene", "height_min": 0.0, "height_max": 1.0, "tile": "gras" }
    ]
}
```

This fills the entire world with `gras`. See [`biomes.md`](biomes.md) to split it into different zones.

---

## Deleting a generated world

Remove or empty the `assets/json/Map/chunks/` folder and set a new `seed`.
The next time the game starts, all chunks are freshly generated.

> Player data (position, inventory) is stored in `assets/json/player/player.json` — it is unaffected.

---

## Switching modes

You can switch between `"noise"` and `"heightmap"` at any time.
Only chunks that have not been generated yet will use the new settings.
To regenerate everything, delete the `chunks/` folder.

For the heightmap workflow see [`heightmap.md`](heightmap.md).
For biome setup see [`biomes.md`](biomes.md).
