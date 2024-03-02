#include <emscripten.h>
#include <stdio.h>

#include "raylib.h"
#include "ecs.h"

struct Game {

};

Game game;

void initGame() {
    const int screenWidth = 400;
    const int screenHeight = 300;
    InitWindow(screenWidth, screenHeight, "oh uh");
}

void mainloop() {

    BeginDrawing();
    {
        ClearBackground(RAYWHITE);

        DrawFPS(10, 10);
    }
    EndDrawing();

    if (WindowShouldClose()) {
        emscripten_cancel_main_loop();
    }
}

void unloadGame() {
    CloseWindow();
}

int main(void) {
    initGame();
    emscripten_set_main_loop(mainloop, 0, 1);
    unloadGame();
}
