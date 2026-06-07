// inventory.cpp
// Inventar-UI: Hotbar, erweitertes Inventar-Grid, Drag & Drop, Tooltip
#include "inventory.h"
#include "Items.h"
#include "raylib.h"
#include <string>

// ── Hilfsfunktion: zeichnet einen einzelnen Slot ──────────────────────────────
void drawSlot(int sx, int sy, int size, const InventorySlot* slot,
                 bool aktiv, bool isDragSource)
{
    Color bg = isDragSource ? Color{80, 80, 40, 220}
             : aktiv        ? Color{80, 80, 30, 240}
             : (slot && !slot->itemId.empty()) ? Color{60, 60, 60, 220}
                                               : Color{40, 40, 40, 180};

    Color border = aktiv        ? Color{255, 215, 0, 255}
                 : isDragSource ? Color{255, 180, 0, 200}
                                : Color{120, 120, 120, 200};

    DrawRectangleRounded(
        { (float)sx, (float)sy, (float)size, (float)size },
        0.15f, 6, bg
    );
    DrawRectangleRoundedLines(
        { (float)sx, (float)sy, (float)size, (float)size },
        0.15f, 6, border
    );

    if (aktiv) {
        DrawRectangleRoundedLines(
            { (float)(sx+2), (float)(sy+2), (float)(size-4), (float)(size-4) },
            0.1f, 4, { 255, 215, 0, 100 }
        );
    }

    if (!slot || slot->itemId.empty()) return;

    Item* item = g_itemManager.getItem(slot->itemId);
    if (item && item->textur.id != 0) {
        int   drawSize = size - 8;
        float scaleX   = (float)drawSize / item->textur.width;
        float scaleY   = (float)drawSize / item->textur.height;
        float scale    = (scaleX < scaleY) ? scaleX : scaleY;
        DrawTextureEx(item->textur, { (float)(sx + 4), (float)(sy + 4) }, 0.0f, scale, WHITE);
    } else {
        int fontSize = 9;
        const char* label = slot->itemId.c_str();
        int tw = MeasureText(label, fontSize);
        DrawText(label, sx + (size - tw) / 2,
                 sy + size / 2 - fontSize / 2, fontSize, LIGHTGRAY);
    }

    if (slot->amount > 1) {
        std::string anzStr = std::to_string(slot->amount);
        int fontSize = 10;
        int tw = MeasureText(anzStr.c_str(), fontSize);
        DrawText(anzStr.c_str(), sx + size - tw - 3 + 1,
                 sy + size - fontSize - 2 + 1, fontSize, BLACK);
        DrawText(anzStr.c_str(), sx + size - tw - 3,
                 sy + size - fontSize - 2,     fontSize, WHITE);
    }
}

