#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <list> // Sử dụng list để dễ dàng xóa phần tử

#include "RenderWindow.hpp"
#include "entity.hpp"
#include "math.hpp"
#include "utils.hpp"
#include "player.hpp" // Đã bao gồm khai báo Player mới
#include "Bullet.hpp"
#include "Enemy.hpp" // <<< INCLUDE FILE HEADER CỦA ENEMY

using namespace std;

// --- Logical Tile Configuration ---
const int LOGICAL_TILE_WIDTH = 96;
const int LOGICAL_TILE_HEIGHT = 90;

// --- Map Data (Collision Grid) ---
// ... (mapData giữ nguyên) ...
vector<vector<int>> mapData = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 2, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 2, 2, 2, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1}
};

int main(int argc, char* args[]) {
    // --- Khởi tạo SDL và IMG ---
    if (SDL_Init(SDL_INIT_VIDEO) > 0) { cerr << "SDL_Init failed: " << SDL_GetError() << endl; return 1; }
    if (!IMG_Init(IMG_INIT_PNG)) { cerr << "IMG_Init failed: " << IMG_GetError() << endl; SDL_Quit(); return 1; }

    // --- Thiết lập Cửa sổ và Renderer ---
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;
    RenderWindow window("Contra Clone by Me :)", SCREEN_WIDTH, SCREEN_HEIGHT);
    int refreshRate = window.getRefreshRate();
    if (refreshRate <= 0) refreshRate = 60;
    cout << "Refresh Rate: " << refreshRate << endl;

    // --- Nạp Tất cả Textures ---
    SDL_Texture* backgroundTexture = window.loadTexture("res/gfx/ContraMapStage1BG.png");
    SDL_Texture* playerRunTexture = window.loadTexture("res/gfx/MainChar2.png");
    SDL_Texture* playerJumpTexture = window.loadTexture("res/gfx/Jumping.png");
    SDL_Texture* playerEnterWaterTexture = window.loadTexture("res/gfx/Watersplash.png");
    SDL_Texture* playerSwimTexture = window.loadTexture("res/gfx/Diving.png");
    SDL_Texture* playerShootHorizTexture = window.loadTexture("res/gfx/Shooting.png");
    SDL_Texture* playerShootUpTexture = window.loadTexture("res/gfx/Shootingupward.png");
    SDL_Texture* playerRunShootHorizTexture = window.loadTexture("res/gfx/Shooting.png");
    SDL_Texture* bulletTexture = window.loadTexture("res/gfx/WBullet.png");
    SDL_Texture* enemyTexture = window.loadTexture("res/gfx/enemy.png");

    // --- Kiểm tra lỗi nạp Texture ---
    if (!backgroundTexture || !playerRunTexture || !playerJumpTexture || !playerEnterWaterTexture || !playerSwimTexture || !playerShootHorizTexture || !playerShootUpTexture || !playerRunShootHorizTexture || !bulletTexture || !enemyTexture) {
        cerr << "Error loading one or more textures!" << endl;
        SDL_DestroyTexture(backgroundTexture); SDL_DestroyTexture(playerRunTexture); SDL_DestroyTexture(playerJumpTexture);
        SDL_DestroyTexture(playerEnterWaterTexture); SDL_DestroyTexture(playerSwimTexture); SDL_DestroyTexture(playerShootHorizTexture);
        SDL_DestroyTexture(playerShootUpTexture); SDL_DestroyTexture(playerRunShootHorizTexture);
        SDL_DestroyTexture(bulletTexture); SDL_DestroyTexture(enemyTexture);
        window.cleanUp(); IMG_Quit(); SDL_Quit(); return 1;
    }

    // --- Lấy kích thước ảnh nền ---
    int BG_TEXTURE_WIDTH, BG_TEXTURE_HEIGHT;
    SDL_QueryTexture(backgroundTexture, NULL, NULL, &BG_TEXTURE_WIDTH, &BG_TEXTURE_HEIGHT);
    cout << "Background Texture Size: " << BG_TEXTURE_WIDTH << "x" << BG_TEXTURE_HEIGHT << endl;

    // --- Cấu hình Player ---
    const int PLAYER_FRAME_W = 40;
    const int PLAYER_FRAME_H = 78;
    const int PLAYER_RUN_SHEET_COLS = 6;
    const int PLAYER_JUMP_SHEET_COLS = 4;
    const int PLAYER_ENTER_WATER_SHEET_COLS = 1;
    const int PLAYER_SWIM_SHEET_COLS = 5;
    const int PLAYER_SHOOT_HORIZ_SHEET_COLS = 3;
    const int PLAYER_SHOOT_UP_SHEET_COLS = 2;
    const int PLAYER_RUN_SHOOT_HORIZ_SHEET_COLS = 3;

    // --- Trạng thái Game ban đầu ---
    const double INITIAL_CAMERA_X = 320.0;
    const double INITIAL_CAMERA_Y = 0.0;
    const double PLAYER_START_OFFSET_X = 100.0;
    const double PLAYER_START_Y = 100.0;

    double cameraX = INITIAL_CAMERA_X;
    double cameraY = INITIAL_CAMERA_Y;

    // --- Tạo đối tượng Player ---
    Player player({INITIAL_CAMERA_X + PLAYER_START_OFFSET_X, PLAYER_START_Y},
                  playerRunTexture, PLAYER_RUN_SHEET_COLS,
                  playerJumpTexture, PLAYER_JUMP_SHEET_COLS,
                  playerEnterWaterTexture, PLAYER_ENTER_WATER_SHEET_COLS,
                  playerSwimTexture, PLAYER_SWIM_SHEET_COLS,
                  playerShootHorizTexture, PLAYER_SHOOT_HORIZ_SHEET_COLS,
                  playerShootUpTexture, PLAYER_SHOOT_UP_SHEET_COLS,
                  playerRunShootHorizTexture, PLAYER_RUN_SHOOT_HORIZ_SHEET_COLS,
                  PLAYER_FRAME_W, PLAYER_FRAME_H);

    // --- Quản lý Đạn và Kẻ Địch ---
    list<Bullet> bullets;
    list<Enemy> enemies;

    // --- Kích thước Lưới Logic Map ---
    int mapRows = mapData.size();
    int mapCols = 0;
    if(mapRows > 0) mapCols = mapData[0].size();
    else { cerr << "Error: mapData is empty!" << endl; return 1; }
    cout << "Logical Map Grid: " << mapRows << " rows x " << mapCols << " cols" << endl;
    cout << "Logical Tile Size: " << LOGICAL_TILE_WIDTH << "x" << LOGICAL_TILE_HEIGHT << endl;

    // --- Spawn Enemies ---
    auto spawnEnemy = [&](double worldX, int groundRow) {
        double enemyHeight = 72;
        double groundY = groundRow * LOGICAL_TILE_HEIGHT;
        double spawnY = groundY - enemyHeight;
        enemies.emplace_back(vector2d{worldX, spawnY}, enemyTexture);
    };
    spawnEnemy(8.0 * LOGICAL_TILE_WIDTH, 3); spawnEnemy(15.0 * LOGICAL_TILE_WIDTH, 3);
    spawnEnemy(25.0 * LOGICAL_TILE_WIDTH, 3); spawnEnemy(50.0 * LOGICAL_TILE_WIDTH, 3);
    spawnEnemy(60.0 * LOGICAL_TILE_WIDTH, 3); spawnEnemy(70.0 * LOGICAL_TILE_WIDTH, 3);
    cout << "Spawned " << enemies.size() << " enemies." << endl;

    // --- Biến Vòng lặp Game ---
    bool gameRunning = true;
    bool isPaused = false;
    SDL_Event event;
    const double timeStep = 0.01;
    double accumulator = 0.0;
    double currentTime = utils::hireTimeInSeconds();

    // --- Vòng lặp Game Chính ---
    while(gameRunning) {
        int startTicks = SDL_GetTicks();

        double newTime = utils::hireTimeInSeconds();
        double frameTime = newTime - currentTime;
        if(frameTime > 0.25) frameTime = 0.25;
        currentTime = newTime;

        // --- Xử lý Sự kiện SDL ---
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) gameRunning = false;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_p && !event.key.repeat) isPaused = !isPaused;
                 if (!isPaused) player.handleKeyDown(event.key.keysym.sym);
            }
        }

        // --- Cập nhật Logic Game (chỉ khi không Pause) ---
        if (!isPaused) {
            accumulator += frameTime;

            const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
            player.handleInput(currentKeyStates);

            // Cập nhật vật lý với bước thời gian cố định
            while(accumulator >= timeStep) {
                // 1. Cập nhật Player
                player.update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                vector2d& playerPos = player.getPos();
                playerPos.x = max(cameraX, playerPos.x);

                // 2. Cập nhật Kẻ Địch
                for (Enemy& enemy : enemies) {
                    // Gọi update với thông tin map
                    enemy.update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                    // enemy.checkMapCollision(mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT); // Gọi nếu cần check va chạm thêm
                }
                // 3. CẬP NHẬT ĐẠN <<<================ SỬA Ở ĐÂY
                for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); ++it_bullet) {
                     it_bullet->update(timeStep); // <<< THÊM DÒNG NÀY
                }
                // =======================================>>>>>

                // 4. Kiểm tra va chạm Đạn vs Kẻ địch
                for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); /*no increment*/) {
                    if (!it_bullet->isActive()) {
                        ++it_bullet; continue;
                    }
                    SDL_Rect bulletHB = it_bullet->getWorldHitbox();
                    bool bullet_hit_someone = false;
                    for (auto it_enemy = enemies.begin(); it_enemy != enemies.end(); ++it_enemy) {
                        if (it_enemy->isAlive()) {
                            SDL_Rect enemyHB = it_enemy->getWorldHitbox();
                            if (SDL_HasIntersection(&bulletHB, &enemyHB)) {
                                it_enemy->takeHit(); it_bullet->setActive(false);
                                bullet_hit_someone = true; break;
                            }
                        }
                    }

                    // 5. Kiểm tra va chạm Đạn vs Map (NẾU CHƯA TRÚNG ĐỊCH)
                    if (!bullet_hit_someone) {
                         it_bullet->checkMapCollision(mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                    }

                    // 6. Xử lý xóa/duyệt iterator Đạn
                    if (!it_bullet->isActive()) {
                         it_bullet = bullets.erase(it_bullet);
                    } else {
                         ++it_bullet;
                    }
                } // Kết thúc vòng lặp kiểm tra va chạm đạn


                // 7. Cập nhật và xóa đạn ra khỏi màn hình (Optional Boundary Check)
                /* // Có thể không cần thiết nếu lifetime và va chạm map/enemy đủ tốt
                 for(auto it = bullets.begin(); it != bullets.end(); ) {
                     SDL_Rect bulletHB = it->getWorldHitbox();
                     bool outOfBounds = ... ; // Tính outOfBounds
                    if (outOfBounds) { it = bullets.erase(it); } else { ++it; }
                 }
                */

                accumulator -= timeStep; // Trừ thời gian đã xử lý
            } // Kết thúc vòng lặp while(accumulator >= timeStep)

            // --- Xóa kẻ địch đã chết ---
            enemies.remove_if([](const Enemy& enemy) { return enemy.isDead(); });

            // --- Tạo đạn mới nếu Player yêu cầu ---
            vector2d bulletStartPos, bulletVelocity;
            if (player.wantsToShoot(bulletStartPos, bulletVelocity)) {
                 bullets.emplace_back(bulletStartPos, bulletVelocity, bulletTexture);
            }

            // --- Cập nhật Camera ---
            double playerCenterX = player.getPos().x + static_cast<double>(PLAYER_FRAME_W) / 2.0;
            double targetCameraX = playerCenterX - static_cast<double>(SCREEN_WIDTH) / 2.0;
            if (targetCameraX > cameraX) cameraX = targetCameraX;

        } // Kết thúc if(!isPaused)

        // --- Luôn giới hạn Camera trong phạm vi map ---
        cameraX = max(0.0, cameraX); cameraY = max(0.0, cameraY);
        if (BG_TEXTURE_WIDTH > SCREEN_WIDTH) cameraX = min(cameraX, static_cast<double>(BG_TEXTURE_WIDTH - SCREEN_WIDTH)); else cameraX = 0.0;
        if (BG_TEXTURE_HEIGHT > SCREEN_HEIGHT) cameraY = min(cameraY, static_cast<double>(BG_TEXTURE_HEIGHT - SCREEN_HEIGHT)); else cameraY = 0.0;

        // --- Vẽ ---
        window.clear();

        // Vẽ nền
        SDL_Rect bgSrcRect = {static_cast<int>(round(cameraX)), static_cast<int>(round(cameraY)), SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_Rect bgDestRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(window.getRenderer(), backgroundTexture, &bgSrcRect, &bgDestRect);

        // Vẽ Lưới Debug (Nếu bật)
        SDL_Renderer* renderer = window.getRenderer();
        if (renderer) {
            SDL_Color oldColor; SDL_GetRenderDrawColor(renderer, &oldColor.r, &oldColor.g, &oldColor.b, &oldColor.a);
            SDL_BlendMode oldBlendMode; SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
            for (int c = 0; c <= mapCols; ++c) {
                 int screenX = static_cast<int>(round(c * LOGICAL_TILE_WIDTH - cameraX));
                 if (screenX >= 0 && screenX < SCREEN_WIDTH) SDL_RenderDrawLine(renderer, screenX, 0, screenX, SCREEN_HEIGHT);
                 else if (screenX >= SCREEN_WIDTH) break;
             }
             for (int r = 0; r <= mapRows; ++r) {
                 int screenY = static_cast<int>(round(r * LOGICAL_TILE_HEIGHT - cameraY));
                 if (screenY >= 0 && screenY < SCREEN_HEIGHT) SDL_RenderDrawLine(renderer, 0, screenY, SCREEN_WIDTH, screenY);
                  else if (screenY >= SCREEN_HEIGHT) break;
             }
              SDL_SetRenderDrawColor(renderer, oldColor.r, oldColor.g, oldColor.b, oldColor.a);
              SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
         }


        // Vẽ Kẻ Địch
        for (Enemy& enemy : enemies) {
            enemy.render(window, cameraX, cameraY);
        }

        // Vẽ Đạn
        for(Bullet& bullet : bullets) {
             bullet.render(window, cameraX, cameraY);
        }

        // Vẽ Player
        player.render(window, cameraX, cameraY);

        // Vẽ Lớp Phủ Pause
        if (isPaused) {
            SDL_Renderer* renderer = window.getRenderer();
            if(renderer){
                 SDL_Color oldColor; SDL_GetRenderDrawColor(renderer, &oldColor.r, &oldColor.g, &oldColor.b, &oldColor.a);
                 SDL_BlendMode oldBlendMode; SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
                 SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
                 SDL_Rect pauseOverlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}; SDL_RenderFillRect(renderer, &pauseOverlay);
                 SDL_SetRenderDrawColor(renderer, oldColor.r, oldColor.g, oldColor.b, oldColor.a); SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
                 // Thêm code vẽ chữ PAUSED
            }
        }

        window.display();

        // Giới hạn FPS
        int frameTicks = SDL_GetTicks() - startTicks;
        int desiredFrameTime = 1000 / refreshRate;
        if(frameTicks < desiredFrameTime) {
            SDL_Delay(desiredFrameTime - frameTicks);
        }

    } // Kết thúc vòng lặp game chính

    // --- Dọn dẹp Tài nguyên ---
    SDL_DestroyTexture(backgroundTexture); SDL_DestroyTexture(playerRunTexture); SDL_DestroyTexture(playerJumpTexture);
    SDL_DestroyTexture(playerEnterWaterTexture); SDL_DestroyTexture(playerSwimTexture); SDL_DestroyTexture(playerShootHorizTexture);
    SDL_DestroyTexture(playerShootUpTexture); SDL_DestroyTexture(playerRunShootHorizTexture);
    SDL_DestroyTexture(bulletTexture); SDL_DestroyTexture(enemyTexture);
    window.cleanUp(); IMG_Quit(); SDL_Quit();

    return 0;
}