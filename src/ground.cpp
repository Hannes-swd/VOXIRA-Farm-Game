// ground.cpp
#include "ground.h"
#include <iostream>
#include <fstream>

// Asset path helper (defined in main.cpp)
extern std::string assetPath(const std::string& relativ);

// Returns a relative assets/ path, regardless of whether the JSON stored
// an absolute or relative path.
// Beispiel:  "C:/Users/foo/proj/assets/textures/gras.png"  →  "textures/gras.png"
//            "assets/textures/gras.png"                    →  "textures/gras.png"
//            "textures/gras.png"                           →  "textures/gras.png"
static std::string normalizePath(const std::string& rawPath) {
    // Trennzeichen fuer assets/ suchen (vorwaerts und rueckwaerts Slash)
    const std::string MARKER = "assets/";
    // Plattformunabhaengig: beide Slash-Varianten pruefen
    for (const std::string& sep : { "assets/", "assets\\" }) {
        size_t pos = rawPath.rfind(sep);
        if (pos != std::string::npos) {
            std::string rest = rawPath.substr(pos + sep.size());
            // Backslashes → Forwardslashes
            for (char& c : rest) if (c == '\\') c = '/';
            return rest;
        }
    }
    // No assets/ marker found → return path unchanged
    // (backslashes normalisieren)
    std::string result = rawPath;
    for (char& c : result) if (c == '\\') c = '/';
    return result;
}

void GroundDatabase::load(const std::string& datei) {
    std::ifstream f(datei.c_str());
    if (f.is_open()) {
        try {
            f >> allGroundTypes;
            std::cout << "Geladen: " << datei << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[Boden] JSON Fehler in " << datei << ": " << e.what() << std::endl;
        }
        f.close();
    } else {
        std::cerr << "Fehler: Kann Datei nicht oeffnen: " << datei << std::endl;
    }
}

bool GroundDatabase::exists(const std::string& typ) const {
    return allGroundTypes.contains(typ);
}

const json& GroundDatabase::get(const std::string& typ) const {
    // .at() wirft bei fehlendem Key – statisches Fallback-Objekt zurueckgeben
    static const json leer = json::object();
    auto it = allGroundTypes.find(typ);
    if (it != allGroundTypes.end()) return it.value();
    return leer;
}

bool GroundDatabase::isWalkable(const std::string& typ) const {
    if (!exists(typ)) return true;
    return get(typ).value("accessible", true);
}

void GroundDatabase::loadTextures() {
    for (auto& [typ, data] : allGroundTypes.items()) {
        std::string rawPath = data.value("textur", "");
        if (rawPath.empty()) continue;

        // Normalize path: absolute path from old device → relative path
        std::string relPath = normalizePath(rawPath);
        std::string fullPath = assetPath(relPath);

        std::cout << "[Boden] Lade Textur: " << fullPath << std::endl;
        Texture2D tex = LoadTexture(fullPath.c_str());
        texturen[typ] = tex;
        if (tex.id == 0)
            std::cerr << "[Boden] FEHLER: Textur nicht geladen: " << fullPath << std::endl;
        else
            std::cout << "[Boden] Textur geladen: " << fullPath << std::endl;
    }
}

void GroundDatabase::unloadTextures() {
    for (auto& [typ, tex] : texturen) {
        UnloadTexture(tex);
        std::cout << "Texture unloaded: " << typ << std::endl;
    }
    texturen.clear();
}