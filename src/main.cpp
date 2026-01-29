#include "raylib.h"
#include <cmath>
#include <iostream>

struct Velocity {
  float speed;
  Vector2 direction;
};
int main() {
  InitWindow(800, 600, "Slick Game");
  SetTargetFPS(60);

  Rectangle player;
  player.height = 64;
  player.width = 64;
  player.x = 0;
  player.y = 0;

  Vector2 direction{0, 0};
  Vector2 velocity{0, 0};
  float speed = 555.0f;
  float friction = 5.0f;
  while (!WindowShouldClose()) {

    direction.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
    direction.y = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);

    //
    float lenght_velocity =
        std::sqrt(std::pow(direction.x, 2) + std::pow(direction.y, 2));
    if (lenght_velocity > 1) {
      float inversion = 1 / lenght_velocity;
      direction.x *= inversion;
      direction.y *= inversion;
    }

    float dt = GetFrameTime(); 
    
    
    velocity.x += (direction.x * speed - velocity.x) * friction * dt;
    velocity.y += (direction.y * speed - velocity.y) * friction * dt;
    
    player.x += velocity.x * dt;
    player.y += velocity.y * dt;
    // std::cout << velocity.x << " " << velocity.y << "\n";
    //

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Hello!", 40, 180, 30, BLACK);
    DrawRectangleRec(player, RED);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
