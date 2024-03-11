#include "ecs.h"

class Game {
   public:
    Game();
    ~Game();

    void input();
    void update();
    void render();

    bool isRunning{true};
};

extern Game gGame;