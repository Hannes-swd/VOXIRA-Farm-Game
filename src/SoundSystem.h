#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "raylib.h"

class SoundManager {
    std::unordered_map<std::string, Sound> sounds;
    std::unordered_map<std::string, Music> music;
    std::string currentMusicId;
    bool audioReady = false;

    Sound buildBeep(float frequency, float duration) const;

public:
    void init();
    void load(const std::string& jsonPath);

    // Ordner automatisch scannen – Dateiname ohne Endung = Sound-ID
    // z.B. assets/sounds/hit.wav  →  playSound("hit")
    void scanSounds(const std::string& folder);
    // z.B. assets/music/main.ogg  →  playMusic("main")
    void scanMusic(const std::string& folder);

    void update();
    void unloadAll();

    void playSound(const std::string& id);
    void stopSound(const std::string& id);
    void setSoundVolume(const std::string& id, float volume);

    void  playMusic(const std::string& id);
    void  stopMusic();
    void  setMusicVolume(float volume);
    const std::string& getCurrentMusic() const { return currentMusicId; }

    bool isReady() const { return audioReady; }
};

inline SoundManager& getSoundManager() {
    static SoundManager instance;
    return instance;
}
#define g_soundManager getSoundManager()
