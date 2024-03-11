#include "game.h"

#include <raylib-cpp.hpp>

Game gGame;

// struct RigidBody {
//     raylib::Vector3 velocity;
// };

struct MyTransform {
    raylib::Vector3 position;
};

// class PhysicsSystem : public System {
//    public:
//     void update(float dt);
// };

// void PhysicsSystem::update(float dt) {
//     for (auto const& entity : entities) {
//         auto& rigidBody = gCoordinator.getComponent<RigidBody>(entity);
//         auto& transform = gCoordinator.getComponent<MyTransform>(entity);
//         transform.position += rigidBody.velocity * dt;
//     }
// }

class RenderSystem : public System {
   public:
    raylib::Camera2D camera{};

    void render();
};

void RenderSystem::render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode2D(camera);

    for (auto const& entity : entities) {
        auto& transform = gCoordinator.getComponent<MyTransform>(entity);
        DrawRectangle(transform.position.x, transform.position.y, 10, 10, RED);
        printf("Rendered %d\n", entity);
    }

    EndMode2D();

    DrawFPS(10, 10);
    EndDrawing();
}

// std::shared_ptr<PhysicsSystem> gPhysicsSystem;
std::shared_ptr<RenderSystem> gRenderSystem;

Game::Game() {
    printf("Initializing game.\n");

    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "oh uh");

	gCoordinator.init();
    gCoordinator.registerComponent<MyTransform>();

    // uint32_t physicsSystemSignature = 1 << gCoordinator.getComponentType<RigidBody>();
    uint32_t renderSystemSignature = 1 << gCoordinator.getComponentType<MyTransform>();
    // gPhysicsSystem = gCoordinator.registerSystem<PhysicsSystem>();
    gRenderSystem = gCoordinator.registerSystem<RenderSystem>();
    // gCoordinator.setSystemSignature<PhysicsSystem>(physicsSystemSignature);
    gCoordinator.setSystemSignature<RenderSystem>(renderSystemSignature);

    raylib::Camera2D& cam = gRenderSystem->camera;
    cam.target = (Vector2){0, 0};
    cam.offset = (Vector2){0, 0};
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;

    Entity entity = gCoordinator.createEntity();
    gCoordinator.addComponent<Transform>(entity, { { 0, 0, 0 } });
    // gCoordinator.addComponent<RigidBody>(entity, { { 0, 0, 0 } });

    // for (int i = 0; i < 20; i++) {
    //     // spawn entity
    // }
}

Game::~Game() {
    CloseWindow();
}

void Game::input() {
}

void Game::update() {
    // float dt = 1.0 / 30.0;
    // gPhysicsSystem->update(dt);

    if (WindowShouldClose()) {
        isRunning = false;
    }
}

void Game::render() {
    gRenderSystem->render();
}