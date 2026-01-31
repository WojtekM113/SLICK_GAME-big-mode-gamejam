#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <iterator>
Rectangle obstacles[2];
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
    
    void MoveAndCollide()
    {
        
    }
};
class Player {
  public:
    Vector2 playerPosition{0, 0};
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

    Player player;

    Vector2 lastPosition{0, 0};
    Rectangle recA = {125, 125, 64, 64};
    Rectangle recB = {155, 200, 64, 64};
    obstacles[0] = recA;
    obstacles[1] = recB;
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        player.player_update();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello!", 40, 180, 30, BLACK);
        DrawRectangleRec(player.playerRect, player.color);
        for (int i = 0; i < std::size(obstacles); i++) {
            DrawRectangleRec(obstacles[i], DARKGREEN);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
};
