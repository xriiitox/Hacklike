#include <math.h>
#include <cstring>
#include "main.hpp"

Actor::Actor(int x, int y, int ch, const char *name,
    const TCODColor &col) :
    x(x), y(y), ch(ch), name(name),
    col(col), blocks(true), fovOnly(true), attacker(nullptr),
    destructible(nullptr), ai(nullptr),pickable(nullptr), container(nullptr) {
}

Actor::~Actor() {
    if (attacker) delete attacker;
    if (destructible) delete destructible;
    if (ai) delete ai;
    if (pickable) delete pickable;
    if (container) delete container;
}

void Actor::render() const {
    TCODConsole::root->setChar(x,y,ch);
    TCODConsole::root->setCharForeground(x,y,col);
}

void Actor::update() {
    if (ai) ai->update(this);
}

bool Actor::moveOrAttack(int x, int y) {
    if (engine.map->isWall(x,y)) return false;
    for (auto actor : engine.actors) {
        if (actor->x == x && actor->y == y) {
            printf("The %s laughs at your puny efforts to attack him!\n", actor->name);
            return false;
        }
    }
    this->x = x;
    this->y = y;
    return true;
}

float Actor::getDistance(int cx, int cy) const {
    int dx = x-cx;
    int dy = y-cy;
    return sqrtf(dx*dx + dy*dy);
}

void Actor::save(TCODZip &zip) {
    zip.putInt(x);
    zip.putInt(y);
    zip.putInt(ch);
    zip.putColor(&col);
    zip.putString(name);
    zip.putInt(blocks);
    zip.putInt(attacker != nullptr);
    zip.putInt(destructible != nullptr);
    zip.putInt(ai != nullptr);
    zip.putInt(pickable != nullptr);
    zip.putInt(container != nullptr);

    if (attacker) attacker -> save(zip);
    if (destructible) destructible -> save(zip);
    if (ai) ai -> save(zip);
    if (pickable) pickable -> save(zip);
    if (container) container -> save(zip);
}

void Actor::load(TCODZip &zip) {
    x = zip.getInt();
    y = zip.getInt();
    ch = zip.getInt();
    col = zip.getColor();
    name = strdup(zip.getString());
    blocks = zip.getInt();

    bool hasAttacker=zip.getInt();
    bool hasDestructible=zip.getInt();
    bool hasAi=zip.getInt();
    bool hasPickable=zip.getInt();
    bool hasContainer=zip.getInt();

    if (hasAttacker) {
        attacker = new Attacker(0.0f);
        attacker->load(zip);
    }
    if (hasDestructible) {
        destructible = Destructible::create(zip);
    }

    if (hasAi) {
        ai = Ai::create(zip);
    }

    if (hasPickable) {
        pickable = Pickable::create(zip);
    }

    if (hasContainer) {
        container = new Container(0);
        container -> load(zip);
    }
}
