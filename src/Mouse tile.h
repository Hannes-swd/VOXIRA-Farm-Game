#pragma once
#include "raylib.h"
#include "Cam.h"
#include "player.h"
#include "Items.h"


inline Vector2 getTileMousePos(int tileSize = 32) {
    // Mausposition im Screen-Space → World-Space (berücksichtigt Zoom + Offset)
    Vector2 screenPos = GetMousePosition();
    Vector2 worldPos  = GetScreenToWorld2D(screenPos, camera);

    // World-Space → Tile-Koordinaten (floor damit negative Koordinaten stimmen)
    Vector2 tilePos;
    tilePos.x = floorf(worldPos.x / (float)tileSize);
    tilePos.y = floorf(worldPos.y / (float)tileSize);
    return tilePos;
}

// ── Baumodus-Raster und Platzierungs-Vorschau ─────────────────────────────────
// Zeichnet das Raster und den Platzierungs-Rahmen wenn der Baumodus aktiv ist.
// boundsMin/Max: erlaubter Tile-Bereich (inklusiv). Standardwerte = unbegrenzt.
// Muss im BeginMode2D / EndMode2D Block aufgerufen werden!
inline void drawBuildModeGrid(const player& p, int tileSize = 32, int sichtRadius = 20,
                               int boundsMinX = -100000, int boundsMinY = -100000,
                               int boundsMaxX =  100000, int boundsMaxY =  100000)
{
    if (!p.isBuildMode()) return;

    Vector2 screenPos = GetMousePosition();
    Vector2 worldPos  = GetScreenToWorld2D(screenPos, camera);

    // Tile under the mouse (integer, floor for negative coordinates)
    int mausTileX = (int)floorf(worldPos.x / (float)tileSize);
    int mausTileY = (int)floorf(worldPos.y / (float)tileSize);

    // Spielerposition als Mittelpunkt für das sichtbare Raster
    Vector2 playerPos  = p.Get_position();
    int playerTileX    = (int)floorf(playerPos.x / (float)tileSize);
    int playerTileY    = (int)floorf(playerPos.y / (float)tileSize);

    // ── Rasterlinien im sichtbaren Bereich (nur innerhalb der Grenzen) ─────────
    Color rasterFarbe = { 100, 140, 255, 45 };
    for (int dy = -sichtRadius; dy <= sichtRadius; dy++) {
        for (int dx = -sichtRadius; dx <= sichtRadius; dx++) {
            int tx = playerTileX + dx;
            int ty = playerTileY + dy;
            if (tx < boundsMinX || tx > boundsMaxX || ty < boundsMinY || ty > boundsMaxY)
                continue;
            DrawRectangleLinesEx(
                { (float)(tx * tileSize), (float)(ty * tileSize),
                  (float)tileSize,        (float)tileSize },
                0.5f, rasterFarbe
            );
        }
    }

    // ── Platzierungs-Vorschau für das aktive Item ──────────────────────────────
    // Nur anzeigen wenn Maus innerhalb der erlaubten Grenzen
    if (mausTileX < boundsMinX || mausTileX > boundsMaxX ||
        mausTileY < boundsMinY || mausTileY > boundsMaxY)
        return;

    Item* hand = p.getHandItem();

    int previewW = 1;
    int previewH = 1;

    float px = (float)(mausTileX * tileSize);
    float py = (float)(mausTileY * tileSize);
    float pw = (float)(previewW  * tileSize);
    float ph = (float)(previewH  * tileSize);

    if (hand && hand->textur.id != 0) {
        float scaleX = pw / (float)hand->textur.width;
        float scaleY = ph / (float)hand->textur.height;
        float scale  = (scaleX < scaleY) ? scaleX : scaleY;
        DrawTextureEx(hand->textur, { px, py }, 0.0f, scale, { 255, 255, 255, 160 });
    } else {
        DrawRectangle((int)px, (int)py, (int)pw, (int)ph, { 80, 140, 255, 60 });
    }

    DrawRectangleLinesEx({ px, py, pw, ph }, 2.0f, { 80, 200, 255, 220 });

    Vector2 labelScreen = GetWorldToScreen2D({ px, py }, camera);
    if (labelScreen.x > 0 && labelScreen.x < GetScreenWidth() &&
        labelScreen.y > 0 && labelScreen.y < GetScreenHeight())
    {
        char koordinaten[32];
        snprintf(koordinaten, sizeof(koordinaten), "%d,%d", mausTileX, mausTileY);
        DrawText(koordinaten, (int)px + 2, (int)py + 2, 6, { 200, 230, 255, 200 });
    }
}