// ── Haupt-UI: Hotbar + erweitertes Inventar + Drag & Drop ────────────────────
void drawInventory(player& p) {
    const int SLOT_SIZE    = 48;
    const int SLOT_PADDING = 6;
    const int SLOTS        = 10;
    const int BAR_W        = SLOTS * (SLOT_SIZE + SLOT_PADDING) - SLOT_PADDING;
    const int BAR_H        = SLOT_SIZE + 16;
    const int MARGIN_BOT   = 16;

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    int barX = (sw - BAR_W) / 2 - 8;
    int barY = sh - BAR_H - MARGIN_BOT;

    auto& inv = p.getInventoryMut();

    // ── Detect mouse over UI (before draw calls) ──────────────────────
    Vector2 mouse = GetMousePosition();
    bool mouseOnHotbar = (mouse.x >= barX && mouse.x <= barX + BAR_W + 16 &&
                          mouse.y >= barY && mouse.y <= barY + BAR_H);
    bool mouseOnInvGrid = false;

    // ── Hotbar-Hintergrund ────────────────────────────────────────────────────
    DrawRectangleRounded(
        { (float)barX, (float)barY, (float)(BAR_W + 16), (float)BAR_H },
        0.2f, 8, { 20, 20, 20, 200 }
    );
    DrawRectangleRoundedLines(
        { (float)barX, (float)barY, (float)(BAR_W + 16), (float)BAR_H },
        0.2f, 8, { 180, 180, 180, 220 }
    );

    // ── Hotbar-Slots ──────────────────────────────────────────────────────────
    for (int i = 0; i < SLOTS; i++) {
        int sx = barX + 8 + i * (SLOT_SIZE + SLOT_PADDING);
        int sy = barY + 8;

        bool aktiv        = (i == p.getCurrentSlot());
        bool isDragSource = (i == p.getDragSlot());

        const InventorySlot* slotPtr = (i < (int)inv.size()) ? &inv[i] : nullptr;
        drawSlot(sx, sy, SLOT_SIZE, slotPtr, aktiv, isDragSource);
    }

    // ── Tooltip: Name des aktiven Items ──────────────────────────────────────
    Item* hand = p.getHandItem();
    if (hand) {
        const char* label = hand->name.c_str();
        int fontSize = 12;
        int tw = MeasureText(label, fontSize);
        int tx = (sw - tw) / 2;
        int ty = barY - fontSize - 8;
        DrawText(label, tx + 1, ty + 1, fontSize, BLACK);
        DrawText(label, tx,     ty,     fontSize, { 255, 230, 100, 255 });
    }

    // ── Drag & Drop ───────────────────────────────────────────────────────────
    // mouse bereits oben deklariert

    auto getSlotUnterMaus = [&]() -> int {
        for (int i = 0; i < SLOTS; i++) {
            int sx = barX + 8 + i * (SLOT_SIZE + SLOT_PADDING);
            int sy = barY + 8;
            if (mouse.x >= sx && mouse.x <= sx + SLOT_SIZE &&
                mouse.y >= sy && mouse.y <= sy + SLOT_SIZE)
                return i;
        }
        return -1;
    };

    int slotUnterMaus = getSlotUnterMaus();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (slotUnterMaus >= 0) {
            p.setCurrentSlot(slotUnterMaus);
            if (slotUnterMaus < (int)inv.size() && !inv[slotUnterMaus].itemId.empty()) {
                p.setDragSlot(slotUnterMaus);
            }
            Item* h = p.getHandItem();
            if (h && h->onHand) h->onHand();
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        int drag = p.getDragSlot();
        if (drag >= 0 && slotUnterMaus >= 0 && slotUnterMaus != drag) {
            p.swapSlots(drag, slotUnterMaus);
        }
        p.setDragSlot(-1);
    }

    // ── Drag-Vorschau (Item folgt dem Mauszeiger) ─────────────────────────────
    if (p.getDragSlot() >= 0 && p.getDragSlot() < (int)inv.size()) {
        const InventorySlot& dragged = inv[p.getDragSlot()];
        Item* dItem = g_itemManager.getItem(dragged.itemId);
        if (dItem && dItem->textur.id != 0) {
            int   drawSize = SLOT_SIZE - 8;
            float scaleX   = (float)drawSize / dItem->textur.width;
            float scaleY   = (float)drawSize / dItem->textur.height;
            float scale    = (scaleX < scaleY) ? scaleX : scaleY;
            DrawTextureEx(dItem->textur,
                { mouse.x - drawSize * scale * 0.5f, mouse.y - drawSize * scale * 0.5f },
                0.0f, scale, { 255, 255, 255, 180 });
        }
    }

    // ── Rechtsklick auf Slot → onClick des Items ──────────────────────────────
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && slotUnterMaus >= 0) {
        p.setCurrentSlot(slotUnterMaus);
        Item* clicked = p.getHandItem();
        if (clicked && clicked->onClick) clicked->onClick();
    }

    // ── Erweitertes Inventar-Grid (TAB) ───────────────────────────────────────
    if (p.isInventoryOpen()) {
        const int COLS    = 5;
        const int ROWS    = 4;
        const int WIN_W   = COLS * (SLOT_SIZE + SLOT_PADDING) - SLOT_PADDING + 24;
        const int WIN_H   = ROWS * (SLOT_SIZE + SLOT_PADDING) - SLOT_PADDING + 40;
        int winX = (sw - WIN_W) / 2;
        int winY = (sh - WIN_H) / 2 - 40;

        mouseOnInvGrid = (mouse.x >= winX && mouse.x <= winX + WIN_W &&
                          mouse.y >= winY && mouse.y <= winY + WIN_H);

        DrawRectangleRounded(
            { (float)winX, (float)winY, (float)WIN_W, (float)WIN_H },
            0.1f, 8, { 15, 15, 15, 230 }
        );
        DrawRectangleRoundedLines(
            { (float)winX, (float)winY, (float)WIN_W, (float)WIN_H },
            0.1f, 8, { 200, 200, 200, 200 }
        );

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                int idx = 10 + r * COLS + c;
                int sx  = winX + 12 + c * (SLOT_SIZE + SLOT_PADDING);
                int sy  = winY + 28 + r * (SLOT_SIZE + SLOT_PADDING);
                const InventorySlot* sp = (idx < (int)inv.size()) ? &inv[idx] : nullptr;
                drawSlot(sx, sy, SLOT_SIZE, sp, false, idx == p.getDragSlot());

                if (mouse.x >= sx && mouse.x <= sx + SLOT_SIZE &&
                    mouse.y >= sy && mouse.y <= sy + SLOT_SIZE)
                {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        if (sp && !sp->itemId.empty()) p.setDragSlot(idx);
                    }
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                        int drag = p.getDragSlot();
                        if (drag >= 0 && drag != idx) p.swapSlots(drag, idx);
                        p.setDragSlot(-1);
                    }
                }
            }
        }
    }

    // ── UI-Flag aktualisieren (wird von updatePlayer gelesen) ──────────────────
    p.setMouseOnUI(mouseOnHotbar || mouseOnInvGrid);
}