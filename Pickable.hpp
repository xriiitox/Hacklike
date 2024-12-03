class Pickable : public Persistent {
public:
    bool pick(Actor *owner, Actor *wearer);
    virtual bool use(Actor *owner, Actor *wearer);
    void drop(Actor *owner, Actor *wearer);

    static Pickable *create(TCODZip &zip);

    virtual ~Pickable() {}
protected:
    enum PickableType {
        HEALER,REMOTE_HACK,CONFUSER,TARGETED_HACK
    };
};

class Healer : public Pickable {
public:
    float amount; // how much hp

    Healer(float amount);
    bool use(Actor *owner, Actor *wearer);
    void load(TCODZip &zip);
    void save(TCODZip &zip);
};

class RemoteHack : public Pickable {
public:
    float range,damage;
    RemoteHack(float range, float damage);
    bool use(Actor *owner, Actor *wearer);
    void load(TCODZip &zip);
    void save(TCODZip &zip);
};

class TargetedHack : public RemoteHack {
public:
    TargetedHack(float range, float damage);
    bool use(Actor *owner, Actor *wearer);
    void save(TCODZip &zip);
};

class Confuser : public Pickable {
public:
    int nbTurns;
    float range;
    Confuser(int nbTurns, float range);
    bool use(Actor *owner, Actor *wearer);
    void load(TCODZip &zip);
    void save(TCODZip &zip);
};