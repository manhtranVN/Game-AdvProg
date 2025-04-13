#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<iostream>
#include<vector>
#include<math.h>

#include "RenderWindow.hpp"
#include "entity.hpp"
#include "math.hpp"
#include "utils.hpp"
#include "player.hpp"

using namespace std;

const int TILE_WIDTH = 9900;
const int TILE_HEIGHT = 720;
const int TILESET_COLS = 100;

const double LOGICAL_TILE_WIDTH = 9900/100*1.0;
const double LOGICAL_TILE_HEIGHT = 720/7*1.0;

// Thay thế hoàn toàn khai báo mapData cũ bằng cái này:
vector<vector<int>> mapData = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 2, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 2, 2, 2, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1}
};

int main(int argc, char* args[])
{
    if(SDL_Init(SDL_INIT_VIDEO) > 0)
    {
        cout << "SDL_Init failed. SDL_Error: " << SDL_GetError() << endl;
    }
    if(!IMG_Init(IMG_INIT_PNG))
    {
        cout << "IMG_Init failed. SDL_Error: " << SDL_GetError() << endl;
    }

    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;

    RenderWindow window("game", SCREEN_WIDTH, SCREEN_HEIGHT);
    //int windowRefreshRate = window.getRefreshRate();
    cout << window.getRefreshRate() << endl;

    SDL_Texture* backgroundTexture = window.loadTexture("res/gfx/ContraMapStage1BG.png"); // Đổi tên biến cho rõ
    SDL_Texture* playerRunTexture = window.loadTexture("res/gfx/MainChar2.png");
    SDL_Texture* playerJumpTexture = window.loadTexture("res/gfx/Jumping.png");

    if (backgroundTexture == nullptr || playerRunTexture == nullptr || playerJumpTexture == nullptr) {
        cout << "Error loading textures!" << endl;
        window.cleanUp();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    entity mapTileDrawer({0, 0}, backgroundTexture, TILE_WIDTH, TILE_HEIGHT, TILESET_COLS);

    int BG_TEXTURE_WIDTH, BG_TEXTURE_HEIGHT;
    SDL_QueryTexture(backgroundTexture, NULL, NULL, &BG_TEXTURE_WIDTH, &BG_TEXTURE_HEIGHT);
    cout << "Background Texture Size: " << BG_TEXTURE_WIDTH << "x" << BG_TEXTURE_HEIGHT << endl;

    const double PLAYER_START_OFFSET_X = 100.0d;
    const double PLAYER_START_Y = 100.0d;
    const int PLAYER_FRAME_W = 40;      // !!! Chiều rộng frame
    const int PLAYER_FRAME_H = 78;      // !!! Chiều cao frame
    const int PLAYER_RUN_SHEET_COLS = 6; // !!! Số cột trong player_run.png
    const int PLAYER_JUMP_SHEET_COLS = 4;// !!! Số cột trong player_jump.png

    const double INITIAL_CAMERA_X = 0.0d; // <<<=== THAY GIÁ TRỊ NÀY BẰNG TỌA ĐỘ X BẠN TÌM ĐƯỢC
    const double INITIAL_CAMERA_Y = 0.0d;   // Thường thì bắt đầu từ trên cùng (Y=0)

    double cameraX = INITIAL_CAMERA_X;
    double cameraY = INITIAL_CAMERA_Y;
    const double CAMERA_SPEED = 300.0d;

    Player player({INITIAL_CAMERA_X + PLAYER_START_OFFSET_X, PLAYER_START_Y},
                  playerRunTexture, PLAYER_RUN_SHEET_COLS,    // Truyền texture chạy
                  playerJumpTexture, PLAYER_JUMP_SHEET_COLS,  // Truyền texture nhảy
                  PLAYER_FRAME_W, PLAYER_FRAME_H);            // Truyền kích thước frame

    int mapRows = mapData.size();
    int mapCols = 0;
    if(mapRows > 0)
    {
        mapCols = mapData[0].size();
    }
    const int MAP_PIXEL_WIDTH = mapCols * TILE_WIDTH;
    const int MAP_PIXEL_HEIGHT = mapRows * TILE_HEIGHT;

    bool gameRunning = true;

    SDL_Event event;

    const double timeStep = 0.01d;
    double accumulator = 0.0d;
    double currentTime = utils::hireTimeInSeconds();

    while(gameRunning)
    {
        int startTicks = SDL_GetTicks();
        double newTime = utils::hireTimeInSeconds();
        double frameTime = newTime - currentTime;
        if(frameTime > 0.25) frameTime = 0.25;
        currentTime = newTime;
        accumulator += frameTime;

        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
        double dx = 0.0d, dy = 0.0d; 
        player.handleInput(currentKeyStates); 

        if (currentKeyStates[SDL_SCANCODE_LEFT]) 
        {
            dx -= CAMERA_SPEED * frameTime;
        }
        if (currentKeyStates[SDL_SCANCODE_RIGHT]) 
        {
            dx += CAMERA_SPEED * frameTime;
        }
        if (currentKeyStates[SDL_SCANCODE_UP]) 
        {
            dy -= CAMERA_SPEED * frameTime;
        }
        if (currentKeyStates[SDL_SCANCODE_DOWN]) 
        {
            dy += CAMERA_SPEED * frameTime;
        }

        cameraX += dx;
        cameraY += dy;

        cameraX = max(0.0d, cameraX);
        cameraY = max(0.0d, cameraY);
        cameraX = min(cameraX, (double)BG_TEXTURE_WIDTH - SCREEN_WIDTH);
        cameraY = min(cameraY, (double)BG_TEXTURE_HEIGHT - SCREEN_HEIGHT);
        if (BG_TEXTURE_WIDTH <= SCREEN_WIDTH) cameraX = 0.0;
        if (BG_TEXTURE_HEIGHT <= SCREEN_HEIGHT) cameraY = 0.0;
        //cout << cameraX << " " << cameraY << endl;
        if (MAP_PIXEL_WIDTH > SCREEN_WIDTH) 
        {
            cameraX = min(cameraX, (double)MAP_PIXEL_WIDTH - SCREEN_WIDTH);
        } 
        else 
        {
            cameraX = 0.0d; 
        }
        if (MAP_PIXEL_HEIGHT > SCREEN_HEIGHT) 
        {
            cameraY = min(cameraY, (double)MAP_PIXEL_HEIGHT - SCREEN_HEIGHT);
        } 
        else 
        {
            cameraY = 0.0d; 
        }

        while(accumulator >= timeStep)
        {
            player.update(static_cast<double>(timeStep), mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_QUIT) gameRunning = false;
            }
            accumulator -= timeStep;
        }

        //const double alpha = accumulator / timeStep;

        window.clear();

        SDL_Rect bgSrcRect; // Phần của ảnh nền cần vẽ
        bgSrcRect.x = static_cast<int>(cameraX);
        bgSrcRect.y = static_cast<int>(cameraY);
        bgSrcRect.w = SCREEN_WIDTH;
        bgSrcRect.h = SCREEN_HEIGHT;

        SDL_Rect bgDestRect; // Vẽ vào đâu trên màn hình
        bgDestRect.x = 0;
        bgDestRect.y = 0;
        bgDestRect.w = SCREEN_WIDTH;
        bgDestRect.h = SCREEN_HEIGHT;

        SDL_RenderCopy(window.getRenderer(), backgroundTexture, &bgSrcRect, &bgDestRect);

        SDL_Renderer* renderer = window.getRenderer();
        if (renderer) {
            // Đặt màu vẽ cho lưới (ví dụ: màu trắng hơi trong suốt)
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50); // Màu trắng, alpha 50

            // Vẽ các đường dọc
            for (int c = 0; c <= mapCols; ++c) {
                int screenX = static_cast<int>(c * LOGICAL_TILE_WIDTH - cameraX);
                // Chỉ vẽ nếu đường kẻ nằm trong màn hình (tối ưu nhỏ)
                if (screenX >= 0 && screenX < SCREEN_WIDTH) {
                    SDL_RenderDrawLine(renderer, screenX, 0, screenX, SCREEN_HEIGHT);
                }
            }
            // Vẽ các đường ngang
            for (int r = 0; r <= mapRows; ++r) {
                int screenY = static_cast<int>(r * LOGICAL_TILE_HEIGHT - cameraY);
                // Chỉ vẽ nếu đường kẻ nằm trong màn hình (tối ưu nhỏ)
                if (screenY >= 0 && screenY < SCREEN_HEIGHT) {
                    SDL_RenderDrawLine(renderer, 0, screenY, SCREEN_WIDTH, screenY);
                }
            }
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); // Tắt blend
        }

        player.render(window, cameraX, cameraY);

        //cout << utils::hireTimeInSeconds() << endl;
        window.display();

        int frameTicks = SDL_GetTicks() - startTicks;
        if(frameTicks < 1000 / window.getRefreshRate())
        {
            SDL_Delay(1000/window.getRefreshRate() - frameTicks);
        }

    }

    window.cleanUp();
    SDL_Quit();

    return 0;
}