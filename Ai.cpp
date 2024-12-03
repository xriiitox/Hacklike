#include <stdio.h>
#include <math.h>
#include "main.hpp"

// how many turns the monster chases the player after losing sight
static const int TRACKING_TURNS = 3;

Ai *Ai::create(TCODZip &zip) {
    AiType type = static_cast<AiType>(zip.getInt());
    Ai *ai = nullptr;
    switch (type) {
        case PLAYER: ai = new PlayerAi(); break;
        case MONSTER: ai = new MonsterAi(); break;
        case CONFUSED_MONSTER: ai = new ConfusedMonsterAi(0, nullptr); break;
    }
    ai -> load(zip);
    return ai;
}

PlayerAi::PlayerAi() : xpLevel(1) {
}

const int LEVEL_UP_BASE=200;
const int LEVEL_UP_FACTOR=150;

int PlayerAi::getNextLevelXp() {
    return LEVEL_UP_BASE + xpLevel*LEVEL_UP_FACTOR;
}



void PlayerAi::update(Actor *owner) {
    int levelUpXp = getNextLevelXp();
    if (owner -> destructible -> xp >= levelUpXp) {
        xpLevel++;
        owner->destructible->xp -= levelUpXp;
        engine.gui->message(TCODColor::yellow, "Your hacking skills grow stronger! You reached level %d", xpLevel);

        engine.gui->menu.clear();
        engine.gui->menu.addItem(Menu::REDUNDANCY, "Redundancy (+20HP)");
        engine.gui->menu.addItem(Menu::SIGNAL, "Signal (+1 attack)");
        engine.gui->menu.addItem(Menu::FIREWALL, "Firewall (+1 defense)");
        Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::PAUSE);

        switch (menuItem) {
            case Menu::REDUNDANCY:
                owner->destructible->maxHp += 20;
                owner->destructible->hp += 20;
                break;
            case Menu::SIGNAL:
                owner->attacker->power += 1;
                break;
            case Menu::FIREWALL:
                owner->destructible->defense += 1;
                break;
            default: break;
        }
    }
    if ( owner->destructible && owner->destructible->isDead() ) {
        return;
    }
    int dx=0,dy=0;
    switch(engine.lastKey.vk) {
        case TCODK_UP : dy=-1; break;
        case TCODK_DOWN : dy=1; break;
        case TCODK_LEFT : dx=-1; break;
        case TCODK_RIGHT : dx=1; break;
        case TCODK_CHAR : handleActionKey(owner, engine.lastKey.c); break;
        default:break;
    }
    if (dx != 0 || dy != 0) {
        engine.gameStatus=Engine::NEW_TURN;
        if (moveOrAttack(owner, owner->x+dx,owner->y+dy)) {
            engine.map->computeFov();
        }
    }
}

bool PlayerAi::moveOrAttack(Actor *owner, int targetx, int targety) {
    if (engine.map->isWall(targetx,targety)) return false;
    // look for living actors to attack
    for (auto actor : engine.actors) {
        if (actor->destructible && !actor->destructible->isDead() && actor->x == targetx && actor->y == targety) {
            owner -> attacker -> attack(owner, actor);
            return false;
        }
    }

    // look for corpses or items
    for (auto actor : engine.actors) {
        bool corpseOrItem = (actor->destructible && actor->destructible->isDead()) || actor->pickable;
        if (corpseOrItem && actor->x == targetx && actor->y == targety) {
            engine.gui->message(TCODColor::lightGrey,"There's a %s here",actor->name);
        }
    }

    owner->x = targetx;
    owner->y = targety;
    return true;
}

void PlayerAi::handleActionKey(Actor *owner, int ascii) {
    switch (ascii) {
        case 'g': //pickup item
        {
            bool found = false;
            for (auto actor : engine.actors) {
                if (actor->pickable && actor->x == owner->x && actor->y == owner->y) {
                    if (actor->pickable->pick(actor,owner)) {
                        found = true;
                        engine.gui->message(TCODColor::lightGrey, "You pick up the %s", actor->name);
                        break;
                    } else if (! found) {
                        found = true;
                        engine.gui->message(TCODColor::red, "Your inventory is full.");
                    }
                }
            }
            if (!found) {
                engine.gui->message(TCODColor::lightGrey, "There's nothing here that you can pick up.");
            }
            engine.gameStatus = Engine::NEW_TURN;
        }
        break;
        case 'i' : // display inventory
        {
            Actor *actor = choseFromInventory(owner);
            if (actor) {
                actor->pickable->use(actor,owner);
                engine.gameStatus = Engine::NEW_TURN;
            }
        }
        break;
        case 'd': // drop item
        {
            Actor *actor = choseFromInventory(owner);
            if (actor) {
                actor->pickable->drop(actor, owner);
                engine.gameStatus = Engine::NEW_TURN;
            }
        }
        break;
        case '.':
            if (engine.lastKey.shift && engine.stairs->x == owner->x && engine.stairs->y == owner->y) {
                engine.nextLevel();
            } else if (engine.lastKey.shift) {
                engine.gui->message(TCODColor::lightGrey, "There are no stairs here.");
            }
        break;
    }
}

