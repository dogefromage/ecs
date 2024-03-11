#include <emscripten.h>
#include <stdio.h>

#include "raylib.h"
#include "game.h"

void mainloop() {

    gGame.input();
    gGame.update();
    gGame.render();

    if (!gGame.isRunning) {
        emscripten_cancel_main_loop();
    }

    if (WindowShouldClose()) {
        emscripten_cancel_main_loop();
    }
}

int main(void) {
    emscripten_set_main_loop(mainloop, 0, 1);
}
