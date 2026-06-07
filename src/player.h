#pragma once
#include <string>
#include <vector>
#include "raylib.h"
#include "Items.h"
#include "direction.h"

struct InventorySlot {
    std::string itemId;
    int amount;
};

class player {
private:
    float posX  = 0.0f;
    float posY  = 0.0f;
    float speed = 100.0f;
    std::vector<InventorySlot> inventory;
    std::string name;

    int  currentSlot  = 0;
    int  dragSlot       = -1;
    bool inventoryOpen  = false;
    bool mouseOnUI      = false;
    bool buildMode       = false;

    direction lastDir = Down;
    std::string bodyId  = "human_base";
    std::string shirtId = "";
    std::string pantsId = "";
    std::string hairId  = "";

    std::vector<std::string> ownedShirts;
    std::vector<std::string> ownedPants;
    std::vector<std::string> ownedHair;

#include "player_ext_vars.h"

public:
    void Set_position(int posx, int posy) {
        posX = (float)posx;
        posY = (float)posy;
    }

    void setPositionF(float x, float y) { posX = x; posY = y; }

    void Move(direction dir, float deltaTime) {
        switch (dir) {
            case Right: posX += speed * deltaTime; break;
            case Left:  posX -= speed * deltaTime; break;
            case Up:    posY -= speed * deltaTime; break;
            case Down:  posY += speed * deltaTime; break;
        }
    }

    float getSpeed() const { return speed; }
    Vector2 Get_position() const { return { posX, posY }; }
    void Change_Name(std::string Name) { name = Name; }
    std::string Get_Name() const { return name; }

    // Slot-Steuerung
    int  getCurrentSlot() const         { return currentSlot; }
    void setCurrentSlot(int s)          {
        int neu = (s < 0) ? 0 : (s > 9 ? 9 : s);
        currentSlot = neu;
        // buildMode wird NICHT hier zurückgesetzt.
        // onHand() des neuen Items setzt ihn korrekt (z.B. Gras → true, Beton → false).
        // Das geschieht automatisch jeden Frame in updatePlayer().
    }
    int  getDragSlot() const              { return dragSlot; }
    void setDragSlot(int s)               { dragSlot = s; }
    bool isInventoryOpen() const          { return inventoryOpen; }
    void toggleInventory()                 { inventoryOpen = !inventoryOpen; }
    bool IsMouseOnUi() const              { return mouseOnUI; }
    void setMouseOnUI(bool b)             { mouseOnUI = b; }
    bool isBuildMode() const               { return buildMode; }
    void setBuildMode(bool b)              { buildMode = b; }
    void toggleBuildMode()                 { buildMode = !buildMode; }

    direction   getLastDir()  const        { return lastDir; }
    void        setLastDir(direction d)    { lastDir = d; }

    const std::string& getBodyId()  const  { return bodyId; }
    const std::string& getShirtId() const  { return shirtId; }
    const std::string& getPantsId() const  { return pantsId; }
    const std::string& getHairId()  const  { return hairId; }
    void setBodyId (const std::string& s)  { bodyId  = s; }
    void setShirtId(const std::string& s)  { shirtId = s; }
    void setPantsId(const std::string& s)  { pantsId = s; }
    void setHairId (const std::string& s)  { hairId  = s; }

    const std::vector<std::string>& getOwnedShirts() const { return ownedShirts; }
    const std::vector<std::string>& getOwnedPants()  const { return ownedPants; }
    const std::vector<std::string>& getOwnedHair()   const { return ownedHair; }

    void addOwnedShirt(const std::string& id) { if (!hasOwnedShirt(id)) ownedShirts.push_back(id); }
    void addOwnedPants(const std::string& id) { if (!hasOwnedPants(id)) ownedPants.push_back(id); }
    void addOwnedHair (const std::string& id) { if (!hasOwnedHair(id))  ownedHair.push_back(id); }

    bool hasOwnedShirt(const std::string& id) const { for (auto& s : ownedShirts) if (s == id) return true; return false; }
    bool hasOwnedPants(const std::string& id) const { for (auto& s : ownedPants)  if (s == id) return true; return false; }
    bool hasOwnedHair (const std::string& id) const { for (auto& s : ownedHair)   if (s == id) return true; return false; }

    // Aktives Item (in der Hand)
    Item* getHandItem() const;

    // Inventar
    void addToInventory(const std::string& itemId, int amount);
    void removeFromInventory(const std::string& itemId, int amount);
    bool hasItem(const std::string& itemId) const;
    const std::vector<InventorySlot>& getInventory() const { return inventory; }
    std::vector<InventorySlot>&       getInventoryMut()    { return inventory; }

    // Slots tauschen
    void swapSlots(int a, int b);

#include "player_ext_methods.h"
};

void loadPlayer(player& p);
void savePlayer(const player& p);
void updatePlayer(player& p);
void drawPlayer(player& p);