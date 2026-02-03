#include "raylib.h"
#include "raymath.h"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
class MovementComponent {

  public:
    Vector2 velocity = {0.0};
    float fricition = 12.f;
    bool applyFricition = true;
    Vector2 currentAcceleration;
    // void UpdateVelocity(Vector2 moveDirection, float speed, float friction
    // = 12.f) {

    void SetVelocity(Vector2 moveDirection, const float speed) {
        // float dt = GetFrameTime();
        // float magnitude = Vector2Length(moveDirection);
        // if (magnitude > 1) {
        //     float inversion = 1 / magnitude;
        //     moveDirection.x *= inversion;
        //     moveDirection.y *= inversion;
        // }
        // velocity.x = moveDirection.x * speed;
        // velocity.y = moveDirection.y * speed;

        Vector2 normalizedDir = Vector2Normalize(moveDirection);

        // 2. Mnożymy znormalizowany kierunek przez stałą prędkość
        velocity.x = normalizedDir.x * speed;
        velocity.y = normalizedDir.y * speed;
    }

    void Accelerate(Vector2 accelerationDir, float speed) {
        float dt = GetFrameTime();
        currentAcceleration.x = accelerationDir.x * speed;
        currentAcceleration.y = accelerationDir.y * speed;

        velocity.x += currentAcceleration.x * dt;
        velocity.y += currentAcceleration.y * dt;
    }

    // velocity.x -= velocity.x * friction * dt;
    // velocity.y -= velocity.y * friction * dt;
    // its tied to fps somehow
    //
    void ApplyFriction(const float friction) {
        float dt = GetFrameTime();
        float damping = 1.0f / (1.0f + (friction * dt));
        velocity.x *= damping;
        velocity.y *= damping;
    }
    // }
};

enum Environment { wall = 1, box = 2, placeholder = 3 };
enum GameStates { main_menu = 0, game = 1, pause_menu = 2, settings = 3, game_over = 4 };
static GameStates gamestate = main_menu;

class Bullet {
  public:
    MovementComponent movementComp;
    Vector2 bulletPosition{0, 0};
    Vector2 direction;
    float speed;

    Bullet(Vector2 bulletPosition, Vector2 direction, float speed) {
        this->bulletPosition = bulletPosition;
        this->direction = direction;
        this->speed = speed;
        movementComp.SetVelocity(direction, speed);
        movementComp.fricition = 0.0f;
    }

    void BulletUpdate() {
        float dt = GetFrameTime();
        bulletPosition.x += movementComp.velocity.x * dt;
        bulletPosition.y += movementComp.velocity.y * dt;
    }

    void Render() { DrawCircleV(bulletPosition, 5.f, RED); }
};

std::vector<Bullet> bullets;
class Actions {
  public:
    static void Shoot(Vector2 position, const Vector2 direction, const float speed, const float,
                      Vector2 addedVelocity) {

        Bullet bullet(position, direction, speed);
        bullet.movementComp.velocity.x += addedVelocity.x;
        bullet.movementComp.velocity.y += addedVelocity.y;
        bullets.push_back(bullet);
    }
};

class GameAssets {
  public:
    std::vector<Texture2D> textures;

    Texture2D crosshair;
    // Texture2D player;
    // Texture2D wall1;
    // Texture2D wall2;
    // Texture2D ground;

    void LoadTextures() {
        crosshair = LoadTexture("Textures/crosshair.png");
        textures.push_back(crosshair);
    }

    void UnloadTextures() {
        for (auto const texture : textures) {
            UnloadTexture(texture);
        }
        textures.clear();
    };
};

struct Tile {
    Rectangle tileRec;
    Color tileCol;
    Environment type;
    Tile(Rectangle rect, Color color, Environment type) {
        tileRec = rect;
        tileCol = color;
        this->type = type;
    }
};

std::vector<Tile> obstacles;

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
        while (std::getline(ss, cell, ',')) {
            int tileType = std::stoi(cell);
            switch (tileType) {
            case wall: {
                Rectangle rect;
                rect.x = currentCol * 124;
                rect.y = currentRow * 124;
                rect.width = 124;
                rect.height = 124;
                Tile tile(rect, BLACK, wall);
                obstacles.push_back(tile);
                break;
            }
            case placeholder:
                Rectangle rect;
                rect.x = currentCol * 124;
                rect.y = currentRow * 124;
                rect.width = 124;
                rect.height = 124;
                Tile tile(rect, GREEN, placeholder);
                obstacles.push_back(tile);
                break;
            }
            currentCol++;
        }
        currentRow++;
    }
    return true;
}

