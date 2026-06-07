#include "Cam.h"
#include "NPC.h"
#include "player.h"
#include <cmath>

Camera2D    camera      = {0};
float       cameraSpeed = 300.0f;
CameraState g_cameraState;

static const float BASE_WIDTH  = 1280.0f;
static const float BASE_HEIGHT =  720.0f;
static const float BASE_ZOOM   =    4.0f;

extern player* g_player;

void initCamera() {
    camera.offset.x = GetScreenWidth()  / 2.0f;
    camera.offset.y = GetScreenHeight() / 2.0f;
    camera.target.x = 0.0f;
    camera.target.y = 0.0f;
    camera.rotation = 0.0f;
    camera.zoom     = BASE_ZOOM;
}

void updateCamera() {
    // Offset + Zoom
    camera.offset.x = GetScreenWidth()  / 2.0f;
    camera.offset.y = GetScreenHeight() / 2.0f;
    float scaleX    = GetScreenWidth()  / BASE_WIDTH;
    float scaleY    = GetScreenHeight() / BASE_HEIGHT;
    camera.zoom     = BASE_ZOOM * ((scaleX < scaleY) ? scaleX : scaleY);

    CameraState& s   = g_cameraState;
    float desiredX   = camera.target.x;
    float desiredY   = camera.target.y;
    float dt         = GetFrameTime();

    switch (s.mode) {

        case CAM_FOLLOW_PLAYER:
            if (g_player) {
                Vector2 pos = g_player->Get_position();
                desiredX = pos.x + s.offsetX;
                desiredY = pos.y + s.offsetY;
            }
            break;

        case CAM_FOLLOW_NPC: {
            PlacedNPC* npc = g_npcManager.getNPC(s.followNpcId);
            if (npc) {
                desiredX = npc->posX + s.offsetX;
                desiredY = npc->posY + s.offsetY;
            }
            break;
        }

        case CAM_FREE:
        case CAM_FIXED:
            desiredX = s.targetX + s.offsetX;
            desiredY = s.targetY + s.offsetY;
            break;

        case CAM_CINEMATIC:
            if (s.cinematicActive && s.cinematicDur > 0.0f) {
                s.cinematicTimer += dt;
                float t = s.cinematicTimer / s.cinematicDur;
                if (t >= 1.0f) { t = 1.0f; s.cinematicActive = false; }
                float st = t * t * (3.0f - 2.0f * t); // smooth step
                desiredX = s.cinematicStartX + (s.cinematicEndX - s.cinematicStartX) * st + s.offsetX;
                desiredY = s.cinematicStartY + (s.cinematicEndY - s.cinematicStartY) * st + s.offsetY;
                s.targetX = s.cinematicEndX;
                s.targetY = s.cinematicEndY;
            } else {
                desiredX = s.targetX + s.offsetX;
                desiredY = s.targetY + s.offsetY;
            }
            break;
    }

    // Deadzone
    if (s.deadzone > 0.0f) {
        float dx   = desiredX - camera.target.x;
        float dy   = desiredY - camera.target.y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist < s.deadzone) {
            desiredX = camera.target.x;
            desiredY = camera.target.y;
        }
    }

    // Lerp
    float l = (s.lerp < 0.0f) ? 0.0f : (s.lerp > 1.0f ? 1.0f : s.lerp);
    camera.target.x += (desiredX - camera.target.x) * l;
    camera.target.y += (desiredY - camera.target.y) * l;

    // Bounds
    if (s.boundsActive) {
        float minX = s.boundsX;
        float minY = s.boundsY;
        float maxX = s.boundsX + s.boundsW;
        float maxY = s.boundsY + s.boundsH;
        if (camera.target.x < minX) camera.target.x = minX;
        if (camera.target.y < minY) camera.target.y = minY;
        if (camera.target.x > maxX) camera.target.x = maxX;
        if (camera.target.y > maxY) camera.target.y = maxY;
    }

    // Schütteln
    if (s.shakeTimer < s.shakeDuration) {
        s.shakeTimer += dt;
        float progress  = s.shakeTimer / s.shakeDuration;
        float intensity = s.shakeIntensity * (1.0f - progress);
        camera.target.x += (float)GetRandomValue(-100, 100) / 100.0f * intensity;
        camera.target.y += (float)GetRandomValue(-100, 100) / 100.0f * intensity;
    }
}
