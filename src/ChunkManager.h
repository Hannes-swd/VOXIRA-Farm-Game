#pragma once
#include <unordered_map>
#include <string>
#include <vector>

static constexpr int CHUNK_SIZE = 16;

struct Chunk {
    int chunkX = 0, chunkY = 0;
    std::string tiles[CHUNK_SIZE][CHUNK_SIZE];
    bool dirty = false;
};

class ChunkManager {
public:
    std::string saveDir;
    std::string defaultTile = "gras";

    void init(const std::string& dir, const std::string& defTile);

    // Lade/Entlade Chunks rund um eine Tile-Position
    void updateAround(int tileX, int tileY, int radius = 2);

    void saveAll();

    // Migration: altes flaches Tile-Format (key "x,y") in Chunks aufteilen
    void importFromFlat(const std::unordered_map<std::string, std::string>& flatTiles);

    std::string getTile(int worldX, int worldY) const;
    void        setTile(int worldX, int worldY, const std::string& type);

    const std::unordered_map<std::string, Chunk>& getLoadedChunks() const { return loaded; }

private:
    std::unordered_map<std::string, Chunk> loaded;

    static std::string chunkKey(int cx, int cy);
    static void toChunkCoords(int wx, int wy, int& cx, int& cy, int& lx, int& ly);

    void loadChunk(int cx, int cy);
    void unloadChunk(int cx, int cy);
    void saveChunk(const Chunk& chunk) const;
};
