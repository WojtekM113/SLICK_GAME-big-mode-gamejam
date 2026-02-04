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

    void SetVelocity(Vector2 moveDirection, const float speed) {
        Vector2 normalizedDir = Vector2Normalize(moveDirection);
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

    void ApplyFriction(const float friction) {
        float dt = GetFrameTime();
        float damping = 1.0f / (1.0f + (friction * dt));
        velocity.x *= damping;
        velocity.y *= damping;
    }
    // }
};

enum Environment { air = 0, wall = 1, box = 2, placeholder = 3 };
enum GameStates { main_menu = 0, game = 1, pause_menu = 2, game_begin = 3, game_over = 4, restart = 5 };
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
Tile CreateTile(int currentCol, int currentRow, Color color, Environment type) {
    Rectangle rect;
    rect.x = currentCol * 64;
    rect.y = currentRow * 64;
    rect.width = 64;
    rect.height = 64;
    Tile tile(rect, color, type);
    return tile;
}
bool LoadCSVLevel() {
    std::ifstream file;
    file.open("./LdkLevels/lvl512/simplified/Level_0/IntGrid.csv");
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
                obstacles.push_back(CreateTile(currentCol, currentRow, BLACK, wall));
                break;
            }
            case placeholder: {
                obstacles.push_back(CreateTile(currentCol, currentRow, GREEN, placeholder));
                break;
            }
            case box: {
                obstacles.push_back(CreateTile(currentCol, currentRow, BLUE, box));
                break;
            }
            case air: {
                obstacles.push_back(CreateTile(currentCol, currentRow, RAYWHITE, air));
                break;
            }
            };
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

    float timer = 0.0f;
    float secondsToDestroy = 3.5f;
    bool Collide(const Rectangle &rect) {
        if (playerRect.x < rect.x + rect.width && playerRect.x + playerRect.width > rect.x &&
            playerRect.y < rect.y + rect.height && playerRect.y + playerRect.height > rect.y) {
            return true;
        } else {
            return false;
        }
    };

    void MoveAndCollide() {
        float dt = GetFrameTime();
        playerPosition.x += MovementComponent.velocity.x * dt;
        playerRect.x = playerPosition.x;
        for (int i = 0; i < obstacles.size(); i++) {
            if (Collide(obstacles[i].tileRec)) {
                switch (obstacles[i].type) {
                case box: {
                    obstacles[i].type = air;
                    obstacles[i].tileCol = RAYWHITE;
                    Rectangle tRect{playerPosition.x, playerPosition.y, 64, 64};
                    tail.push_back(tRect);
                }
                case wall: {
                    if (MovementComponent.velocity.x > 0) {
                        playerPosition.x = obstacles[i].tileRec.x - playerRect.width;
                        playerRect.x = playerPosition.x;
                        MovementComponent.velocity.x = 0;
                        break;
                    } else if (MovementComponent.velocity.x < 0) {
                        playerPosition.x = obstacles[i].tileRec.x + obstacles[i].tileRec.width;
                        playerRect.x = playerPosition.x;
                        MovementComponent.velocity.x = 0;
                        break;
                    }
                }
                };
            }
        }

        playerPosition.y += MovementComponent.velocity.y * dt;
        playerRect.y = playerPosition.y;
        for (int i = 0; i < obstacles.size(); i++) {
            if (Collide(obstacles[i].tileRec)) {
                switch (obstacles[i].type) {
                case box: {
                    obstacles[i].type = air;
                    obstacles[i].tileCol = RAYWHITE;
                    Rectangle tRect{playerPosition.x, playerPosition.y, 64, 64};
                    tail.push_back(tRect);
                    break;
                }
                case wall: {
                    if (MovementComponent.velocity.y > 0) {
                        playerPosition.y = obstacles[i].tileRec.y - playerRect.height;
                        playerRect.y = playerPosition.y;
                        MovementComponent.velocity.y = 0;
                        break;
                    } else if (MovementComponent.velocity.y < 0) {
                        playerPosition.y = obstacles[i].tileRec.y + obstacles[i].tileRec.height;
                        playerRect.y = playerPosition.y;
                        MovementComponent.velocity.y = 0;
                        break;
                    }
                }
                };
            }
        }
    }

    void player_update() {
        direction.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
        direction.y = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
        // MovementComponent.Accelerate(direction, 5000);
        MovementComponent.ApplyFriction(0.3f);
        MoveAndCollide();

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

        timer += GetFrameTime();
        while (timer >= secondsToDestroy) {
            if (!tail.empty()) {
                tail.pop_back();
                std::cout << "popped\n";
                std::cout << GetTime();
            } else {
                gamestate = game_over;
            }
            timer -= secondsToDestroy;
        };
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

    if (IsKeyPressed(KEY_GRAVE)) {
        gamestate = restart;
    }
}

