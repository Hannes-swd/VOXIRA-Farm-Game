#pragma once
#include "player.h"

// Zeichnet einen einzelnen Inventar-Slot (intern + extern verwendbar)
void drawSlot(int sx, int sy, int size, const InventorySlot* slot,
                 bool aktiv, bool isDragSource);

// Zeichnet die gesamte Inventar-UI (Hotbar + erweitertes Inventar)
void drawInventory(player& p);