# Heightmap Mode

> Instead of procedural noise, you supply **grayscale PNG images** to control terrain.
> The engine supports two independent maps:
> - **Biome map** (`heightmap`) — determines which biome each tile belongs to
> - **Texture map** (`texture_heightmap`) — controls tile variation *within* each biome
>
> Both are optional and can be mixed freely with noise.

---

## How the two maps work together

```
Biome map pixel:    dark  (0.20) → biome "see"   → tile "wasser"
Biome map pixel:    grey  (0.55) → biome "ebene" → texture map decides:
  Texture map pixel: dark  (0.25) → tile "gras_dunkel"
  Texture map pixel: light (0.80) → tile "gras_hell"
```

You can use:
- Both maps as PNG
- Biome map as PNG + texture variation as automatic noise (most common)
- Both maps as automatic noise (no images needed at all)

---

## Creating the images

Any image editor works (Photoshop, GIMP, Aseprite, Paint.NET).

| Property | Value |
|---|---|
| Format | PNG |
| Color mode | Grayscale (or RGB — the engine converts it automatically) |
| Size | Any size. The image **tiles** across the world. 256×256 px is a good starting point. |

| Pixel brightness | Height value | Typical use |
|---|---|---|
| Black (0) | 0.0 | Deep water, valleys |
| Dark grey | ~0.3 | Shallow water, coast |
| Mid grey | ~0.5 | Plains, meadows |
| Light grey | ~0.7 | Hills, forest |
| White (255) | 1.0 | Mountains, peaks |

Save images anywhere inside `assets/`, e.g.:
```
assets/Images/Map/biome_map.png
assets/Images/Map/texture_map.png
```

---

## Step 1 — Biome map only (simplest setup)

Open [`assets/json/Map/generation.json`](../assets/json/Map/generation.json):

```json
{
    "enabled": true,
    "mode": "heightmap",
    "heightmap": "Images/Map/biome_map.png",
    "default_tile": "gras",
    "biomes": [
        { "name": "see",    "height_min": 0.00, "height_max": 0.30, "tile": "wasser" },
        { "name": "strand", "height_min": 0.30, "height_max": 0.42, "tile": "sand"   },
        { "name": "ebene",  "height_min": 0.42, "height_max": 0.68, "tile": "gras"   },
        { "name": "berg",   "height_min": 0.68, "height_max": 1.00, "tile": "stein"  }
    ]
}
```

The biome map pixel brightness maps directly to `height_min`/`height_max`.
Texture variation inside each biome is handled automatically by noise — no second image needed.

---

## Step 2 — Adding a texture map

Add `texture_heightmap` to use a second PNG for tile variation within biomes.
Add `tiles` inside each biome to define which tile appears at which texture value:

```json
{
    "enabled": true,
    "mode": "heightmap",
    "heightmap":         "Images/Map/biome_map.png",
    "texture_heightmap": "Images/Map/texture_map.png",
    "default_tile": "gras",
    "biomes": [
        {
            "name": "ebene",
            "height_min": 0.42,
            "height_max": 0.68,
            "tile": "gras",
            "tiles": [
                { "tile": "gras_dunkel", "height_min": 0.00, "height_max": 0.35 },
                { "tile": "gras",        "height_min": 0.35, "height_max": 0.70 },
                { "tile": "gras_hell",   "height_min": 0.70, "height_max": 1.00 }
            ]
        },
        {
            "name": "berg",
            "height_min": 0.68,
            "height_max": 1.00,
            "tile": "stein"
        }
    ]
}
```

The `tiles` list uses the **texture map** value — same range syntax as biomes.
Biomes without a `tiles` list just use their `tile` field.

---

## Texture variation without a PNG

If you don't supply `texture_heightmap`, the engine generates texture noise automatically.
Tune it with these optional fields:

```json
{
    "tex_noise_scale":  0.08,
    "tex_octaves":      2,
    "tex_persistence":  0.5
}
```

| Field | Effect |
|---|---|
| `tex_noise_scale` | Patch size. `0.04` = large patches, `0.15` = fine grain |
| `tex_octaves` | Detail layers. `1`–`2` is usually enough |
| `tex_persistence` | Contrast of the variation |

---

## Tiling and image reading

- Pixel at world tile `(x, y)` → `image[(y % height) * width + (x % width)]`
- A 128×128 image repeats every 128 tiles in both directions
- To avoid a visible seam: blur the image edges, or make the image large enough that the repeat isn't noticeable

---

## Fallback behavior

| Situation | Result |
|---|---|
| `heightmap` file not found | Warning in console, falls back to `"noise"` mode |
| `texture_heightmap` file not found | Warning in console, uses auto noise for texture |
| `tiles` entry with unknown tile ID | Silently uses the biome's `tile` fallback |

```
[WorldGen] Biome-Heightmap fehlt – Fallback auf Noise.
[WorldGen] Heightmap nicht gefunden: assets/Images/Map/... – Fallback auf Noise.
```

---

## Tips

- **Paint large shapes** for the biome map (low detail, soft edges), **fine detail** for the texture map.
- **Test ranges in noise mode first** — tune `height_min`/`height_max` values without repainting anything.
- **High contrast** in the biome map = sharp biome borders. **Soft gradients** = smooth transitions.
- To regenerate the world with changed images, delete `assets/json/Map/chunks/`.
- For object placement (trees, rocks) inside biomes, see [`biomes.md`](biomes.md).