class Player {
  public:
    std::vector<Rectangle> tail;

    Vector2 playerPosition{160, 121};
    Vector2 direction{0, 0};
    Rectangle playerRect{playerPosition.x, playerPosition.y, 64, 64};
    MovementComponent MovementComponent;
    Color color = RED;
    int playerHealth = 3;
    float playerSpeed = 6000;

    bool Collide(const Rectangle &rect) {
        if (playerRect.x < rect.x + rect.width && playerRect.x + playerRect.width > rect.x &&
            playerRect.y < rect.y + rect.height && playerRect.y + playerRect.height > rect.y) {
            return true;
        } else {
            return false;
        }
    };
    void AfterMovementCollision() {
        float dt = GetFrameTime();
        playerPosition.x += MovementComponent.velocity.x * dt;
        playerRect.x = playerPosition.x;
        for (const auto &obstacle : obstacles) {
            if (Collide(obstacle.tileRec)) {
                if (obstacle.type != wall) {
                    break;
                }
                if (MovementComponent.velocity.x > 0) {
                    playerPosition.x = obstacle.tileRec.x - playerRect.width;
                    playerRect.x = playerPosition.x;
                    break;
                } else if (MovementComponent.velocity.x < 0) {
                    playerPosition.x = obstacle.tileRec.x + obstacle.tileRec.width;
                    playerRect.x = playerPosition.x;
                    break;
                }
            }
        }

        playerPosition.y += MovementComponent.velocity.y * dt;
        playerRect.y = playerPosition.y;
        for (const auto &obstacle : obstacles) {
            if (Collide(obstacle.tileRec)) {
                if (obstacle.type != wall) {
                    break;
                }
                if (MovementComponent.velocity.y > 0) {
                    playerPosition.y = obstacle.tileRec.y - playerRect.height;
                    playerRect.y = playerPosition.y;
                    break;
                } else if (MovementComponent.velocity.y < 0) {
                    playerPosition.y = obstacle.tileRec.y + obstacle.tileRec.height;
                    playerRect.y = playerPosition.y;
                    break;
                }
            }
        }
    }

    void player_update() {
        direction.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
        direction.y = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
        // MovementComponent.Accelerate(direction, 5000);
        MovementComponent.ApplyFriction(0);
        AfterMovementCollision();
        for (int i = 0; i < tail.size(); i++) {
            Vector2 targetPosition;
            if (i == 0) {
                targetPosition.x = playerPosition.x;
                targetPosition.y = playerPosition.y;
            } else {
                targetPosition.x = tail[i - 1].x;
                targetPosition.y = tail[i - 1].y;
            }
            tail[i].x += (targetPosition.x - tail[i].x) * 0.1f;
            tail[i].y += (targetPosition.y - tail[i].y) * 0.1f;
        }
    };
};

class Crosshair {
  public:
    Vector2 crosshairPosition;
    Rectangle crosshairSourceRec{0.0f, 0.0f, 33, 33};
    Rectangle crosshairDestRec;
    float crosshairSize = 64;
    float offset = crosshairSize / 2.f;

    void UpdateCrosshairPos() {
        crosshairPosition = GetMousePosition();
        crosshairDestRec = {crosshairPosition.x - offset, crosshairPosition.y - offset, crosshairSize, crosshairSize};
    }
    void RenderCrosshair(const Texture2D &crosshairTexture) {
        DrawTexturePro(crosshairTexture, crosshairSourceRec, crosshairDestRec, {0, 0}, 0, RAYWHITE);
    }
};

void ReadInput() {
    if (IsKeyPressed(KEY_P)) {
        if (gamestate == game) {
            gamestate = pause_menu;
        } else if (gamestate == pause_menu) {
            gamestate = game;
        }
    }
    if (IsKeyPressed(KEY_M)) {
        gamestate = main_menu;
    }
}

