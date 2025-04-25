#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "RenderWindow.hpp"
#include "entity.hpp"
#include "math.hpp" // Giả định vector2d dùng double
#include "utils.hpp"
#include "player.hpp"

using namespace std;

// --- KÍCH THƯỚC TILE LOGIC ---
const int LOGICAL_TILE_WIDTH = 9900/100; // VÍ DỤ - Đặt kích thước logic bạn muốn
const int LOGICAL_TILE_HEIGHT = 720/8;

// --- MAP DATA (8x100) ---
// Cần khớp với ảnh nền theo lưới LOGICAL_TILE đã chọn
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
    if (SDL_Init(SDL_INIT_VIDEO) > 0) { cerr << "SDL_Init failed: " << SDL_GetError() << endl; return 1; }
    if (!IMG_Init(IMG_INIT_PNG)) { cerr << "IMG_Init failed: " << IMG_GetError() << endl; SDL_Quit(); return 1; }

    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;
    RenderWindow window("game", SCREEN_WIDTH, SCREEN_HEIGHT);
    int refreshRate = window.getRefreshRate();
    if (refreshRate <= 0) refreshRate = 60;
    cout << "Refresh Rate: " << refreshRate << endl;

    SDL_Texture* backgroundTexture = window.loadTexture("res/gfx/ContraMapStage1BG.png");
    SDL_Texture* playerRunTexture = window.loadTexture("res/gfx/MainChar2.png");
    SDL_Texture* playerJumpTexture = window.loadTexture("res/gfx/Jumping.png");
    SDL_Texture* playerEnterWaterTexture = window.loadTexture("res/gfx/Watersplash.png"); // THAY TÊN FILE
    SDL_Texture* playerSwimTexture = window.loadTexture("res/gfx/Diving.png");      // THAY TÊN FILE

    if (!backgroundTexture || !playerRunTexture || !playerJumpTexture || !playerEnterWaterTexture || !playerSwimTexture) {
        cerr << "Error loading one or more textures!" << endl;
        SDL_DestroyTexture(backgroundTexture); SDL_DestroyTexture(playerRunTexture);
        SDL_DestroyTexture(playerJumpTexture); SDL_DestroyTexture(playerEnterWaterTexture);
        SDL_DestroyTexture(playerSwimTexture);
        window.cleanUp(); IMG_Quit(); SDL_Quit(); return 1;
    }

    int BG_TEXTURE_WIDTH, BG_TEXTURE_HEIGHT;
    SDL_QueryTexture(backgroundTexture, NULL, NULL, &BG_TEXTURE_WIDTH, &BG_TEXTURE_HEIGHT);
    cout << "Background Texture Size: " << BG_TEXTURE_WIDTH << "x" << BG_TEXTURE_HEIGHT << endl;

    const int PLAYER_FRAME_W = 40; // KIỂM TRA LẠI
    const int PLAYER_FRAME_H = 78; // KIỂM TRA LẠI
    const int PLAYER_RUN_SHEET_COLS = 6;
    const int PLAYER_JUMP_SHEET_COLS = 4;
    const int PLAYER_ENTER_WATER_SHEET_COLS = 4; // GIẢ ĐỊNH - SỬA LẠI
    const int PLAYER_SWIM_SHEET_COLS = 4;        // GIẢ ĐỊNH - SỬA LẠI

    const double INITIAL_CAMERA_X = 320.0;
    const double INITIAL_CAMERA_Y = 0.0;
    const double PLAYER_START_OFFSET_X = 100.0;
    const double PLAYER_START_Y = 100.0;

    double cameraX = INITIAL_CAMERA_X;
    double cameraY = INITIAL_CAMERA_Y;

    Player player({INITIAL_CAMERA_X + PLAYER_START_OFFSET_X, PLAYER_START_Y},
                  playerRunTexture, PLAYER_RUN_SHEET_COLS,
                  playerJumpTexture, PLAYER_JUMP_SHEET_COLS,
                  playerEnterWaterTexture, PLAYER_ENTER_WATER_SHEET_COLS,
                  playerSwimTexture, PLAYER_SWIM_SHEET_COLS,
                  PLAYER_FRAME_W, PLAYER_FRAME_H);

    int mapRows = mapData.size();
    int mapCols = 0;
    if(mapRows > 0) mapCols = mapData[0].size();
    else { cerr << "Error: mapData is empty!" << endl; /*...*/ return 1; }
    cout << "Logical Map Grid: " << mapRows << " rows x " << mapCols << " cols" << endl;
    cout << "Logical Tile Size: " << LOGICAL_TILE_WIDTH << "x" << LOGICAL_TILE_HEIGHT << endl;

    bool gameRunning = true;
    SDL_Event event;
    const double timeStep = 0.01;
    double accumulator = 0.0;
    double currentTime = utils::hireTimeInSeconds();

    while(gameRunning) {
        int startTicks = SDL_GetTicks();
        double newTime = utils::hireTimeInSeconds();
        double frameTime = newTime - currentTime;
        if(frameTime > 0.25) frameTime = 0.25;
        currentTime = newTime;
        accumulator += frameTime;

        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) gameRunning = false;
        }

        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
        player.handleInput(currentKeyStates);

        while(accumulator >= timeStep) {
            player.update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
            vector2d& playerPos = player.getPos();
            // Đảm bảo sử dụng kiểu double cho cameraX khi so sánh và gán
            playerPos.x = max(cameraX, playerPos.x); // max hoạt động với double
            accumulator -= timeStep;
        }

        double playerCenterX = player.getPos().x + static_cast<double>(PLAYER_FRAME_W) / 2.0;
        double targetCameraX = playerCenterX - static_cast<double>(SCREEN_WIDTH) / 2.0;
        if (targetCameraX > cameraX) {
            cameraX = targetCameraX;
        }

        cameraX = max(0.0, cameraX);
        cameraY = max(0.0, cameraY);
        cameraX = min(cameraX, static_cast<double>(BG_TEXTURE_WIDTH - SCREEN_WIDTH));
        cameraY = min(cameraY, static_cast<double>(BG_TEXTURE_HEIGHT - SCREEN_HEIGHT));
        if (BG_TEXTURE_WIDTH <= SCREEN_WIDTH) cameraX = 0.0;
        if (BG_TEXTURE_HEIGHT <= SCREEN_HEIGHT) cameraY = 0.0;

        window.clear();

        SDL_Rect bgSrcRect = {static_cast<int>(round(cameraX)), static_cast<int>(round(cameraY)), SCREEN_WIDTH, SCREEN_HEIGHT}; // Làm tròn để vẽ
        SDL_Rect bgDestRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(window.getRenderer(), backgroundTexture, &bgSrcRect, &bgDestRect);

        #ifdef DEBUG_DRAW_GRID
            SDL_Renderer* renderer = window.getRenderer();
            if (renderer) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
                for (int c = 0; c <= mapCols; ++c) {
                    int screenX = static_cast<int>(round(c * LOGICAL_TILE_WIDTH - cameraX));
                    if (screenX >= 0 && screenX < SCREEN_WIDTH) {
                        SDL_RenderDrawLine(renderer, screenX, 0, screenX, SCREEN_HEIGHT);
                    }
                }
                for (int r = 0; r <= mapRows; ++r) {
                    int screenY = static_cast<int>(round(r * LOGICAL_TILE_HEIGHT - cameraY));
                    if (screenY >= 0 && screenY < SCREEN_HEIGHT) {
                        SDL_RenderDrawLine(renderer, 0, screenY, SCREEN_WIDTH, screenY);
                    }
                }
                 SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }
        #endif

        player.render(window, cameraX, cameraY);

        window.display();

        int frameTicks = SDL_GetTicks() - startTicks;
        int desiredFrameTime = 1000 / refreshRate;
        if(frameTicks < desiredFrameTime) {
            SDL_Delay(desiredFrameTime - frameTicks);
        }
    }

    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(playerRunTexture);
    SDL_DestroyTexture(playerJumpTexture);
    SDL_DestroyTexture(playerEnterWaterTexture);
    SDL_DestroyTexture(playerSwimTexture);
    window.cleanUp();
    IMG_Quit();
    SDL_Quit();

    return 0;
}