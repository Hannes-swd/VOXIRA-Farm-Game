# Biomes

> A biome is a zone determined by the **biome map** (height value 0.0–1.0).
> Inside each biome a second independent map — the **texture map** — controls which exact tile variant is placed.
> Both maps can be noise (automatic) or a PNG image you supply.

---

## Two independent maps

```
Biome map              Texture map
──────────────         ──────────────────────────────
height = 0.61          texHeight = 0.72
  → biome "wald"         → within "wald":  tile "wald_dunkel"
```

| Map | Controls | Source |
|---|---|---|
| **Biome map** | Which biome the tile belongs to | `heightmap` PNG  or noise (`noise_scale`) |
| **Texture map** | Which tile variant inside that biome | `texture_heightmap` PNG or noise (`tex_noise_scale`) |

If you don't supply a `texture_heightmap`, the engine automatically generates one using a finer noise — no extra work needed.

---

## Biome structure

```json
{
    "name":       "wald",
    "height_min": 0.55,
    "height_max": 0.75,
    "tile":       "gras",
    "tiles": [
        { "tile": "wald_boden_dunkel", "height_min": 0.00, "height_max": 0.40 },
        { "tile": "wald_boden",        "height_min": 0.40, "height_max": 0.75 },
        { "tile": "wald_boden_hell",   "height_min": 0.75, "height_max": 1.00 }
    ],
    "objects": [
        { "id": "baum",  "chance": 0.18 },
        { "id": "busch", "chance": 0.06 }
    ]
}
```

| Field | Required | Description |
|---|---|---|
| `name` | yes | Label for readability only |
| `height_min` / `height_max` | yes | Range on the **biome map** |
| `tile` | yes | Fallback tile — used when `tiles` is empty or no variant matches |
| `tiles` | no | Texture variants — picked by the **texture map** |
| `objects` | no | Objects randomly placed in this biome |

---

## Biome height ranges

All biomes cover the 0.0–1.0 range of the biome map without gaps.

```
0.00 ──── 0.30 ──── 0.42 ──────── 0.65 ──── 0.82 ──── 1.00
│  see    │ strand  │    ebene     │  wald   │   berg   │
```

Ranges must not overlap. Touching is correct (`height_max` of one = `height_min` of the next).

---

## Texture variants inside a biome

`tiles` uses the **texture map** value (0.0–1.0) to pick one of several tiles within the same biome.
The ranges work exactly like biome ranges, just on the texture map.

```json
"tiles": [
    { "tile": "gras_dunkel", "height_min": 0.00, "height_max": 0.35 },
    { "tile": "gras",        "height_min": 0.35, "height_max": 0.70 },
    { "tile": "gras_hell",   "height_min": 0.70, "height_max": 1.00 }
]
```

If no `tiles` entry is defined (or none matches), the biome's `tile` is used.

Each tile ID must exist in [`assets/json/Map/ground.json`](../assets/json/Map/ground.json).

---

## Using PNG heightmaps for both maps

```json
{
    "enabled": true,
    "mode": "heightmap",
    "heightmap":         "Images/Map/biome_map.png",
    "texture_heightmap": "Images/Map/texture_map.png",
    "default_tile": "gras",
    "biomes": [ ... ]
}
```

| Field | Description |
|---|---|
| `heightmap` | PNG that determines which biome — bright = high value, dark = low value |
| `texture_heightmap` | PNG that drives texture variation within each biome |

Both paths are relative to `assets/`.
If `texture_heightmap` is missing the engine uses automatic noise for texture variation.

---

## Texture map — noise without a PNG

If you don't provide `texture_heightmap`, these fields control the auto-noise:

```json
{
    "tex_noise_scale":  0.08,
    "tex_octaves":      2,
    "tex_persistence":  0.5
}
```

| Field | Effect |
|---|---|
| `tex_noise_scale` | Size of texture patches. `0.04` = large patches, `0.15` = fine grain |
| `tex_octaves` | Detail layers. `1`–`2` is usually enough for texture variation |
| `tex_persistence` | Contrast of the noise |

---

## Full example — four biomes with texture variation

```json
{
    "enabled": true,
    "mode": "noise",
    "seed": 7,
    "noise_scale": 0.035,
    "octaves": 5,
    "persistence": 0.48,
    "default_tile": "gras",

    "tex_noise_scale": 0.09,
    "tex_octaves": 2,
    "tex_persistence": 0.5,

    "biomes": [
        {
            "name": "see",
            "height_min": 0.00,
            "height_max": 0.30,
            "tile": "wasser"
        },
        {
            "name": "strand",
            "height_min": 0.30,
            "height_max": 0.42,
            "tile": "sand"
        },
        {
            "name": "ebene",
            "height_min": 0.42,
            "height_max": 0.65,
            "tile": "gras",
            "tiles": [
                { "tile": "gras_dunkel", "height_min": 0.00, "height_max": 0.40 },
                { "tile": "gras",        "height_min": 0.40, "height_max": 0.75 },
                { "tile": "gras_hell",   "height_min": 0.75, "height_max": 1.00 }
            ],
            "objects": [
                { "id": "blume", "chance": 0.04 }
            ]
        },
        {
            "name": "wald",
            "height_min": 0.65,
            "height_max": 0.82,
            "tile": "wald_boden",
            "tiles": [
                { "tile": "wald_boden_dunkel", "height_min": 0.00, "height_max": 0.45 },
                { "tile": "wald_boden",        "height_min": 0.45, "height_max": 1.00 }
            ],
            "objects": [
                { "id": "baum",  "chance": 0.20 },
                { "id": "busch", "chance": 0.07 }
            ]
        },
        {
            "name": "berg",
            "height_min": 0.82,
            "height_max": 1.00,
            "tile": "stein",
            "tiles": [
                { "tile": "stein",      "height_min": 0.00, "height_max": 0.60 },
                { "tile": "stein_hell", "height_min": 0.60, "height_max": 1.00 }
            ],
            "objects": [
                { "id": "felsen", "chance": 0.10 }
            ]
        }
    ]
}
```

---

## Placing objects (trees, rocks, bushes)

Each entry in `"objects"`:

| Field | Description |
|---|---|
| `id` | Object ID — must exist in `assets/json/objects/objects.json` |
| `chance` | Probability per tile: `0.15` = 15 % |

Multiple objects per biome use a priority roll — the sum of all chances is the total fill rate.
Object placement is seed-based and deterministic.

---

## Tips

- The **biome map** should have low-frequency features (large blobs). Use `noise_scale` around `0.02`–`0.05`.
- The **texture map** should be finer than the biome map. Use `tex_noise_scale` around `0.07`–`0.15`.
- Using a PNG for the biome map and auto-noise for texture is a common workflow: paint the large layout, let noise add variation.
- Tile IDs in `tiles` must exist in `ground.json` — unknown IDs silently fall back to the biome's `tile`.
- To regenerate the world with new settings, delete `assets/json/Map/chunks/`.
