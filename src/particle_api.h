#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  particle_api.h  –  Partikel-System für Modder
//
//  Schnell-Spawn:
//      SpawnParams p;
//      p.count   = 20;
//      p.color   = RED;
//      p.speed   = 150.0f;
//      p.gravity = 200.0f;
//      p.fade    = true;
//      spawnParticles(worldX, worldY, p);
//
//  Template-Spawn:
//      PARTICLE_BEGIN("funken", funken)
//          void onSpawn() {
//              p.color = ORANGE;
//              p.size  = randomFloat(2.0f, 5.0f);
//              p.speed = 120.0f;  // wird direkt in vy → vx aufgelöst
//          }
//          void onUpdate(float dt) { }
//      PARTICLE_END("funken")
//
//      spawnParticleTemplate("funken", x, y, 15);
//
//  Dauerhafter Emitter:
//      auto* emitter = createParticleEmitter("funken", x, y, 20.0f); // 20/s
//      destroyParticleEmitter(emitter);
//
//  Positionen sind in World-Space (Pixel), da Partikel innerhalb BeginMode2D gezeichnet werden.
// ═══════════════════════════════════════════════════════════════════════════════

#include "Particles.h"
#include "Items.h"
#include <string>
#include <functional>

// ── Hilfsfunktionen ───────────────────────────────────────────────────────────

// Gibt einen Pointer auf die Textur eines Items zurück.
// Nutzbar in SpawnParams::texture oder in onSpawn() als p.texture = ...
// Beispiel: p.texture = getItemTexture("grasItem");
inline Texture2D* getItemTexture(const std::string& itemId) {
    Item* item = g_itemManager.getItem(itemId);
    return (item && item->textur.id != 0) ? &item->textur : nullptr;
}

inline float randomFloat(float minVal, float maxVal) {
    if (maxVal <= minVal) return minVal;
    int range = (int)((maxVal - minVal) * 1000.0f);
    if (range <= 0) range = 1;
    return minVal + (float)GetRandomValue(0, range) / 1000.0f;
}

// ── Freie Spawn-Funktionen ────────────────────────────────────────────────────

inline void spawnParticles(float x, float y, const SpawnParams& params) {
    g_particleManager.spawn(x, y, params);
}

inline void spawnParticleTemplate(const std::string& templateId, float x, float y, int count = 10) {
    g_particleManager.spawnTemplate(templateId, x, y, count);
}

inline ParticleEmitter* createParticleEmitter(const std::string& templateId,
                                               float x, float y, float rate = 10.0f) {
    return g_particleManager.createEmitter(templateId, x, y, rate);
}

inline void destroyParticleEmitter(ParticleEmitter* emitter) {
    g_particleManager.destroyEmitter(emitter);
}

// ── Auto-Registrar ────────────────────────────────────────────────────────────

struct _ParticleAutoRegistrar {
    _ParticleAutoRegistrar(const char*                    ptId,
                            std::function<void(Particle&)> fnSpawn,
                            std::function<void(Particle&, float)> fnUpdate)
    {
        ParticleTemplate t;
        t.id       = ptId;
        t.onSpawn  = fnSpawn;
        t.onUpdate = fnUpdate;
        getParticleManager().registerTemplate(ptId, std::move(t));
    }
};

// ── PARTICLE_BEGIN / PARTICLE_END ─────────────────────────────────────────────
//
//  tag = gültiger C++-Bezeichner, z.B. funken, blut, staub
//  id  = Template-String, z.B. "funken"
//
//  'p' (Particle&) ist in onSpawn und onUpdate direkt verfügbar.
//  onSpawn:        Partikel-Eigenschaften initialisieren
//  onUpdate(dt):   Jedes Frame aufgerufen (für Sondereffekte)

#define PARTICLE_BEGIN(ptId, tag)                                          \
    namespace _particle_impl_##tag {                                       \
        static struct _Reg {                                               \
            _Reg() {                                                       \
                struct _Impl {                                             \
                    Particle& p;                                           \
                    explicit _Impl(Particle& _p) : p(_p) {}

#define PARTICLE_END(ptId)                                                 \
                };                                                         \
                static _ParticleAutoRegistrar _r(                         \
                    ptId,                                                  \
                    [](Particle& _p)         { _Impl(_p).onSpawn(); },    \
                    [](Particle& _p, float _dt){ _Impl(_p).onUpdate(_dt); } \
                );                                                         \
            }                                                              \
        } _reg;                                                            \
    }