Vector2 MouseWorldDir(Vector2 crosshairPos, Camera2D camera, Vector2 origin) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(crosshairPos, camera);
    Vector2 diection = Vector2Subtract(mouseWorldPos, origin);
    Vector2 normalizedDir = Vector2Normalize(diection);
    return normalizedDir;
}
int main() {
    InitWindow(800, 600, "Slick Game");
    SetTargetFPS(60);
    GameAssets assets;
    assets.LoadTextures();
    LoadCSVLevel();

    Player player;

    Camera2D camera = {0};
    camera.target = Vector2{player.playerPosition.x + 32.f, player.playerPosition.y + 32.f};
    camera.offset = Vector2{GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.zoom = 1.0f;
    bool startGame = false;
    bool game_pause = false;
    Crosshair crosshair;

    Rectangle tRect{player.playerPosition.x, player.playerPosition.y, 64, 64};
    player.tail.push_back(tRect);
    Rectangle tRect1{player.playerPosition.x, player.playerPosition.y, 64, 64};
    Rectangle tRect2{player.playerPosition.x, player.playerPosition.y, 64, 64};
    player.tail.push_back(tRect1);
    player.tail.push_back(tRect2);
    while (!WindowShouldClose()) {
        ReadInput();
        crosshair.UpdateCrosshairPos();
        switch (gamestate) {
        case game: {

            float dt = GetFrameTime();
            Vector2 playerOffset{player.playerPosition.x + 32.f, player.playerPosition.y + 32.f};
            camera.target = playerOffset;

            Vector2 aimDir = MouseWorldDir(crosshair.crosshairPosition, camera, playerOffset);
            // camera.zoom = 1 -   player.MovementComponent.velocity.x;
            // float dot = Vector2DotProduct(aimDir, Vector2Normalize(player.MovementComponent.velocity));
            //
            float currentSpeed = Vector2Length(player.MovementComponent.velocity);
            float calculateZoom = 1.0 - (currentSpeed / 10000.f);
            if (calculateZoom <= 0.6) {
                calculateZoom = 0.6f;
            }
            camera.zoom = Lerp(camera.zoom, calculateZoom, 0.05f);
            Vector2 currentMoveDirection = Vector2Normalize(player.MovementComponent.velocity);
            float turnSpeed = 0.05f;
            float currentTraction = turnSpeed / (1.0f + (currentSpeed * 0.005f));

            Vector2 dir = Vector2Lerp(currentMoveDirection, aimDir, currentTraction);
            player.MovementComponent.velocity = Vector2Scale(dir, currentSpeed);

            player.MovementComponent.Accelerate(aimDir, 300);

            player.player_update();
            for (auto &bullet : bullets) {
                bullet.BulletUpdate();
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

                Actions::Shoot(playerOffset, aimDir, 1000, 500, player.MovementComponent.velocity);
            }
            break;
        }
        case pause_menu: {
            break;
        }
        };

        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        switch (gamestate) {
        case main_menu: {
            if (GuiButton(Rectangle{(GetScreenWidth() / 2.f) - 120 / 2.f, 24, 120, 30}, "#191#Show Message")) {
                gamestate = game;
            }
            startGame = false;
            break;
        }
        case pause_menu: {
            // Just skips update loop and renders game
            // in the background
        }
        case game: {
            BeginMode2D(camera);
            {

                for (int i = 0; i < obstacles.size(); i++) {
                    DrawRectangleRec(obstacles[i].tileRec, obstacles[i].tileCol);
                }

                DrawRectangleRec(player.playerRect, player.color);
                for (const auto &tailPiece : player.tail) {
                    DrawRectangleRec(tailPiece, RED);
                }
                for (auto &bullet : bullets) {
                    bullet.Render();
                }

                Vector2 debugCenter{player.playerPosition.x + 32, player.playerPosition.y + 32};
                Vector2 mouseWorldPos = GetScreenToWorld2D(crosshair.crosshairPosition, camera);
                DrawLineV(debugCenter, mouseWorldPos, RED);
            }
            EndMode2D();
            Rectangle textRect{0, 0, 125, 60};
            DrawText(TextFormat("Velocity: %.2f", Vector2Length(player.MovementComponent.velocity)), 0, 0, 32, GREEN);
            crosshair.RenderCrosshair(assets.crosshair);
            break;
        }
        case game_over: {
            break;
        }
        };
        EndDrawing();
    };
    assets.UnloadTextures();
    CloseWindow();
    return 0;
}
