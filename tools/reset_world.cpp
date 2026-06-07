#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <ctime>

using json = nlohmann::json;
namespace fs = std::filesystem;

#ifndef ASSETS_PATH
#define ASSETS_PATH "assets"
#endif

static json readJson(const std::string& path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "Fehler: Kann nicht lesen: " << path << "\n"; return {}; }
    return json::parse(f);
}

static bool writeJson(const std::string& path, const json& j) {
    std::ofstream f(path);
    if (!f) { std::cerr << "Fehler: Kann nicht schreiben: " << path << "\n"; return false; }
    f << j.dump(4);
    return true;
}

static std::string makeTimestamp() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", std::localtime(&t));
    return buf;
}

// Kopiert src/ rekursiv nach dst/, gibt Anzahl kopierter Dateien zurück
static int backupDir(const fs::path& src, const fs::path& dst) {
    int count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(src)) {
        if (!entry.is_regular_file()) continue;
        fs::path rel   = fs::relative(entry.path(), src);
        fs::path target = dst / rel;
        fs::create_directories(target.parent_path());
        fs::copy_file(entry.path(), target, fs::copy_options::overwrite_existing);
        ++count;
    }
    return count;
}

int main() {
    const std::string assets = ASSETS_PATH;

    std::cout << "=== Welt-Reset ===\n\n";

    // ── 1. Backup ins Mülleimer-Verzeichnis ─────────────────────────────────
    std::string timestamp = makeTimestamp();
    fs::path trashRoot = fs::path(assets).parent_path() / "trash" / timestamp;
    fs::create_directories(trashRoot);

    std::cout << "[BACKUP] Kopiere assets/ -> trash/" << timestamp << "/\n";
    int copied = backupDir(fs::path(assets), trashRoot);
    std::cout << "[BACKUP] " << copied << " Dateien gesichert\n\n";

    // ── 2. Welt leeren (nur Startfeld übrig) ────────────────────────────────
    {
        std::string path = assets + "/json/Map/world.json";
        json w = readJson(path);
        w["tiles"]     = json::object();
        w["tiles"]["0,0"] = "gras";
        w["buildings"] = json::object();
        if (writeJson(path, w))
            std::cout << "[OK] world.json     -> Tiles + Gebäude geleert\n";
    }

    // ── 3. Platzierte NPCs löschen ───────────────────────────────────────────
    {
        std::string path = assets + "/json/Map/npcs.json";
        if (writeJson(path, json::object()))
            std::cout << "[OK] Map/npcs.json  -> Alle NPCs entfernt\n";
    }

    // ── 4. Platzierte Objekte löschen ────────────────────────────────────────
    {
        std::string path = assets + "/json/Map/objects.json";
        if (writeJson(path, json::array()))
            std::cout << "[OK] objects.json   -> Alle Objekte entfernt\n";
    }

    // ── 5. Gebäude-Instanzen löschen ─────────────────────────────────────────
    {
        std::string path = assets + "/json/Buildings/houses.json";
        if (writeJson(path, json::object()))
            std::cout << "[OK] houses.json    -> Alle Häuser entfernt\n";
    }

    // ── 6. Item-Definitionen löschen ─────────────────────────────────────────
    {
        std::string path = assets + "/json/items/item.json";
        if (writeJson(path, json::object()))
            std::cout << "[OK] item.json      -> Alle Item-Definitionen entfernt\n";
    }

    // ── 7. NPC-Definitionen löschen ──────────────────────────────────────────
    {
        std::string npcDir = assets + "/json/npcs";
        if (fs::exists(npcDir)) {
            int deleted = 0;
            for (const auto& entry : fs::directory_iterator(npcDir)) {
                if (entry.path().extension() != ".json") continue;
                fs::remove(entry.path());
                ++deleted;
            }
            std::cout << "[OK] npcs/          -> " << deleted << " NPC-Definitionen gelöscht\n";
        }
    }

    // ── 8. Spieler zurücksetzen ──────────────────────────────────────────────
    {
        std::string path = assets + "/json/player/player.json";
        json p = readJson(path);
        p["posX"]      = 0;
        p["posY"]      = 0;
        p["inventory"] = json::array();
        p["buildMode"] = false;
        if (writeJson(path, p))
            std::cout << "[OK] player.json    -> Position (0,0), Inventar geleert\n";
    }

    // ── 9. Dimension-Instanzen löschen ───────────────────────────────────────
    {
        std::string dimDir = assets + "/json/Map/dimensions";
        if (fs::exists(dimDir)) {
            json dimDefs = readJson(assets + "/json/Map/dimensions.json");
            std::set<std::string> baseIds;
            for (auto& [key, _] : dimDefs.items())
                baseIds.insert(key);

            int deleted = 0, cleared = 0;
            for (const auto& entry : fs::directory_iterator(dimDir)) {
                if (entry.path().extension() != ".json") continue;
                std::string stem = entry.path().stem().string();
                if (baseIds.count(stem)) {
                    json d = readJson(entry.path().string());
                    d["tiles"] = json::object();
                    if (writeJson(entry.path().string(), d)) ++cleared;
                } else {
                    fs::remove(entry.path());
                    ++deleted;
                }
            }
            std::cout << "[OK] dimensions/    -> " << cleared
                      << " Basis-Tiles geleert, " << deleted << " Instanzen gelöscht\n";
        }
    }

    // ── Zusammenfassung ──────────────────────────────────────────────────────
    std::cout << "\n=== Fertig! ===\n";
    std::cout << "  Backup:            trash/" << timestamp << "/\n";
    std::cout << "  Welt:              leeres Grasfeld bei (0,0)\n";
    std::cout << "  NPCs:              alle Definitionen + Platzierungen entfernt\n";
    std::cout << "  Items:             alle Definitionen entfernt\n";
    std::cout << "  Objekte:           entfernt\n";
    std::cout << "  Häuser:            entfernt\n";
    std::cout << "  Spieler:           Position (0,0), Inventar leer\n";
    std::cout << "  Innenraeume:       alle leer\n";
    std::cout << "\nDruecke Enter zum Beenden...";
    std::cin.get();
    return 0;
}