Vector2 MouseWorldDir(Vector2 crosshairPos, Camera2D camera, Vector2 origin) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(crosshairPos, camera);
    Vector2 diection = Vector2Subtract(mouseWorldPos, origin);
    Vector2 normalizedDir = Vector2Normalize(diection);
    return normalizedDir;
}
int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1920, 1080, "Slick Game");
    SetTargetFPS(60);

    GameAssets assets;
    assets.LoadTextures();
    LoadCSVLevel();
    Player player;
    Vector2 playerStartPosition{160, 121};
    float turnSpeed = 0.05f;
    Camera2D camera = {0};
    camera.target = Vector2{player.playerPosition.x + 32.f, player.playerPosition.y + 32.f};
    camera.offset = Vector2{GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.zoom = 2.0f;
    bool startGame = false;
    bool game_pause = false;
    bool gameStartPause = true;
    Crosshair crosshair;

    Rectangle tRect1{player.playerPosition.x, player.playerPosition.y, 64, 64};
    Rectangle tRect2{player.playerPosition.x, player.playerPosition.y, 64, 64};
    player.tail.push_back(tRect1);
    player.tail.push_back(tRect2);
    gamestate = game_begin;
    while (!WindowShouldClose()) {
        ReadInput();
        crosshair.UpdateCrosshairPos();
        switch (gamestate) {
        case restart: {
            obstacles.clear();
            player.tail.clear();
            LoadCSVLevel();
            player.secondsToDestroy = 3.5;
            player.playerPosition = playerStartPosition;
            player.MovementComponent.velocity = {0, 0};

            Rectangle tRect1{player.playerPosition.x, player.playerPosition.y, 64, 64};
            Rectangle tRect2{player.playerPosition.x, player.playerPosition.y, 64, 64};
            player.tail.push_back(tRect1);
            player.tail.push_back(tRect2);

            player.playerRect.x = player.playerPosition.x;
            player.playerRect.y = player.playerPosition.y;

            camera.target = Vector2{player.playerPosition.x + 32.f, player.playerPosition.y + 32.f};

            gameStartPause = true;
            gamestate = game_begin;
            break;
        }
        case game_begin: {
            player.secondsToDestroy = 4;
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                gamestate = game;
            }
            break;
        }
        case game: {

            float dt = GetFrameTime();
            Vector2 playerOffset{player.playerPosition.x + 32.f, player.playerPosition.y + 32.f};
            camera.target = playerOffset;

            Vector2 aimDir = MouseWorldDir(crosshair.crosshairPosition, camera, playerOffset);

            float currentSpeed = Vector2Length(player.MovementComponent.velocity);
            float calculateZoom = 1.0 - (currentSpeed / 1800.f);
            if (calculateZoom <= 0.12f) {
                calculateZoom = 0.12f;
            }

            camera.zoom = Lerp(camera.zoom, calculateZoom, 0.02f);
            Vector2 currentMoveDirection = Vector2Normalize(player.MovementComponent.velocity);
            float dot = Vector2DotProduct(currentMoveDirection, aimDir);

            float slideSpeed = 0.05f;
            if (dot < 0.7f) {
                turnSpeed = 0.02f;
                slideSpeed = 0.000001f;
                player.MovementComponent.ApplyFriction(0.0f);
            } else {
                turnSpeed = 0.15f;
                slideSpeed = 0.0005f;
            }

            float currentTraction = turnSpeed / (1.0f + (currentSpeed * slideSpeed));

            Vector2 dir = Vector2Lerp(currentMoveDirection, aimDir, currentTraction);
            player.MovementComponent.velocity = Vector2Scale(dir, currentSpeed);
            player.MovementComponent.Accelerate(aimDir, 500);

            player.player_update();
            for (auto &bullet : bullets) {
                bullet.BulletUpdate();
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Actions::Shoot(playerOffset, aimDir, 1000, 500, player.MovementComponent.velocity);
            }
            break;
        }
        case game_over: {
        }
        case pause_menu: {
            break;
        }
        };
        //////////////////////////////////
        // DRAW
        /////////////
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
        case game_begin : {
            DrawText(TextFormat("Right click to start"), GetScreenHeight() * 0.5f, GetScreenWidth() * 0.5f, 64, BLACK);
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

            crosshair.RenderCrosshair(assets.crosshair);

            const char *velocityText = TextFormat("Velocity: %.2f", Vector2Length(player.MovementComponent.velocity));
            DrawText(velocityText, (GetScreenWidth() * 0.5f) - (MeasureText(velocityText, 50) * 0.5f), 0, 50, GREEN);

            const char *timerText = TextFormat("Time left: %.2f", player.secondsToDestroy - player.timer);
            DrawText(timerText, (GetScreenWidth() * 0.5f) - (MeasureText(timerText, 64) * 0.5f), 64, 80, RED);

            break;
        }
        case game_over: {
            const char *gameOverText = TextFormat("Game over! Try again(press  ~)");
            DrawText(gameOverText, (GetScreenWidth() * 0.5f) - (MeasureText(gameOverText, 64) * 0.5f),
                     GetScreenHeight() * 0.5f, 64, RED);
            break;
        }
        };
        EndDrawing();
    };
    assets.UnloadTextures();
    CloseWindow();
    return 0;
}
