#include "raylib.h"

int main() {
    InitWindow(800, 600, "Slick Game");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello!",40,180,30,BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
