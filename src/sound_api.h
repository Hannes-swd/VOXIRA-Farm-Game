#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  sound_api.h  –  Sound-Funktionen für Modder
//
//  Überall nutzbar: in Items, Buildings, NPCs, Objects, Dimensionen.
//
//  ── Ordner-Import (empfohlen) ─────────────────────────────────────────────
//  Datei in assets/sounds/ legen → automatisch als Sound geladen.
//  Datei in assets/music/  legen → automatisch als Musik geladen.
//  Dateiname ohne Endung = ID:
//    assets/sounds/hit.wav       →  playSound("hit")
//    assets/music/main_theme.ogg →  playMusic("main_theme")
//
//  Unterstützte Formate:
//    Sounds:  .wav  .ogg  .mp3  .flac
//    Musik:   .ogg  .mp3  .wav  .flac  .xm  .mod
//
//  ── JSON-Definition (für prozedurale Beeps) ───────────────────────────────
//  assets/json/sounds.json:
//    {
//      "sounds": {
//        "ding": { "type": "beep", "freq": 880.0, "duration": 0.1 }
//      }
//    }
// ═══════════════════════════════════════════════════════════════════════════════

#include "SoundSystem.h"

inline void playSound(const std::string& id)                     { g_soundManager.playSound(id);          }
inline void stopSound(const std::string& id)                     { g_soundManager.stopSound(id);          }
inline void setSoundVolume(const std::string& id, float volume)  { g_soundManager.setSoundVolume(id, volume); }

inline void playMusic(const std::string& id)                     { g_soundManager.playMusic(id);          }
inline void stopMusic()                                          { g_soundManager.stopMusic();             }
inline void setMusicVolume(float volume)                         { g_soundManager.setMusicVolume(volume); }
