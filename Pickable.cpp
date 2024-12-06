#include "main.hpp"

bool Pickable::pick(Actor *owner, Actor *wearer) {
    if ( wearer -> container && wearer -> container -> add(owner) ) {
        engine.actors.remove(owner);
        return true;
    }
    return false;
}

bool Pickable::use(Actor *owner, Actor *wearer) {
    if (wearer -> container) {
        wearer -> container -> remove(owner);
        delete owner;
        return true;
    }
    return false;
}

void Pickable::drop(Actor *owner, Actor *wearer) {
    if (wearer->container) {
        wearer->container->remove(owner);
        engine.actors.push(owner);
        owner->x = wearer->x;
        owner->y = wearer->y;
        engine.gui->message(TCODColor::lightGrey, "%s drops a %s.", wearer->name, owner->name);
    }
}

Pickable *Pickable::create(TCODZip &zip) {
    PickableType type = static_cast<PickableType>(zip.getInt());
    Pickable *pickable = nullptr;
    switch (type) {
        case HEALER : pickable = new Healer(0); break;
        case REMOTE_HACK : pickable = new RemoteHack(0,0); break;
        case CONFUSER : pickable = new Confuser(0,0); break;
        case TARGETED_HACK : pickable = new TargetedHack(0,0); break;
    }
    pickable -> load(zip);
    return pickable;
}

// Healer

Healer::Healer(float amount) : amount(amount) {
}

bool Healer::use(Actor *owner, Actor *wearer) {
    if (wearer -> destructible) {
        float amountHealed = wearer->destructible->heal(amount);
        if (amountHealed > 0) {
            return Pickable::use(owner,wearer);
        }
    }
    return false;
}

void Healer::load(TCODZip &zip) {
    amount = zip.getFloat();
}

void Healer::save(TCODZip &zip) {
    zip.putInt(HEALER);
    zip.putFloat(amount);
}

// Remote Hack

RemoteHack::RemoteHack(float range, float damage)
    : range(range), damage(damage) {
}

bool RemoteHack::use(Actor *owner, Actor *wearer) {
    Actor *closestMonster = engine.getClosestMonster(wearer->x, wearer->y, range);
    if (! closestMonster) {
        engine.gui->message(TCODColor::lightGrey, "No enemy is close enough to hack.");
        return false;
    }

    if (closestMonster -> destructible) {
        // hit the closest monster for <damage> hp
        engine.gui->message(TCODColor::lightBlue,
            "Your remote hack reveals the %s's password!!\n"
            "The damage is %g hit points.",
            closestMonster->name, damage);
        closestMonster->destructible->takeDamage(closestMonster, damage);
    }
    return Pickable::use(owner, wearer);
}

void RemoteHack::load(TCODZip &zip) {
    range = zip.getFloat();
    damage = zip.getFloat();
}

void RemoteHack::save(TCODZip &zip) {
    zip.putInt(REMOTE_HACK);
    zip.putFloat(range);
    zip.putFloat(damage);
}

// Fireball

TargetedHack::TargetedHack(float range, float damage) : RemoteHack(range, damage) {
}

bool TargetedHack::use(Actor *owner, Actor *wearer) {
    engine.gui->message(TCODColor::cyan, "Left-click a target tile for the hack,\nor right-click to cancel.");
    int x, y;
    if (! engine.pickATile(&x, &y)) {
        return false;
    }
    TCODRandom *rng = TCODRandom::getInstance();

    TCOD_dice_t d20 = {1, 20};

    if (rng->diceRoll(d20) > 6) {
        // burn everything in range (including player)
        engine.gui->message(TCODColor::orange, "The hack succeeds, breaching everything within %g tiles!", range);
        for (auto actor : engine.actors) {
            if (actor -> destructible && !actor->destructible->isDead() && actor->getDistance(x,y) <= range) {
                engine.gui->message(TCODColor::orange, "The %s is breached for %g hit points!", actor->name, damage);
                actor->destructible->takeDamage(actor, damage);
            }
        }
    }

    return Pickable::use(owner, wearer);
}

void TargetedHack::save(TCODZip &zip) {
    zip.putInt(TARGETED_HACK);
    zip.putFloat(range);
    zip.putFloat(damage);
}

// confuser

Confuser::Confuser(int nbTurns, float range) : nbTurns(nbTurns), range(range) {
}

bool Confuser::use(Actor *owner, Actor *wearer) {
    engine.gui->message(TCODColor::cyan, "Left click an enemy to phish it,\nor right-click to cancel.");
    int x, y;
    if (! engine.pickATile(&x,&y,range)) {
        return false;
    }
    Actor *actor = engine.getActor(x,y);
    if (! actor) {
        return false;
    }

    // confuse the monster for nbTurns turns
    Ai *confusedAi = new ConfusedMonsterAi(nbTurns, actor->ai);
    actor -> ai = confusedAi;

    engine.gui->message(TCODColor::lightGreen, "The %s's computer looks strange,\nand it starts to stumble around!", actor->name);

    return Pickable::use(owner, wearer);
}

void Confuser::load(TCODZip &zip) {
    nbTurns = zip.getInt();
    range = zip.getFloat();
}

void Confuser::save(TCODZip &zip) {
    zip.putInt(CONFUSER);
    zip.putInt(nbTurns);
    zip.putFloat(range);
}
