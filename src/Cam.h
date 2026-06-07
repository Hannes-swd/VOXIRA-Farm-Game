#pragma once
#include "raylib.h"
#include <string>

extern Camera2D camera;
extern float    cameraSpeed;

// ── Kamera-Modi ───────────────────────────────────────────────────────────────
// Eigener Name um Konflikt mit raylib::CameraMode zu vermeiden.
enum EngineCameraMode {
    CAM_FOLLOW_PLAYER,  // Standard: Kamera folgt dem Spieler
    CAM_FOLLOW_NPC,     // Kamera folgt einem NPC (per Instance-ID)
    CAM_FREE,           // Kamera bleibt wo sie ist
    CAM_FIXED,          // Kamera steht an fester Position
    CAM_CINEMATIC       // Kamera fährt animiert zu einem Zielpunkt
};

// ── Kamera-Zustand ────────────────────────────────────────────────────────────
struct CameraState {
    EngineCameraMode mode = CAM_FOLLOW_PLAYER;
    float lerp        = 1.0f;    // 1.0 = sofort | 0.1 = weich
    float deadzone    = 0.0f;    // Pixel-Radius in dem Kamera sich nicht bewegt
    float offsetX     = 0.0f;
    float offsetY     = 0.0f;

    // Bounds (optional)
    bool  boundsActive = false;
    float boundsX = 0, boundsY = 0, boundsW = 0, boundsH = 0;

    // Schütteln
    float shakeIntensity = 0.0f;
    float shakeDuration  = 0.0f;
    float shakeTimer     = 0.0f;

    // Festes Ziel (für FREE, FIXED, CINEMATIC)
    float targetX = 0.0f;
    float targetY = 0.0f;

    // Cinematic-Animation
    bool  cinematicActive = false;
    float cinematicStartX = 0.0f, cinematicStartY = 0.0f;
    float cinematicEndX   = 0.0f, cinematicEndY   = 0.0f;
    float cinematicDur    = 0.0f;
    float cinematicTimer  = 0.0f;

    // NPC-Folgen
    std::string followNpcId;
};

extern CameraState g_cameraState;

void initCamera();
void updateCamera();
