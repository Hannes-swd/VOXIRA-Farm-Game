# Functions Reference

---

## setBuildMode

```cpp
setBuildMode(bool active)
```

Enables or disables build mode. When active, the engine shows a placement grid and a preview of the item under the cursor.

> Call this inside `onHand()` so the grid shows automatically while the item is held. The engine resets build mode to `false` every frame, so switching items turns it off automatically.

```cpp
void onHand() {
    setBuildMode(true);
}
```

---

## isBuildMode

```cpp
bool isBuildMode()
```

Returns `true` if build mode is currently active.

```cpp
if (isBuildMode()) {
    // build mode is on
}
```

---

## setTile

```cpp
setTile(int x, int y, std::string type)
```

Places a tile on the map at the given tile coordinates.

- `x` — tile column
- `y` — tile row
- `type` — tile type ID (must exist in `ground.json`)

```cpp
setTile(3, 5, "grass");
```

---

## getTileMouse

```cpp
Vector2 getTileMouse()
```

Returns the tile coordinates currently under the mouse cursor as a `Vector2`.

```cpp
Vector2 tile = getTileMouse();
// tile.x = column, tile.y = row
```

---

## IsMouseOnUi

```cpp
bool IsMouseOnUi()
```

Returns `true` if the mouse is currently over a UI element (hotbar or inventory).

> `onClick()` already skips execution automatically when the mouse is over the UI, so you usually don't need to call this manually inside `onClick()`.

```cpp
if (IsMouseOnUi()) {
    // mouse is over the UI
}
```

---

## leftClick

```cpp
bool leftClick()
```

Returns `true` every frame the left mouse button is held down.

```cpp
if (leftClick()) {
    // runs every frame the button is held
}
```

---

## leftClickPressed

```cpp
bool leftClickPressed()
```

Returns `true` only on the first frame the left mouse button is pressed.

```cpp
if (leftClickPressed()) {
    // runs once per click
}
```

---

## rightClick

```cpp
bool rightClick()
```

Returns `true` every frame the right mouse button is held down.

```cpp
if (rightClick()) {
    // ...
}
```

---

## addToInventory

```cpp
g_player->addToInventory(std::string itemId, int amount)
```

Adds an item to the player's inventory. If the player already has the item, the amount is added to the existing stack.

- `itemId` — item ID (must exist in `item.json`)
- `amount` — how many to add

```cpp
g_player->addToInventory("grassItem", 5);
```

---

## removeFromInventory

```cpp
g_player->removeFromInventory(std::string itemId, int amount)
```

Removes an item from the player's inventory. If the count reaches zero the slot is cleared automatically.

- `itemId` — item ID
- `amount` — how many to remove

```cpp
g_player->removeFromInventory("grassItem", 1);
```

---

## hasItem

```cpp
bool g_player->hasItem(std::string itemId)
```

Returns `true` if the player has at least one of the given item in their inventory.

```cpp
if (g_player->hasItem("grassItem")) {
    // player is carrying grass
}
```

---

## getTile

```cpp
std::string world.getTile(int x, int y)
```

Returns the tile type ID at the given coordinates. Returns the default tile type if no tile has been placed there.

```cpp
std::string type = world.getTile(3, 5);
if (type == "grass") {
    // tile is grass
}
```

---

## rightClickPressed

```cpp
bool rightClickPressed()
```

Returns `true` only on the first frame the right mouse button is pressed.

```cpp
if (rightClickPressed()) {
    // runs once per right-click
}
```

---

## setPendingTooltip

```cpp
setPendingTooltip(std::string text)
```

Shows a tooltip near the mouse cursor for this frame. Call it every frame the tooltip should appear (e.g. inside `onHand()` or `onUpdate()`).

```cpp
void onHand() {
    setPendingTooltip("[Click] Place tile");
}
```

---

## Get_position

```cpp
Vector2 g_player->Get_position()
```

Returns the player's current world-space position in pixels as a `Vector2`.

```cpp
Vector2 pos = g_player->Get_position();
// pos.x = world X, pos.y = world Y
```

---

## setPositionF

```cpp
g_player->setPositionF(float x, float y)
```

Teleports the player to the given world-space pixel position.

```cpp
g_player->setPositionF(512.0f, 256.0f);
```

---

## switchDimension

```cpp
switchDimension(std::string id)
```

Teleports the player into a dimension registered in `dimensions.json`. Pass an empty string to return to the main world.

```cpp
switchDimension("house_interior");  // enter a dimension
switchDimension("");                // return to main world
```

> See [`dimension.md`](dimension.md) for how to create dimensions.

---

## leaveCurrentDimension

```cpp
leaveCurrentDimension()
```

Returns the player to the main world from whichever dimension they are currently in. Has no effect if the player is already in the main world.

```cpp
if (IsKeyPressed(KEY_E)) {
    leaveCurrentDimension();
}
```

---

## isInDimension

```cpp
bool isInDimension()
```

Returns `true` if the player is currently inside any dimension.

```cpp
if (isInDimension()) {
    // player is inside a dimension
}
```

---

## getCurrentDimensionId

```cpp
std::string getCurrentDimensionId()
```

Returns the ID of the dimension the player is currently in. Returns an empty string `""` when the player is in the main world.

```cpp
std::string id = getCurrentDimensionId();
if (id == "house_interior") {
    // player is inside the house
}
```

---

## openUI

```cpp
openUI(std::string id)
```

Opens a UI popup registered in `popups.json`. Closes any currently open popup first.

```cpp
openUI("shop_menu");
```

> See [`ui.md`](ui.md) for how to create popups.

---

## closeCurrentUI

```cpp
closeCurrentUI()
```

Closes the currently open UI popup.

```cpp
UI_BUTTON("Close", []() {
    closeCurrentUI();
})
```

---

## isUIOpen

```cpp
bool isUIOpen()
```

Returns `true` if any UI popup is currently open.

```cpp
if (isUIOpen()) {
    // a popup is on screen
}
```

---

## placeBuilding

```cpp
placeBuilding(std::string buildingId, int tileX, int tileY)
```

Places a building at the given tile coordinates. The building ID must exist in `houses.json`.

```cpp
Vector2 t = getTileMouse();
placeBuilding("House", (int)t.x, (int)t.y);
```

---

## placeObject

```cpp
placeObject(std::string objectId, int tileX, int tileY)
```

Places an object at the given tile coordinates. The object ID must exist in `objects.json`.

```cpp
Vector2 t = getTileMouse();
placeObject("truhe", (int)t.x, (int)t.y);
```

---

## removeObject

```cpp
removeObject(std::string instanceId)
```

Removes a placed object by its instance ID. Call `getObjectInstanceId()` inside an object callback to get the current instance's ID.

```cpp
void onInteract() {
    removeObject(getObjectInstanceId());
}
```