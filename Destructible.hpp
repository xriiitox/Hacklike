class Destructible : public Persistent {
public:
    float maxHp; // maximum health points
    float hp; // current health points
    float defense; // hit points deflected
    std::unique_ptr<char[]> corpseName; // the actor's name once dead/destroyed
    int xp; // XP gained when killing this monster (or player xp)

    Destructible(float maxHp, float defense, const char *corpseName, int xp);
    inline bool isDead() { return hp <= 0; }

    float takeDamage(Actor *owner, float damage);

    virtual void die(Actor *owner);

    float heal(float amount);

    virtual ~Destructible();

    void load(TCODZip &zip);
    void save(TCODZip &zip);
    static Destructible *create(TCODZip &zip);
protected:
    enum DestructibleType {
        MONSTER,PLAYER
    };
};

class MonsterDestructible : public Destructible {
public:
    MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp);

    void save(TCODZip &zip);

    void die(Actor *owner);
};

class PlayerDestructible : public Destructible {
public:
    PlayerDestructible(float maxHp, float defense, const char *corpseName);

    void save(TCODZip &zip);

    void die(Actor *owner);
};
