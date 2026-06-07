#include "npc_api.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  npc_settings.cpp  –  Globale Engine-Einstellungen für das NPC-System
//
//  Nur Performance-Parameter. Keine Spiel-Logik.
// ═══════════════════════════════════════════════════════════════════════════════

NPC_SETTINGS_BEGIN
    PATHFINDING_ENABLED(true)
    WANDER_INTERVAL(3.0f, 6.0f)
    MAX_PATHFIND_PER_FRAME(3)
NPC_SETTINGS_END
