#include "Particles.h"
#include <algorithm>
#include <cmath>

void ParticleManager::registerTemplate(const std::string& id, ParticleTemplate tmpl) {
    templates[id] = std::move(tmpl);
}

void ParticleManager::spawn(float x, float y, const SpawnParams& p) {
    for (int i = 0; i < p.count; i++) {
        float angle;
        if (p.spread >= 360.0f) {
            angle = (float)GetRandomValue(0, 36000) / 100.0f;
        } else {
            float half = p.spread * 0.5f;
            int   iMin = (int)(-half * 10.0f);
            int   iMax = (int)( half * 10.0f);
            if (iMax <= iMin) iMax = iMin + 1;
            angle = p.dirAngle + (float)GetRandomValue(iMin, iMax) / 10.0f;
        }
        float rad = angle * DEG2RAD;
        float spd = p.speed * ((float)GetRandomValue(60, 140) / 100.0f);

        Particle pt;
        pt.x            = x;
        pt.y            = y;
        pt.vx           = cosf(rad) * spd;
        pt.vy           = sinf(rad) * spd;
        pt.life         = p.lifetime;
        pt.maxLife      = p.lifetime;
        pt.color        = p.color;
        pt.size         = p.size;
        pt.gravity      = p.gravity;
        pt.fade         = p.fade;
        pt.texture      = p.texture;
        pt.rotation     = p.rotation;
        // Zufälliges Vorzeichen für Rotation damit Partikel sich unterschiedlich drehen
        float sign      = (GetRandomValue(0, 1) == 0) ? 1.0f : -1.0f;
        pt.rotationSpeed = p.rotationSpeed * sign;
        particles.push_back(pt);
    }
}

void ParticleManager::spawnTemplate(const std::string& templateId, float x, float y, int count) {
    auto it = templates.find(templateId);
    if (it == templates.end()) return;
    ParticleTemplate& tmpl = it->second;
    for (int i = 0; i < count; i++) {
        Particle pt;
        pt.x = x; pt.y = y;
        if (tmpl.onSpawn) tmpl.onSpawn(pt);
        particles.push_back(pt);
    }
}

ParticleEmitter* ParticleManager::createEmitter(const std::string& templateId,
                                                 float x, float y, float rate) {
    ParticleEmitter e;
    e.templateId = templateId;
    e.x     = x;
    e.y     = y;
    e.rate  = rate;
    e.timer = 0.0f;
    e.active = true;
    emitters.push_back(e);
    return &emitters.back();
}

void ParticleManager::destroyEmitter(ParticleEmitter* emitter) {
    if (!emitter) return;
    emitter->active = false;
    emitters.erase(
        std::remove_if(emitters.begin(), emitters.end(),
            [](const ParticleEmitter& e){ return !e.active; }),
        emitters.end());
}

void ParticleManager::update(float dt) {
    for (auto& e : emitters) {
        if (!e.active) continue;
        e.timer += dt;
        float interval = (e.rate > 0.0f) ? (1.0f / e.rate) : 1.0f;
        while (e.timer >= interval) {
            e.timer -= interval;
            spawnTemplate(e.templateId, e.x, e.y, 1);
        }
    }
    emitters.erase(
        std::remove_if(emitters.begin(), emitters.end(),
            [](const ParticleEmitter& e){ return !e.active; }),
        emitters.end());

    for (auto& p : particles) {
        p.life     -= dt;
        p.vy       += p.gravity * dt;
        p.x        += p.vx * dt;
        p.y        += p.vy * dt;
        p.rotation += p.rotationSpeed * dt;
    }

    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p){ return p.life <= 0.0f; }),
        particles.end());
}

void ParticleManager::draw() {
    for (auto& p : particles) {
        Color c = p.color;
        if (p.fade) {
            c.a = (unsigned char)(255.0f * p.lifeRatio());
        }

        if (p.texture && p.texture->id != 0) {
            // Textur-Partikel: als rotiertes Sprite zeichnen
            float half = p.size;
            Rectangle src = { 0, 0, (float)p.texture->width, (float)p.texture->height };
            Rectangle dst = { p.x, p.y, half * 2.0f, half * 2.0f };
            Vector2   origin = { half, half }; // Mittelpunkt als Drehpunkt
            DrawTexturePro(*p.texture, src, dst, origin, p.rotation, c);
        } else {
            DrawCircleV({ p.x, p.y }, p.size, c);
        }
    }
}
