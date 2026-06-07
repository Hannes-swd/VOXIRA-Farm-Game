#pragma once
#include "Items.h"
#include <functional>
#include <iostream>

// Basis-Klasse fuer Items (optional fuer OOP-Erweiterung)
class BaseItem {
public:
    virtual ~BaseItem() = default;
    virtual void onClick()   {}
    virtual void onInventory(){}
    virtual void onHand()    {}
    virtual void onUpdate()  {}
    virtual void zeichnen(int x, int y, Texture2D textur) {
        if (textur.id != 0) {
            DrawTexture(textur, x, y, WHITE);
        }
    }
};

// Makro fuer manuelle Item-Registrierung (optional)
// Verwendung: REGISTER_ITEM_CALLBACKS(id, clickFn, inventoryFn, handFn)
#define REGISTER_ITEM_CALLBACKS(id, clickFn, inventoryFn, handFn) \
    do { \
        Item* _item = g_itemManager.getItem(id); \
        if (_item) { \
            _item->onClick   = clickFn; \
            _item->onInventory= inventoryFn; \
            _item->onHand    = handFn; \
        } \
    } while(0)