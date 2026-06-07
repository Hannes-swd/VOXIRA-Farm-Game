#include "raylib.h"
#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "ground.h"
#include "map.h"
#include "Cam.h"
#include "player.h"
#include "inventory.h"
#include "Items.h"
#include "Mouse tile.h"
#include "item api.h"
#include "Buildings.h"
#include "Dimension.h"
#include "UI.h"
#include "clothing.h"
#include "NPC.h"
#include "Object.h"
#include "SoundSystem.h"
#include "Particles.h"
#include "HUD.h"
#include "WorldGen.h"


using json = nlohmann::json;

#ifndef ASSETS_PATH
    #define ASSETS_PATH "assets"
#endif

std::string assetPath(const std::string& relativ) {
    return std::string(ASSETS_PATH) + "/" + relativ;
}

Map world;
player* g_player = nullptr;

int main()
{
    int screenWidth  = 1280;
    int screenHeight = 720;
    int TILE_SIZE    = 32;

    // ── Working Directory auf den Projektordner setzen ────────────────────────
    {
        namespace fs = std::filesystem;
        fs::path assetsDir = fs::path(ASSETS_PATH).parent_path();
        if (!assetsDir.empty() && fs::exists(assetsDir)) {
            ChangeDirectory(assetsDir.string().c_str());
        }
    }

    InitWindow(screenWidth, screenHeight, "Farm Game");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    g_soundManager.init();
    g_worldGen.load(assetPath("json/Map/generation.json"));

    // config.json load – try/catch damit schlechte JSON nicht abort() ausloest
    {
        std::ifstream configFile(assetPath("config.json"));
        if (configFile.is_open()) {
            try {
                json config = json::parse(configFile);
                SetWindowTitle(config.value("title", std::string("2D Game Engine")).c_str());
                SetTargetFPS(config.value("fps", 60));
            } catch (const std::exception& e) {
                std::cerr << "[Config] JSON Fehler: " << e.what() << " – Standardwerte werden verwendet." << std::endl;
            }
        }
    }

    player localPlayer;
    g_player = &localPlayer;
    initCamera();

    g_soundManager.load(assetPath("json/sounds.json"));
    g_soundManager.scanSounds(assetPath("sounds/"));
    g_soundManager.scanMusic(assetPath("music/"));
    g_objectManager.loadTemplates(assetPath("json/objects/objects.json"));
    g_itemManager.scanAndLoadItems();
    g_clothingManager.loadAll();
    g_buildingManager.scanAndLoadBuildings();
    g_dimensionManager.load(assetPath("json/Map/dimensions.json"));
    g_uiManager.load(assetPath("json/ui/popups.json"));

    // ── Map load ─────────────────────────────────────────────────────────────
    {
        namespace fs = std::filesystem;
        const std::string save = assetPath("json/Map/world.json");
        const std::string def  = assetPath("json/Map/welt_default.json");
        if (!fs::exists(save) && fs::exists(def))
            fs::copy_file(def, save);
    }
    world.load(assetPath("json/Map/world.json"));

    loadPlayer(localPlayer);

    GroundDatabase ground;
    ground.load(assetPath("json/Map/ground.json"));
    ground.loadTextures();
    world.init(ground);

    g_npcManager.loadTemplates();
    g_npcManager.load(assetPath("json/Map/npcs.json"));
    g_objectManager.load(assetPath("json/Map/objects.json"));

    float speicherTimer = 0.0f;
    const float SPEICHER_INTERVALL = 30.0f;

    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        bool uiBlocksWorld = g_uiManager.isUIOpen() && g_uiManager.isModal();
        g_uiManager.update();
        {
            Vector2 ppos = localPlayer.Get_position();
            world.updateChunks((int)(ppos.x / TILE_SIZE), (int)(ppos.y / TILE_SIZE));
        }
        if (!uiBlocksWorld) {
            updatePlayer(localPlayer);
            if (!g_dimensionManager.isInDimension()) {
                updateBuildings(world, g_buildingManager, TILE_SIZE);
                g_objectManager.update(delta, TILE_SIZE);
            }
            g_npcManager.update(delta);
            g_dimensionManager.update();
        }
        g_particleManager.update(delta);
        g_hudManager.update();
        g_soundManager.update();
        updateCamera();

        speicherTimer += delta;
        if (speicherTimer >= SPEICHER_INTERVALL) {
            world.save(assetPath("json/Map/world.json"));
            g_npcManager.save(assetPath("json/Map/npcs.json"));
            g_objectManager.save(assetPath("json/Map/objects.json"));
            if (g_dimensionManager.isInDimension()) {
                // Weltposition speichern, nicht die Positions innerhalb der Dimension
                Vector2 dimPos = localPlayer.Get_position();
                localPlayer.setPositionF(g_dimensionManager.getSavedWorldX(),
                                          g_dimensionManager.getSavedWorldY());
                savePlayer(localPlayer);
                localPlayer.setPositionF(dimPos.x, dimPos.y);
            } else {
                savePlayer(localPlayer);
            }
            g_dimensionManager.saveAll(assetPath("json/Map/dimensions/"));
            speicherTimer = 0.0f;
        }

        BeginDrawing();
        if (g_dimensionManager.isInDimension()) {
            DimensionData* dim = g_dimensionManager.getCurrentDimension();
            ClearBackground(dim->backgroundColor);
            BeginMode2D(camera);
                draw_dimension(*dim, ground, TILE_SIZE);
                if (dim->onDraw) dim->onDraw();
                g_npcManager.draw();
                drawBuildModeGrid(localPlayer, TILE_SIZE, 20, 0, 0, dim->width - 1, dim->height - 1);
                drawPlayer(localPlayer);
                g_particleManager.draw();
            EndMode2D();
            {
                std::string tip = consumeTooltip();
                if (!tip.empty()) {
                    Vector2 mouse = GetMousePosition();
                    DrawText(tip.c_str(), (int)mouse.x + 12, (int)mouse.y - 20, 14, BLACK);
                }
            }
        } else {
            ClearBackground(WHITE);
            BeginMode2D(camera);
                draw_ground(world, ground, TILE_SIZE);
                draw_buildings(world, g_buildingManager, TILE_SIZE);
                g_objectManager.draw(TILE_SIZE, false);
                g_npcManager.draw();
                drawBuildModeGrid(localPlayer, TILE_SIZE);
                drawPlayer(localPlayer);
                g_objectManager.draw(TILE_SIZE, true);
                g_particleManager.draw();
            EndMode2D();
            // Tooltip von Building/NPC-onHover zeichnen
            std::string tip = consumeTooltip();
            if (!tip.empty()) {
                Vector2 mouse = GetMousePosition();
                DrawText(tip.c_str(), (int)mouse.x + 12, (int)mouse.y - 20, 14, BLACK);
            }
        }
            drawInventory(localPlayer);
            g_hudManager.draw();
            g_uiManager.draw();
        EndDrawing();

        setBuildMode(false);
    }

    world.save(assetPath("json/Map/world.json"));
    g_npcManager.save(assetPath("json/Map/npcs.json"));
    g_objectManager.save(assetPath("json/Map/objects.json"));
    if (g_dimensionManager.isInDimension()) {
        Vector2 dimPos = localPlayer.Get_position();
        localPlayer.setPositionF(g_dimensionManager.getSavedWorldX(),
                                  g_dimensionManager.getSavedWorldY());
        savePlayer(localPlayer);
        localPlayer.setPositionF(dimPos.x, dimPos.y);
    } else {
        savePlayer(localPlayer);
    }
    g_dimensionManager.saveAll(assetPath("json/Map/dimensions/"));
    g_soundManager.unloadAll();
    ground.unloadTextures();
    CloseWindow();
    return 0;
}