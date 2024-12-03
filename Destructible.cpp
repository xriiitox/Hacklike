#include <stdio.h>
#include <memory>
#include <cstring>
#include "main.hpp"

Destructible::Destructible(float maxHp, float defense, const char *corpseName, int xp) :
    maxHp(maxHp),hp(maxHp),defense(defense),xp(xp) {
    if (corpseName != nullptr) {
        this->corpseName = std::make_unique<char[]>(std::strlen(corpseName) + 1);
        std::strcpy(this->corpseName.get(), corpseName);
    }
}

Destructible::~Destructible() {
}


float Destructible::takeDamage(Actor *owner, float damage) {
    damage -= defense;
    if (damage > 0) {
        hp -= damage;
        if (hp <= 0) {
            die(owner);
        }
    } else {
        damage = 0;
    }
    return damage;
}

void Destructible::die(Actor* owner) {
    // transform the actor into a corpse
    owner->ch = '%';
    owner->col = TCODColor::darkRed;
    owner->name = corpseName.get();
    owner->blocks = false;
    // make sure corpses are drawn before living actors
    engine.sendToBack(owner);
}

float Destructible::heal(float amount) {
    hp += amount;
    if ( hp > maxHp ) {
        amount -= hp-maxHp;
        hp = maxHp;
    }
    return amount;
}

void Destructible::load(TCODZip &zip) {
    maxHp = zip.getFloat();
    hp = zip.getFloat();
    defense = zip.getFloat();
    const char* name = zip.getString();
    corpseName=std::make_unique<char[]>(std::strlen(name)+1);
    std::strcpy(corpseName.get(), name);
}

void Destructible::save(TCODZip &zip) {
    zip.putFloat(maxHp);
    zip.putFloat(hp);
    zip.putFloat(defense);
    zip.putString(corpseName.get());
}

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp) :
    Destructible(maxHp, defense, corpseName, xp) {
}

void MonsterDestructible::save(TCODZip &zip) {
    zip.putInt(MONSTER);
    Destructible::save(zip);
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, const char *corpseName) :
    Destructible(maxHp, defense, corpseName, 0) {
}

void PlayerDestructible::save(TCODZip &zip) {
    zip.putInt(PLAYER);
    Destructible::save(zip);
}

void MonsterDestructible::die(Actor *owner) {
    // transform it into a nasty corpse! it doesn't block, can't be
    // attacked and doesn't move
    engine.gui->message(TCODColor::lightGrey,"%s is dead. You gain %d xp",owner->name, xp);
    engine.player->destructible->xp += xp;
    Destructible::die(owner);
}

void PlayerDestructible::die(Actor *owner) {
    engine.gui->message(TCODColor::red,"You died!");
    Destructible::die(owner);
    engine.gameStatus=Engine::DEFEAT;
}

Destructible *Destructible::create(TCODZip &zip) {
    DestructibleType type = static_cast<DestructibleType>(zip.getInt());
    Destructible *destructible = nullptr;
    switch (type) {
        case MONSTER: destructible = new MonsterDestructible(0,0,nullptr, 0); break;
        case PLAYER: destructible = new PlayerDestructible(0,0,nullptr); break;
    }

    destructible->load(zip);
    return destructible;
}
