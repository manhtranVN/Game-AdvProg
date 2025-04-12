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

const int TILE_WIDTH = 3300*3;
const int TILE_HEIGHT = 240*3;
const int TILESET_COLS = 100;

vector<vector<int>> mapData = 
{
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

    SDL_Texture* tileMap = window.loadTexture("res/gfx/ContraMapStage1BG.jpg");
    SDL_Texture* playerRunTexture = window.loadTexture("res/gfx/MainChar2.png");
    SDL_Texture* playerJumpTexture = window.loadTexture("res/gfx/Jumping.png");

    
    if (tileMap == nullptr || playerRunTexture == nullptr) 
    {
        window.cleanUp();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    entity mapTileDrawer({0, 0}, tileMap, TILE_WIDTH, TILE_HEIGHT, TILESET_COLS);

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
    int mapCols = 100;
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

        while(accumulator >= timeStep)
        {
            player.update(static_cast<double>(timeStep), mapData, TILE_WIDTH, TILE_HEIGHT);
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_QUIT) gameRunning = false;
            }
            accumulator -= timeStep;
        }

        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
        double dx = 0.0f, dy = 0.0f; 

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

        cameraX = max(1536.0d, cameraX);
        cameraY = max(0.0d, cameraY);
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

        //const double alpha = accumulator / timeStep;

        while(accumulator >= timeStep)
        {
            accumulator -= timeStep;
        }

        window.clear();

        if (mapRows > 0 && mapCols > 0) 
        {
            int startCol = static_cast<int>(floor(cameraX / TILE_WIDTH));
            int endCol = static_cast<int>(ceil((cameraX + SCREEN_WIDTH) / TILE_WIDTH));
            int startRow = static_cast<int>(floor(cameraY / TILE_HEIGHT));
            int endRow = static_cast<int>(ceil((cameraY + SCREEN_HEIGHT) / TILE_HEIGHT));

            startCol = max(0, startCol);
            endCol = min(mapCols, endCol); 
            startRow = max(0, startRow);
            endRow = min(mapRows, endRow); 

            for (int r = startRow; r < endRow; ++r) 
            { 
                for (int c = startCol; c < endCol; ++c) 
                { 
                    int tileIndex = mapData[r][c];
                    if (tileIndex >= 0) 
                    {
                        mapTileDrawer.setTileFrame(tileIndex);
                        SDL_Rect srcRect = mapTileDrawer.getCurrentFrame();
                        SDL_Rect destRect;
                        destRect.x = static_cast<int>(c * TILE_WIDTH - cameraX);
                        destRect.y = static_cast<int>(r * TILE_HEIGHT - cameraY);
                        destRect.w = TILE_WIDTH;
                        destRect.h = TILE_HEIGHT;
                        SDL_RenderCopy(window.getRenderer(), mapTileDrawer.getTex(), &srcRect, &destRect);
                    }
                }
            }
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