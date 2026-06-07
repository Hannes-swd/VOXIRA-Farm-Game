#pragma once
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include "raylib.h"

struct Particle {
    float x = 0, y = 0;
    float vx = 0, vy = 0;
    float life    = 1.0f;
    float maxLife = 1.0f;
    Color color   = WHITE;
    float size    = 3.0f;
    float gravity = 0.0f;
    bool  fade    = true;

    // Textur (optional) – Pointer, NICHT owned by Particle.
    // nullptr = als Kreis zeichnen, gesetzt = als Sprite zeichnen.
    Texture2D* texture      = nullptr;
    float      rotation     = 0.0f;   // Grad
    float      rotationSpeed = 0.0f;  // Grad pro Sekunde

    float lifeRatio() const { return maxLife > 0.0f ? life / maxLife : 0.0f; }
};

struct SpawnParams {
    int   count    = 10;
    Color color    = WHITE;
    float speed    = 100.0f;
    float lifetime = 1.0f;
    float size     = 3.0f;
    float spread   = 360.0f;
    float dirAngle = 0.0f;
    float gravity  = 0.0f;
    bool  fade     = true;

    // Optionale Textur für alle gespawnten Partikel
    Texture2D* texture       = nullptr;
    float      rotation      = 0.0f;   // Start-Rotation in Grad
    float      rotationSpeed = 0.0f;   // Grad pro Sekunde (zufälliges Vorzeichen pro Partikel)
};

struct ParticleTemplate {
    std::string id;
    std::function<void(Particle&)>        onSpawn;
    std::function<void(Particle&, float)> onUpdate;
};

struct ParticleEmitter {
    std::string templateId;
    float x = 0, y = 0;
    float rate  = 10.0f;
    float timer = 0.0f;
    bool  active = true;
};

class ParticleManager {
    std::vector<Particle>         particles;
    std::vector<ParticleEmitter>  emitters;
    std::unordered_map<std::string, ParticleTemplate> templates;

public:
    void registerTemplate(const std::string& id, ParticleTemplate tmpl);

    void spawn(float x, float y, const SpawnParams& params);
    void spawnTemplate(const std::string& templateId, float x, float y, int count = 10);

    ParticleEmitter* createEmitter(const std::string& templateId, float x, float y, float rate = 10.0f);
    void             destroyEmitter(ParticleEmitter* emitter);

    void update(float dt);
    void draw();

    int getCount() const { return (int)particles.size(); }
};

inline ParticleManager& getParticleManager() {
    static ParticleManager instance;
    return instance;
}
#define g_particleManager getParticleManager()
