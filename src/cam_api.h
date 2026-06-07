#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  cam_api.h  –  Kamera-Steuerung für Modder
//
//  Kamera-Modi:
//      setCameraFollowPlayer()                  Standard
//      setCameraFollowNpc("npc_0")              Folgt einem NPC
//      setCameraFree()                          Kamera bleibt wo sie ist
//      setCameraFixed(x, y)                     Springt zu fester Position
//      cameraMoveToSmooth(x, y, 2.0f)           Gleitet in 2 Sekunden zum Ziel
//      cameraMoveToInstant(x, y)                Springt sofort
//
//  Effekte:
//      cameraShake(8.0f, 0.4f)                  Schütteln: Stärke, Dauer
//      setCameraZoom(0.8f)                       Zoom-Faktor (1.0 = normal)
//
//  Feineinstellung:
//      setCameraLerp(0.1f)                      Weich folgen (0=sofort, 0.1=weich)
//      setCameraDeadzone(20.0f)                 Spieler kann 20px frei laufen
//      setCameraOffset(0.0f, -30.0f)            Kamera leicht oberhalb zentrieren
//      setCameraBounds(0, 0, 1000, 1000)        Kamera verlässt diesen Bereich nie
//      clearCameraBounds()
// ═══════════════════════════════════════════════════════════════════════════════

#include "Cam.h"
#include <string>

// ── Modi ──────────────────────────────────────────────────────────────────────

inline void setCameraFollowPlayer() {
    g_cameraState.mode = CAM_FOLLOW_PLAYER;
}

inline void setCameraFollowNpc(const std::string& npcInstanceId) {
    g_cameraState.mode        = CAM_FOLLOW_NPC;
    g_cameraState.followNpcId = npcInstanceId;
}

inline void setCameraFree() {
    g_cameraState.mode    = CAM_FREE;
    g_cameraState.targetX = camera.target.x - g_cameraState.offsetX;
    g_cameraState.targetY = camera.target.y - g_cameraState.offsetY;
}

inline void setCameraFixed(float x, float y) {
    g_cameraState.mode    = CAM_FIXED;
    g_cameraState.targetX = x;
    g_cameraState.targetY = y;
}

inline void cameraMoveToSmooth(float x, float y, float duration) {
    g_cameraState.mode             = CAM_CINEMATIC;
    g_cameraState.cinematicStartX  = camera.target.x - g_cameraState.offsetX;
    g_cameraState.cinematicStartY  = camera.target.y - g_cameraState.offsetY;
    g_cameraState.cinematicEndX    = x;
    g_cameraState.cinematicEndY    = y;
    g_cameraState.cinematicDur     = duration;
    g_cameraState.cinematicTimer   = 0.0f;
    g_cameraState.cinematicActive  = true;
    g_cameraState.targetX          = x;
    g_cameraState.targetY          = y;
}

inline void cameraMoveToInstant(float x, float y) {
    g_cameraState.mode    = CAM_FIXED;
    g_cameraState.targetX = x;
    g_cameraState.targetY = y;
    camera.target.x       = x + g_cameraState.offsetX;
    camera.target.y       = y + g_cameraState.offsetY;
}

// ── Effekte ───────────────────────────────────────────────────────────────────

inline void cameraShake(float intensity, float duration) {
    g_cameraState.shakeIntensity = intensity;
    g_cameraState.shakeDuration  = duration;
    g_cameraState.shakeTimer     = 0.0f;
}

// ── Feineinstellung ───────────────────────────────────────────────────────────

inline void setCameraLerp(float lerp) {
    g_cameraState.lerp = (lerp < 0.0f) ? 0.0f : (lerp > 1.0f ? 1.0f : lerp);
}

inline void setCameraDeadzone(float radius) {
    g_cameraState.deadzone = radius;
}

inline void setCameraOffset(float x, float y) {
    g_cameraState.offsetX = x;
    g_cameraState.offsetY = y;
}

inline void setCameraBounds(float x, float y, float w, float h) {
    g_cameraState.boundsActive = true;
    g_cameraState.boundsX = x;
    g_cameraState.boundsY = y;
    g_cameraState.boundsW = w;
    g_cameraState.boundsH = h;
}

inline void clearCameraBounds() {
    g_cameraState.boundsActive = false;
}
