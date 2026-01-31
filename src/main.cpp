#include "raylib.h"
#include "raymath.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

enum Environment {
    wall = 1,
    box = 2,
};
std::vector<Rectangle> obstacles;
class World {
  public:
    bool LoadCSVLevel() {
        std::ifstream file;
        file.open("./LdkLevels/test/simplified/Level_0/IntGrid.csv");
        if (!file.is_open()) {
            std::cerr << "File couldn't be opened, ldtklevel";
            return false;
        }
        std::string currentLine;
        int currentRow = 0;
        while (std::getline(file, currentLine)) {
            std::stringstream ss(currentLine);
            std::string cell;
            int currentCol = 0;
            // break cells to one tile with ','
            while (std::getline(ss, cell, ',')) {
                // get cell type
                int tileType = std::stoi(cell);
                switch (tileType) {
                case wall:
                    Rectangle rect;
                    rect.x = currentCol * 64;
                    rect.y = currentRow * 64;
                    rect.width = 64;
                    rect.height = 64;
                    obstacles.push_back(rect);
                }
                currentCol++;
            }
            currentRow++;
        }
        return true;
    }
};

class MovementComponent {
  public:
    Vector2 velocity = {0.0};
    Vector2 lastPosition{0, 0};
    void UpdateVelocity(Vector2 moveDirection, float speed, float friction = 12.f) {
        Vector2 acceleration;

        float magnitude = Vector2Length(moveDirection);

        if (magnitude > 1) {
            float inversion = 1 / magnitude;
            moveDirection.x *= inversion;
            moveDirection.y *= inversion;
        }

        float dt = GetFrameTime();

        acceleration.x = moveDirection.x * speed;
        acceleration.y = moveDirection.y * speed;

        velocity.x += acceleration.x * dt;
        velocity.y += acceleration.y * dt;

        velocity.x -= velocity.x * friction * dt;
        velocity.y -= velocity.y * friction * dt;
    }

    void MoveAndCollide() {}
};

class Player {
  public:
    Vector2 playerPosition{160, 121};
    Vector2 direction{0, 0};
    Rectangle playerRect{playerPosition.x, playerPosition.y, 64, 64};
    MovementComponent MovementComponent;
    Color color = RED;
    bool Collide(const Rectangle &rect) {
        if (playerRect.x < rect.x + rect.width && playerRect.x + playerRect.width > rect.x &&
            playerRect.y < rect.y + rect.height && playerRect.y + playerRect.height > rect.y) {
            return true;
        } else {
            return false;
        }
    };
    void player_update() {
        float dt = GetFrameTime();
        direction.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
        direction.y = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
        MovementComponent.UpdateVelocity(direction, 7000);

        playerPosition.x += MovementComponent.velocity.x * dt;
        playerRect.x = playerPosition.x;
        for (const auto &obstacle : obstacles) {
            if (Collide(obstacle)) {
                if (MovementComponent.velocity.x > 0) {
                    playerPosition.x = obstacle.x - playerRect.width;
                    playerRect.x = playerPosition.x;
                    break;
                } else if (MovementComponent.velocity.x < 0) {
                    playerPosition.x = obstacle.x + obstacle.width;
                    playerRect.x = playerPosition.x;
                    break;
                }
            }
        }

        playerPosition.y += MovementComponent.velocity.y * dt;
        playerRect.y = playerPosition.y;
        for (const auto &obstacle : obstacles) {
            if (Collide(obstacle)) {
                if (MovementComponent.velocity.y > 0) {
                    playerPosition.y = obstacle.y - playerRect.height;
                    playerRect.y = playerPosition.y;
                    break;
                } else if (MovementComponent.velocity.y < 0) {
                    playerPosition.y = obstacle.y + obstacle.height;
                    playerRect.y = playerPosition.y;
                    break;
                }
            }
        }
    };
};
int main() {
    InitWindow(800, 600, "Slick Game");
    SetTargetFPS(60);
    World world;
    world.LoadCSVLevel();

    Player player;
    Vector2 lastPosition{0, 0};

    Camera2D camera = {0};
    camera.target = Vector2{player.playerPosition.x + 20.f, player.playerPosition.y + 20.f};
    camera.offset = Vector2{GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.zoom = 1.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        player.player_update();
        camera.target = Vector2{player.playerPosition.x + 20, player.playerPosition.y + 20};

        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            
            BeginMode2D(camera);
            {
                DrawText("Hello!", 40, 180, 30, BLACK);
                DrawRectangleRec(player.playerRect, player.color);
                for (int i = 0; i < obstacles.size(); i++) {
                    DrawRectangleRec(obstacles[i], BLACK);
                }
            }
            EndMode2D();
        }
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
};
