struct Tile {
    bool explored;
    Tile() : explored(false) {}
};

class Map : public Persistent {
public:
    int width,height;

    Map(int width, int height);
    ~Map();
    bool isWall(int x, int y) const;
    bool isInFov(int x, int y) const;
    bool isExplored(int x, int y) const;
    bool canWalk(int x, int y) const;
    void computeFov();
    void render() const;
    void init(bool withActors);
    void load(TCODZip &zip);
    void save(TCODZip &zip);
protected:
    Tile *tiles;
    TCODMap *map;
    long seed;
    TCODRandom *rng;
    friend class BspListener;

    void dig(int x1, int y1, int x2, int y2);
    void createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors);
    void addMonster(int x, int y);
    void addItem(int x, int y);
};