Actor *PlayerAi::choseFromInventory(Actor *owner) {
    static const int INVENTORY_WIDTH = 50;
    static const int INVENTORY_HEIGHT = 28;
    static TCODConsole con(INVENTORY_WIDTH, INVENTORY_HEIGHT);

    // display the inventory frame
    con.setDefaultForeground(TCODColor(200,180,50));
    con.printFrame(0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT, true, TCOD_BKGND_DEFAULT, "inventory");

    //display the items with their keyboard shortcut
    con.setDefaultForeground(TCODColor(200,180,50));
    int shortcut = 'a';
    int y = 1;
    for (auto actor : owner->container->inventory) {
        con.print(2,y,"(%c) %s", shortcut, actor->name);
        y++;
        shortcut++;
    }

    //blit the inventory console on the root console
    TCODConsole::blit(&con, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT,
        TCODConsole::root, engine.screenWidth/2 - INVENTORY_WIDTH/2,
        engine.screenHeight/2 - INVENTORY_HEIGHT/2);
    TCODConsole::flush();

    // wait for a key press
    TCOD_key_t key;
    TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, nullptr, true);

    if (key.vk == TCODK_CHAR) {
        int actorIndex = key.c - 'a';
        if ( actorIndex >= 0 && actorIndex < owner->container->inventory.size()) {
            return owner->container->inventory.get(actorIndex);
        }
    }
    return nullptr;
}

void PlayerAi::load(TCODZip &zip) {
}

void PlayerAi::save(TCODZip &zip) {
    zip.putInt(PLAYER);
}


void MonsterAi::update(Actor *owner) {
    if (owner->destructible && owner->destructible->isDead()) {
        return;
    }

    if (engine.map->isInFov(owner->x, owner->y)) {
        // we can see the player, move towards it
        moveCount = TRACKING_TURNS;
    } else {
        moveCount--;
    }
    if (moveCount > 0) {
        moveOrAttack(owner, engine.player->x, engine.player->y);
    }
}

void MonsterAi::moveOrAttack(Actor *owner, int targetx, int targety) {
    int dx = targetx - owner->x;
    int dy = targety - owner->y;
    int stepdx = dx > 0 ? 1 : -1;
    int stepdy = dy > 0 ? 1 : -1;

    float distance = sqrtf(dx*dx+dy*dy);

    if (distance >= 2) {
        dx = (int)(round(dx / distance));
        dy = (int)(round(dy / distance));
        if (engine.map->canWalk(owner->x+dx, owner->y+dy)) {
            owner->x += dx;
            owner->y += dy;
        } else if (engine.map->canWalk(owner->x+stepdx, owner->y)) {
            owner->x += stepdx;
        } else if (engine.map->canWalk(owner->x, owner->y+stepdy)) {
            owner->y += stepdy;
        }
    } else if (owner->attacker) {
        owner -> attacker -> attack(owner,engine.player);
    }
}

void MonsterAi::load(TCODZip &zip) {
    moveCount = zip.getInt();
}

void MonsterAi::save(TCODZip &zip) {
    zip.putInt(MONSTER);
    zip.putInt(moveCount);
}

ConfusedMonsterAi::ConfusedMonsterAi(int nbTurns, Ai *oldAi) : nbTurns(nbTurns), oldAi(oldAi) {
}

void ConfusedMonsterAi::update(Actor *owner) {
    TCODRandom *rng = TCODRandom::getInstance();
    int dx = rng->getInt(-1,1);
    int dy = rng-> getInt(-1,1);

    if (dx != 0 || dy != 0) {
        int destx = owner->x+dx;
        int desty = owner->y+dy;
        if (engine.map->canWalk(destx,desty) ) {
            owner->x = destx;
            owner->y = desty;
        } else {
            Actor *actor = engine.getActor(destx,desty);
            if (actor) {
                owner->attacker->attack(owner, actor);
            }
        }
    }
    nbTurns--;
    if (nbTurns == 0) {
        owner->ai = oldAi;
        delete this;
    }
}

void ConfusedMonsterAi::load(TCODZip &zip) {
    nbTurns = zip.getInt();
    oldAi = Ai::create(zip);
}

void ConfusedMonsterAi::save(TCODZip &zip) {
    zip.putInt(CONFUSED_MONSTER);
    zip.putInt(nbTurns);
    oldAi->save(zip);
}