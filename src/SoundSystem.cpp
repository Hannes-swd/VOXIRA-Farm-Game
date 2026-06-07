#include "SoundSystem.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern std::string assetPath(const std::string&);

static std::string normalizePath(const std::string& raw) {
    for (const std::string& sep : {"assets/", "assets\\"}) {
        size_t pos = raw.rfind(sep);
        if (pos != std::string::npos) {
            std::string rest = raw.substr(pos + sep.size());
            for (char& c : rest) if (c == '\\') c = '/';
            return rest;
        }
    }
    std::string r = raw;
    for (char& c : r) if (c == '\\') c = '/';
    return r;
}

Sound SoundManager::buildBeep(float frequency, float duration) const {
    int      sampleRate = 44100;
    int      frameCount = (int)(duration * (float)sampleRate);
    if (frameCount <= 0) frameCount = 1;
    short* data = (short*)malloc((size_t)frameCount * sizeof(short));
    for (int i = 0; i < frameCount; i++) {
        float t   = (float)i / (float)sampleRate;
        float env = 1.0f - (t / duration);
        data[i]   = (short)(sinf(2.0f * 3.14159265f * frequency * t) * 32767.0f * 0.3f * env);
    }
    Wave wave       = {0};
    wave.data        = data;
    wave.frameCount  = (unsigned int)frameCount;
    wave.sampleRate  = (unsigned int)sampleRate;
    wave.sampleSize  = 16;
    wave.channels    = 1;
    Sound s = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return s;
}

void SoundManager::init() {
    InitAudioDevice();
    audioReady = IsAudioDeviceReady();
    if (!audioReady) std::cerr << "[Sound] Audio-Device konnte nicht gestartet werden." << std::endl;
}

void SoundManager::load(const std::string& jsonPath) {
    if (!audioReady) return;
    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        std::cerr << "[Sound] Nicht gefunden: " << jsonPath << std::endl;
        return;
    }
    json data;
    try { f >> data; } catch (const std::exception& e) {
        std::cerr << "[Sound] JSON Fehler: " << e.what() << std::endl;
        return;
    }

    if (data.contains("sounds")) {
        for (auto& [id, v] : data["sounds"].items()) {
            std::string type = v.value("type", "file");
            if (type == "beep") {
                float freq = v.value("freq",     440.0f);
                float dur  = v.value("duration", 0.2f);
                sounds[id] = buildBeep(freq, dur);
                std::cout << "[Sound] Beep: " << id << " (" << freq << " Hz)" << std::endl;
            } else {
                std::string path = assetPath(normalizePath(v.value("path", "")));
                if (!path.empty()) {
                    sounds[id] = LoadSound(path.c_str());
                    std::cout << "[Sound] Datei: " << id << std::endl;
                }
            }
        }
    }

    if (data.contains("music")) {
        for (auto& [id, v] : data["music"].items()) {
            std::string path = assetPath(normalizePath(v.value("path", "")));
            if (!path.empty()) {
                music[id] = LoadMusicStream(path.c_str());
                std::cout << "[Sound] Musik: " << id << std::endl;
            }
        }
    }
}

void SoundManager::update() {
    if (!audioReady || currentMusicId.empty()) return;
    auto it = music.find(currentMusicId);
    if (it != music.end()) UpdateMusicStream(it->second);
}

void SoundManager::unloadAll() {
    for (auto& [id, s] : sounds) UnloadSound(s);
    for (auto& [id, m] : music)  UnloadMusicStream(m);
    sounds.clear();
    music.clear();
    if (audioReady) CloseAudioDevice();
}

void SoundManager::scanSounds(const std::string& folder) {
    if (!audioReady) return;
    namespace fs = std::filesystem;
    if (!fs::exists(folder)) return;

    static const std::vector<std::string> exts = {".wav", ".ogg", ".mp3", ".flac"};

    for (const auto& entry : fs::recursive_directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        for (char& c : ext) c = (char)tolower((unsigned char)c);

        bool supported = false;
        for (auto& e : exts) if (ext == e) { supported = true; break; }
        if (!supported) continue;

        // Dateiname ohne Endung = ID
        std::string id = entry.path().stem().string();
        if (sounds.count(id)) continue; // bereits geladen → nicht überschreiben

        sounds[id] = LoadSound(entry.path().string().c_str());
        std::cout << "[Sound] Scan: " << id << " (" << entry.path().filename().string() << ")" << std::endl;
    }
}

void SoundManager::scanMusic(const std::string& folder) {
    if (!audioReady) return;
    namespace fs = std::filesystem;
    if (!fs::exists(folder)) return;

    static const std::vector<std::string> exts = {".ogg", ".mp3", ".wav", ".flac", ".xm", ".mod"};

    for (const auto& entry : fs::recursive_directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        for (char& c : ext) c = (char)tolower((unsigned char)c);

        bool supported = false;
        for (auto& e : exts) if (ext == e) { supported = true; break; }
        if (!supported) continue;

        std::string id = entry.path().stem().string();
        if (music.count(id)) continue;

        music[id] = LoadMusicStream(entry.path().string().c_str());
        std::cout << "[Sound] Musik-Scan: " << id << " (" << entry.path().filename().string() << ")" << std::endl;
    }
}

void SoundManager::playSound(const std::string& id) {
    if (!audioReady) return;
    auto it = sounds.find(id);
    if (it != sounds.end()) PlaySound(it->second);
    else std::cerr << "[Sound] Unbekannt: " << id << std::endl;
}

void SoundManager::stopSound(const std::string& id) {
    if (!audioReady) return;
    auto it = sounds.find(id);
    if (it != sounds.end()) StopSound(it->second);
}

void SoundManager::setSoundVolume(const std::string& id, float volume) {
    if (!audioReady) return;
    auto it = sounds.find(id);
    if (it != sounds.end()) SetSoundVolume(it->second, volume);
}

void SoundManager::playMusic(const std::string& id) {
    if (!audioReady) return;
    if (!currentMusicId.empty()) stopMusic();
    auto it = music.find(id);
    if (it != music.end()) {
        PlayMusicStream(it->second);
        currentMusicId = id;
    } else {
        std::cerr << "[Sound] Musik unbekannt: " << id << std::endl;
    }
}

void SoundManager::stopMusic() {
    if (!audioReady || currentMusicId.empty()) return;
    auto it = music.find(currentMusicId);
    if (it != music.end()) StopMusicStream(it->second);
    currentMusicId.clear();
}

void SoundManager::setMusicVolume(float volume) {
    if (!audioReady || currentMusicId.empty()) return;
    auto it = music.find(currentMusicId);
    if (it != music.end()) SetMusicVolume(it->second, volume);
}
