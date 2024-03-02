#include "game.h"

static SDL_Window* win;
static SDL_Renderer* ren;

static bool running = true;
static uint32_t frameCount = 0;
static uint32_t fps = 0;

static void update() {
}

static void input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:  // exit game
                        running = false;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

static void render() {
    SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
    SDL_Rect rect;
    rect.x = rect.y = 0;
    rect.w = 360;
    rect.h = 240;
    SDL_RenderFillRect(ren, &rect);

    SDL_RenderPresent(ren);
}

static void quit() {
    running = false;
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

static void mainloop() {
    static uint32_t lastTicks = SDL_GetTicks();
    static uint32_t lastFrameCount = 0;

    int32_t startTicks = SDL_GetTicks();

    // fps
    if (startTicks >= (lastTicks + 1000)) {
        lastTicks += 1000;
        fps = frameCount - lastFrameCount;
        lastFrameCount = frameCount;

        // std::cout << "FPS " << fps << std::endl;
    }

    update();
    render();

    frameCount++;

    int minimumLoopTicks = 16;

#ifndef __EMSCRIPTEN__
    // delay to get better loop time
    int32_t executionTicks = SDL_GetTicks() - startTicks;
    if (executionTicks < minimumLoopTicks) {
        SDL_Delay(minimumLoopTicks - executionTicks);
    }
#endif

    // if game closes loop will end
    input();

    if (!running) {
        quit();
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop(); /* this should "kill" the app. */
#else
        exit(0);
#endif
    }
}

void init() {
    SDL_Init(0);
    SDL_CreateWindowAndRenderer(360, 240, SDL_RENDERER_PRESENTVSYNC, &win, &ren);
    SDL_SetWindowTitle(win, "Shitty game!");

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 1);
#else
    while (1) {
        mainloop();
    }
#endif
}
