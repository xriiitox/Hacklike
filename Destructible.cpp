#include <stdio.h>
#include "main.hpp"

Destructible::Destructible(float maxHp, float defense, const char *corpseName) :
    maxHp(maxHp),hp(maxHp),defense(defense) {
    if (corpseName == nullptr) {
        this->corpseName = nullptr;
    } else {
        this->corpseName = strdup(corpseName);
    }
}

Destructible::~Destructible() {
    if (corpseName != nullptr) free((char*)corpseName);
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
    if (corpseName != nullptr) free((char*)corpseName); // Free the old corpseName
    this->corpseName = strdup(owner->name);
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
    corpseName=strdup(zip.getString());
}

void Destructible::save(TCODZip &zip) {
    zip.putFloat(maxHp);
    zip.putFloat(hp);
    zip.putFloat(defense);
    zip.putString(corpseName);
}

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char *corpseName) :
    Destructible(maxHp, defense, corpseName) {
}

void MonsterDestructible::save(TCODZip &zip) {
    zip.putInt(MONSTER);
    Destructible::save(zip);
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, const char *corpseName) :
    Destructible(maxHp, defense, corpseName) {
}

void PlayerDestructible::save(TCODZip &zip) {
    zip.putInt(PLAYER);
    Destructible::save(zip);
}

void MonsterDestructible::die(Actor *owner) {
    // transform it into a nasty corpse! it doesn't block, can't be
    // attacked and doesn't move
    engine.gui->message(TCODColor::lightGrey,"%s is dead",owner->name);
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
        case MONSTER: destructible = new MonsterDestructible(0,0,nullptr); break;
        case PLAYER: destructible = new PlayerDestructible(0,0,nullptr); break;
    }

    destructible->load(zip);
    return destructible;
}
