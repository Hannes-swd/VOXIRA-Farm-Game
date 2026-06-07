#pragma once
// ─────────────────────────────────────────────────────────────────────────────
//  npc_pathfinding.h  –  A*-Algorithmus (intern, nicht für Modder)
// ─────────────────────────────────────────────────────────────────────────────
#include "raylib.h"
#include "map.h"
#include "ground.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>

struct _AStarNode {
    int x, y;
    float g = 0.0f, f = 0.0f;
    bool operator>(const _AStarNode& o) const { return f > o.f; }
};

inline std::vector<Vector2> astar_findPath(
    Vector2 from, Vector2 to, int tileSize,
    const Map& world, const GroundDatabase& ground,
    int maxNodes = 800)
{
    int sx = (int)floorf(from.x / tileSize);
    int sy = (int)floorf(from.y / tileSize);
    int gx = (int)floorf(to.x   / tileSize);
    int gy = (int)floorf(to.y   / tileSize);

    if (sx == gx && sy == gy) return {};

    auto isWalkable = [&](int tx, int ty) -> bool {
        return ground.isWalkable(world.getTile(tx, ty));
    };

    if (!isWalkable(gx, gy)) return {};

    auto encode = [](int x, int y) -> int64_t {
        return (int64_t)(x + 32768) * 65536LL + (int64_t)(y + 32768);
    };

    std::priority_queue<_AStarNode, std::vector<_AStarNode>, std::greater<_AStarNode>> open;
    std::unordered_map<int64_t, int64_t>  cameFrom;
    std::unordered_map<int64_t, float>    gCost;

    int64_t startEnc = encode(sx, sy);
    int64_t goalEnc  = encode(gx, gy);

    open.push({sx, sy, 0.0f, (float)(abs(sx-gx) + abs(sy-gy))});
    gCost[startEnc] = 0.0f;
    cameFrom[startEnc] = -1;

    const int dx[] = {0, 0, 1, -1};
    const int dy[] = {1, -1, 0, 0};
    int visited = 0;

    while (!open.empty() && visited < maxNodes) {
        _AStarNode cur = open.top(); open.pop();
        ++visited;

        int64_t curEnc = encode(cur.x, cur.y);
        if (curEnc == goalEnc) {
            std::vector<Vector2> path;
            int64_t enc = goalEnc;
            while (enc != startEnc && enc != -1) {
                int tx = (int)(enc / 65536) - 32768;
                int ty = (int)(enc % 65536) - 32768;
                path.push_back({(float)(tx * tileSize + tileSize / 2),
                                (float)(ty * tileSize + tileSize / 2)});
                auto it = cameFrom.find(enc);
                if (it == cameFrom.end()) break;
                enc = it->second;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        float curG = gCost.count(curEnc) ? gCost[curEnc] : 1e9f;

        for (int d = 0; d < 4; ++d) {
            int nx = cur.x + dx[d];
            int ny = cur.y + dy[d];
            if (!isWalkable(nx, ny)) continue;
            int64_t nEnc = encode(nx, ny);
            float newG = curG + 1.0f;
            auto it = gCost.find(nEnc);
            if (it == gCost.end() || newG < it->second) {
                gCost[nEnc]    = newG;
                cameFrom[nEnc] = curEnc;
                float h = (float)(abs(nx - gx) + abs(ny - gy));
                open.push({nx, ny, newG, newG + h});
            }
        }
    }
    return {};
}
