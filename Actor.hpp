class Actor : public Persistent {
public:
    int x,y; // position on map
    int ch; // ascii code
    const char *name;
    TCODColor col;
    bool blocks; // can we walk on this actor?
    bool fovOnly; // only display when in FOV
    Attacker *attacker; // something that deals damage
    Destructible *destructible; // something that can be damaged
    Ai *ai; // something self-updating
    Pickable *pickable; // something than can be picked up and used
    Container *container;

    ~Actor();
    Actor(int x, int y, int ch, const char *name, const TCODColor &col);
    void update();
    bool moveOrAttack(int x, int y);
    void render() const;
    float getDistance(int cx, int cy) const;

    void load(TCODZip &zip);
    void save(TCODZip &zip);
};
