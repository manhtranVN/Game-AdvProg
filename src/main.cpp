#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h> // Include SDL_ttf
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <list>
#include <string> // For std::string and std::to_string

#include "RenderWindow.hpp"
#include "entity.hpp" // Base entity class
#include "math.hpp"   // vector2d struct/class
#include "utils.hpp"  // For hireTimeInSeconds()
#include "player.hpp" // Player class header
#include "Bullet.hpp" // Bullet class header
#include "Enemy.hpp"  // Enemy class header

using namespace std;

// --- Game State Enum ---
enum class GameState {
    MAIN_MENU,
    PLAYING,
    WON
};

// --- Logical Tile Configuration ---
const int LOGICAL_TILE_WIDTH = 99;
const int LOGICAL_TILE_HEIGHT = 90;

// --- Map Data (Collision Grid) ---
// Paste your mapData here
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

// --- Định nghĩa Biến Toàn cục cho Âm thanh Chết ---
Mix_Chunk* gEnemyDeathSound = nullptr;

int main(int argc, char* args[]) {
    // --- Khởi tạo SDL, IMG, Mixer, và TTF ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) > 0) { cerr << "SDL_Init failed: " << SDL_GetError() << endl; _sleep(10000); return 1; }
    if (!IMG_Init(IMG_INIT_PNG)) { cerr << "IMG_Init failed: " << IMG_GetError() << endl; SDL_Quit(); _sleep(10000); return 1; }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << endl; IMG_Quit(); SDL_Quit(); _sleep(10000); return 1; }
    if (TTF_Init() == -1) { cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << endl; Mix_CloseAudio(); IMG_Quit(); SDL_Quit(); _sleep(10000); return 1; }
    cout << "SDL_mixer and SDL_ttf initialized." << endl;

    // --- Thiết lập Cửa sổ và Renderer ---
    const int SCREEN_WIDTH = 1280; const int SCREEN_HEIGHT = 720;
    RenderWindow window("Contra Clone by Me :)", SCREEN_WIDTH, SCREEN_HEIGHT);
    int refreshRate = window.getRefreshRate(); if (refreshRate <= 0) refreshRate = 60;
    cout << "Refresh Rate: " << refreshRate << endl;

    // --- Nạp Font ---
    TTF_Font* uiFont = TTF_OpenFont("res/fonts/Contra.ttf", 24); // Font cho UI trong game
    TTF_Font* menuFont = TTF_OpenFont("res/fonts/Contra.ttf", 28); // Font cho menu
    if (uiFont == nullptr || menuFont == nullptr) { cerr << "Failed to load font(s)! TTF_Error: " << TTF_GetError() << endl; TTF_Quit(); Mix_CloseAudio(); IMG_Quit(); SDL_Quit(); _sleep(10000); return 1; }
    cout << "Fonts loaded." << endl;

    // --- Nạp Textures ---
    SDL_Texture* menuBackgroundTexture = window.loadTexture("res/gfx/menu_background.png");
    SDL_Texture* backgroundTexture = window.loadTexture("res/gfx/ContraMapStage1BG.png");
    SDL_Texture* playerRunTexture = window.loadTexture("res/gfx/MainChar2.png");
    SDL_Texture* playerJumpTexture = window.loadTexture("res/gfx/Jumping.png");
    SDL_Texture* playerEnterWaterTexture = window.loadTexture("res/gfx/Watersplash.png");
    SDL_Texture* playerSwimTexture = window.loadTexture("res/gfx/Diving.png");
    SDL_Texture* playerShootHorizTexture = window.loadTexture("res/gfx/Shooting.png");
    SDL_Texture* playerShootUpTexture = window.loadTexture("res/gfx/Shootingupward.png");
    SDL_Texture* playerRunShootHorizTexture = window.loadTexture("res/gfx/Shooting.png");
    SDL_Texture* bulletTexture = window.loadTexture("res/gfx/WBullet.png");
    SDL_Texture* enemyTexture = window.loadTexture("res/gfx/Enemy.png");

    // --- Nạp Âm Thanh ---
    Mix_Music* backgroundMusic = Mix_LoadMUS("res/snd/background_music.wav");
    Mix_Chunk* shootSound = Mix_LoadWAV("res/snd/player_shoot.wav");
    gEnemyDeathSound = Mix_LoadWAV("res/snd/enemy_death.wav");
    // Mix_Chunk* winSound = Mix_LoadWAV("res/snd/win_sound.wav");
    // Mix_Chunk* menuConfirmSound = Mix_LoadWAV("res/snd/menu_confirm.wav");

    // --- Kiểm tra lỗi nạp tài nguyên ---
    bool loadError = false;
    if (!menuBackgroundTexture) { cerr << "Error loading menu background texture!" << endl; loadError = true; }
    if (!backgroundTexture || !playerRunTexture || !playerJumpTexture || !playerEnterWaterTexture || !playerSwimTexture || !playerShootHorizTexture || !playerShootUpTexture || !playerRunShootHorizTexture || !bulletTexture || !enemyTexture) { cerr << "Error loading game textures!" << endl; loadError = true; }
    if (!backgroundMusic) { cerr << "Failed to load music! Mix_Error: " << Mix_GetError() << endl; loadError = true; }
    if (!shootSound) { cerr << "Failed to load shoot sound! Mix_Error: " << Mix_GetError() << endl; loadError = true; }
    if (!gEnemyDeathSound) { cerr << "Failed to load enemy death sound! Mix_Error: " << Mix_GetError() << endl; loadError = true; }
    if (loadError) {
        SDL_DestroyTexture(menuBackgroundTexture); SDL_DestroyTexture(backgroundTexture); SDL_DestroyTexture(playerRunTexture); SDL_DestroyTexture(playerJumpTexture); SDL_DestroyTexture(playerEnterWaterTexture); SDL_DestroyTexture(playerSwimTexture); SDL_DestroyTexture(playerShootHorizTexture); SDL_DestroyTexture(playerShootUpTexture); SDL_DestroyTexture(playerRunShootHorizTexture); SDL_DestroyTexture(bulletTexture); SDL_DestroyTexture(enemyTexture);
        Mix_FreeMusic(backgroundMusic); Mix_FreeChunk(shootSound); Mix_FreeChunk(gEnemyDeathSound);
        TTF_CloseFont(uiFont); TTF_CloseFont(menuFont);
        TTF_Quit(); Mix_CloseAudio(); IMG_Quit(); SDL_Quit(); _sleep(10000); return 1;
    }

    // --- Lấy kích thước ảnh nền game ---
    int BG_TEXTURE_WIDTH = 0, BG_TEXTURE_HEIGHT = 0;
    if(backgroundTexture) SDL_QueryTexture(backgroundTexture, NULL, NULL, &BG_TEXTURE_WIDTH, &BG_TEXTURE_HEIGHT);
    cout << "Background Texture Size: " << BG_TEXTURE_WIDTH << "x" << BG_TEXTURE_HEIGHT << endl;

    // --- Cấu hình Player ---
    const int PLAYER_FRAME_W = 40; const int PLAYER_FRAME_H = 78;
    const int PLAYER_RUN_SHEET_COLS = 6; const int PLAYER_JUMP_SHEET_COLS = 4;
    const int PLAYER_ENTER_WATER_SHEET_COLS = 1; const int PLAYER_SWIM_SHEET_COLS = 5;
    const int PLAYER_SHOOT_HORIZ_SHEET_COLS = 3; const int PLAYER_SHOOT_UP_SHEET_COLS = 2;
    const int PLAYER_RUN_SHOOT_HORIZ_SHEET_COLS = 3;

    // --- Trạng thái Game ban đầu ---
    const double INITIAL_CAMERA_X = 320.0; const double INITIAL_CAMERA_Y = 0.0;
    const double PLAYER_START_OFFSET_X = 100.0; const double PLAYER_START_Y = 100.0;
    double cameraX = 0; double cameraY = 0;
    GameState currentGameState = GameState::MAIN_MENU;

    // --- Khai báo đối tượng game ---
    Player* player_ptr = nullptr;
    list<Bullet> bullets;
    list<Enemy> enemies;
    int initialEnemyCount = 0;

    // --- Biến Vòng lặp Game ---
    bool gameRunning = true; bool isPaused = false; SDL_Event event;
    bool gameWon = false;
    bool isMusicPlaying = false;
    const double timeStep = 0.01; double accumulator = 0.0;
    double currentTime = utils::hireTimeInSeconds();

    // --- Hàm Lambda để Khởi tạo/Reset Game ---
    auto initializeGame = [&]() {
        cout << "Initializing Game State..." << endl;
        bullets.clear(); enemies.clear();
        if (player_ptr) { delete player_ptr; player_ptr = nullptr; }

        cameraX = INITIAL_CAMERA_X; cameraY = INITIAL_CAMERA_Y;
        gameWon = false; isPaused = false;

        player_ptr = new Player({INITIAL_CAMERA_X + PLAYER_START_OFFSET_X, PLAYER_START_Y},
                              playerRunTexture, PLAYER_RUN_SHEET_COLS, playerJumpTexture, PLAYER_JUMP_SHEET_COLS,
                              playerEnterWaterTexture, PLAYER_ENTER_WATER_SHEET_COLS, playerSwimTexture, PLAYER_SWIM_SHEET_COLS,
                              playerShootHorizTexture, PLAYER_SHOOT_HORIZ_SHEET_COLS, playerShootUpTexture, PLAYER_SHOOT_UP_SHEET_COLS,
                              playerRunShootHorizTexture, PLAYER_RUN_SHOOT_HORIZ_SHEET_COLS, PLAYER_FRAME_W, PLAYER_FRAME_H);

        auto spawnEnemy = [&](double worldX, int groundRow) {
             double enemyHeight = 72; double groundY = groundRow * LOGICAL_TILE_HEIGHT; double spawnY = groundY - enemyHeight;
             enemies.emplace_back(vector2d{worldX, spawnY}, enemyTexture);
        };
        // Điều chỉnh chỉ số hàng cho đúng
        spawnEnemy(8.0 * LOGICAL_TILE_WIDTH, 4); spawnEnemy(10.0 * LOGICAL_TILE_WIDTH, 4);
        spawnEnemy(15.0 * LOGICAL_TILE_WIDTH, 4); spawnEnemy(20.0 * LOGICAL_TILE_WIDTH, 4);
        spawnEnemy(25.0 * LOGICAL_TILE_WIDTH, 4); spawnEnemy(30.0 * LOGICAL_TILE_WIDTH, 4);
        spawnEnemy(50.0 * LOGICAL_TILE_WIDTH, 3); spawnEnemy(55.0 * LOGICAL_TILE_WIDTH, 4);
        //spawnEnemy(60.0 * LOGICAL_TILE_WIDTH, 4); //spawnEnemy(65.0 * LOGICAL_TILE_WIDTH, 4);
        //spawnEnemy(70.0 * LOGICAL_TILE_WIDTH, 4);
        initialEnemyCount = enemies.size();
        cout << "Game Initialized. Spawned " << initialEnemyCount << " enemies." << endl;

        if (!Mix_PlayingMusic()) {
             if (Mix_PlayMusic(backgroundMusic, -1) == -1) { cerr << "Mix_PlayMusic Error: " << Mix_GetError() << endl; }
             else { isMusicPlaying = true; }
        } else if (!isMusicPlaying) { Mix_ResumeMusic(); isMusicPlaying = true; }
        else { isMusicPlaying = true; } // Đảm bảo cờ đúng
    };

    // --- Kích thước Lưới Logic Map ---
    int mapRows = mapData.size(); int mapCols = 0;
    if(mapRows > 0) mapCols = mapData[0].size(); else { cerr << "Error: mapData is empty!" << endl; _sleep(10000); return 1; }
    cout << "Logical Map Grid: " << mapRows << " rows x " << mapCols << " cols" << endl;
    cout << "Logical Tile Size: " << LOGICAL_TILE_WIDTH << "x" << LOGICAL_TILE_HEIGHT << endl;

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

             switch (currentGameState) {
                case GameState::MAIN_MENU:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                            currentGameState = GameState::PLAYING;
                            initializeGame();
                            // if(menuConfirmSound) Mix_PlayChannel(-1, menuConfirmSound, 0);
                        } else if (event.key.keysym.sym == SDLK_ESCAPE) gameRunning = false;
                    }
                    break;

                case GameState::PLAYING:
                     if (event.type == SDL_KEYDOWN) {
                         if (event.key.keysym.sym == SDLK_p && !event.key.repeat) {
                             isPaused = !isPaused;
                             if (isPaused) { if(isMusicPlaying) Mix_PauseMusic(); }
                             else { if(isMusicPlaying) Mix_ResumeMusic(); }
                             cout << (isPaused ? "GAME PAUSED" : "GAME RESUMED") << endl;
                         }
                         else if (event.key.keysym.sym == SDLK_m && !event.key.repeat) {
                             isMusicPlaying = !isMusicPlaying;
                             if (isMusicPlaying) { Mix_ResumeMusic(); cout << "Music Resumed" << endl; }
                             else { Mix_PauseMusic(); cout << "Music Paused" << endl; }
                         }
                         else if (!isPaused && player_ptr) { player_ptr->handleKeyDown(event.key.keysym.sym); }
                         else if (event.key.keysym.sym == SDLK_ESCAPE) gameRunning = false;
                     }
                     break;

                 case GameState::WON:
                      if (event.type == SDL_KEYDOWN) {
                          if (event.key.keysym.sym == SDLK_ESCAPE) gameRunning = false;
                          else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                               currentGameState = GameState::MAIN_MENU;
                          }
                      }
                      break;
             }
        } // Kết thúc xử lý event

        // --- Cập nhật Logic Game ---
        if (currentGameState == GameState::PLAYING && !isPaused) {
            accumulator += frameTime;
            const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
            if(player_ptr) player_ptr->handleInput(currentKeyStates);

            // Fixed Timestep
            while(accumulator >= timeStep) {
                // Updates
                if(player_ptr) player_ptr->update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                if(player_ptr) { vector2d& playerPos = player_ptr->getPos(); playerPos.x = max(cameraX, playerPos.x); }
                for (Enemy& enemy : enemies) { enemy.update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT); }
                for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); ++it_bullet) { it_bullet->update(timeStep); }

                // Bullet Collisions & Erase
                for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); ) {
                     if (!it_bullet->isActive()) { ++it_bullet; continue; }
                     SDL_Rect bulletHB = it_bullet->getWorldHitbox(); bool bullet_hit_someone = false;
                     for (auto it_enemy = enemies.begin(); it_enemy != enemies.end(); ++it_enemy) {
                         if (it_enemy->isAlive()) {
                             SDL_Rect enemyHB = it_enemy->getWorldHitbox();
                             if (SDL_HasIntersection(&bulletHB, &enemyHB)) { it_enemy->takeHit(); it_bullet->setActive(false); bullet_hit_someone = true; break; }
                         }
                     }
                     if (!bullet_hit_someone) it_bullet->checkMapCollision(mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                     if (!it_bullet->isActive()) it_bullet = bullets.erase(it_bullet); else ++it_bullet;
                }

                accumulator -= timeStep;
            } // End Fixed Timestep

            // Remove Dead Enemies
            enemies.remove_if([](const Enemy& enemy) { return enemy.isDead(); });

            // Check Win Condition
            if (enemies.empty() && initialEnemyCount > 0) {
                cout << "*********************\n***   YOU WIN!    ***\n*********************" << endl;
                currentGameState = GameState::WON;
                Mix_HaltMusic(); isMusicPlaying = false;
                // if(winSound) Mix_PlayChannel(-1, winSound, 0);
            }

            // Create New Bullets & Play Sound
            if (player_ptr) {
                 vector2d bulletStartPos, bulletVelocity;
                 if (player_ptr->wantsToShoot(bulletStartPos, bulletVelocity)) {
                      bullets.emplace_back(bulletStartPos, bulletVelocity, bulletTexture);
                      Mix_PlayChannel(-1, shootSound, 0);
                 }
            }

            // Update Camera
            if(player_ptr){
                double playerCenterX = player_ptr->getPos().x + static_cast<double>(PLAYER_FRAME_W) / 2.0;
                double targetCameraX = playerCenterX - static_cast<double>(SCREEN_WIDTH) / 2.0;
                if (targetCameraX > cameraX) cameraX = targetCameraX;
            }

        } // End if(PLAYING && !isPaused)

        // --- Clamp Camera ---
        if(currentGameState != GameState::MAIN_MENU){
            cameraX = max(0.0, cameraX); cameraY = max(0.0, cameraY);
            if (BG_TEXTURE_WIDTH > 0 && BG_TEXTURE_WIDTH > SCREEN_WIDTH) cameraX = min(cameraX, static_cast<double>(BG_TEXTURE_WIDTH - SCREEN_WIDTH)); else cameraX = 0.0;
            if (BG_TEXTURE_HEIGHT > 0 && BG_TEXTURE_HEIGHT > SCREEN_HEIGHT) cameraY = min(cameraY, static_cast<double>(BG_TEXTURE_HEIGHT - SCREEN_HEIGHT)); else cameraY = 0.0;
        }

        // --- Vẽ ---
        window.clear();

        switch (currentGameState) {
            case GameState::MAIN_MENU: {
                SDL_RenderCopy(window.getRenderer(), menuBackgroundTexture, NULL, NULL);
                // Vẽ chữ "PRESS ENTER TO START"
                SDL_Color textColor = {255, 255, 255, 255}; // Trắng
                SDL_Surface* textSurface = TTF_RenderText_Solid(menuFont, "PRESS ENTER TO START", textColor); // Sử dụng menuFont
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(window.getRenderer(), textSurface);
                    if (textTexture) {
                        int textW = textSurface->w; int textH = textSurface->h;
                        SDL_Rect textDestRect = {(SCREEN_WIDTH - textW) / 2, SCREEN_HEIGHT - textH - 60, textW, textH}; // Vị trí gần cuối
                        SDL_RenderCopy(window.getRenderer(), textTexture, NULL, &textDestRect);
                        SDL_DestroyTexture(textTexture);
                    } else { cerr << "Failed to create menu text texture! SDL_Error: " << SDL_GetError() << endl; }
                    SDL_FreeSurface(textSurface);
                } else { cerr << "Failed to render menu text surface! TTF_Error: " << TTF_GetError() << endl; }
                break;
            } // Kết thúc case MAIN_MENU

            case GameState::PLAYING:
            case GameState::WON: {
                // Vẽ nền game
                SDL_Rect bgSrcRect = {static_cast<int>(round(cameraX)), static_cast<int>(round(cameraY)), SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_Rect bgDestRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                if(backgroundTexture) SDL_RenderCopy(window.getRenderer(), backgroundTexture, &bgSrcRect, &bgDestRect);

                // Vẽ Lưới Debug (nếu bật)
                #ifdef DEBUG_DRAW_GRID
                    SDL_Renderer* renderer = window.getRenderer();
                    if (renderer) {
                        SDL_Color oldColor; SDL_GetRenderDrawColor(renderer, &oldColor.r, &oldColor.g, &oldColor.b, &oldColor.a);
                        SDL_BlendMode oldBlendMode; SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 255, 255, 255, 70);
                        int startCol = static_cast<int>(floor(cameraX / LOGICAL_TILE_WIDTH)); int endCol = static_cast<int>(ceil((cameraX + SCREEN_WIDTH) / LOGICAL_TILE_WIDTH));
                        int startRow = static_cast<int>(floor(cameraY / LOGICAL_TILE_HEIGHT)); int endRow = static_cast<int>(ceil((cameraY + SCREEN_HEIGHT) / LOGICAL_TILE_HEIGHT));
                        endCol = min(endCol, mapCols); endRow = min(endRow, mapRows);
                        for (int c = startCol; c <= endCol; ++c) { int screenX = static_cast<int>(round(c * LOGICAL_TILE_WIDTH - cameraX)); SDL_RenderDrawLine(renderer, screenX, 0, screenX, SCREEN_HEIGHT); }
                        for (int r = startRow; r <= endRow; ++r) { int screenY = static_cast<int>(round(r * LOGICAL_TILE_HEIGHT - cameraY)); SDL_RenderDrawLine(renderer, 0, screenY, SCREEN_WIDTH, screenY); }
                        SDL_SetRenderDrawColor(renderer, oldColor.r, oldColor.g, oldColor.b, oldColor.a); SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
                    }
                #endif

                // Vẽ các đối tượng game
                for (Enemy& enemy : enemies) { enemy.render(window, cameraX, cameraY); }
                for(Bullet& bullet : bullets) { bullet.render(window, cameraX, cameraY); }
                if(player_ptr) player_ptr->render(window, cameraX, cameraY); // Vẽ player

                // --- VẼ SỐ LƯỢNG ĐỊCH CÒN LẠI ---
                if (currentGameState == GameState::PLAYING) {
                    SDL_Color enemyCountColor = {255, 255, 255, 255}; // Trắng
                    string enemyCountText = "Enemies: " + std::to_string(enemies.size());
                    SDL_Surface* surface = TTF_RenderText_Solid(uiFont, enemyCountText.c_str(), enemyCountColor); // Dùng uiFont
                    if (surface) {
                        SDL_Texture* texture = SDL_CreateTextureFromSurface(window.getRenderer(), surface);
                        if (texture) {
                            int textW = surface->w; int textH = surface->h;
                            SDL_Rect dest = {10, 10, textW, textH}; // Góc trên trái
                            SDL_RenderCopy(window.getRenderer(), texture, NULL, &dest);
                            SDL_DestroyTexture(texture);
                        } else { cerr << "Failed to create enemy count texture! SDL_Error: " << SDL_GetError() << endl; }
                        SDL_FreeSurface(surface);
                    } else { cerr << "Failed to render enemy count surface! TTF_Error: " << TTF_GetError() << endl; }
                }

                // Vẽ lớp phủ Pause hoặc Win
                if (isPaused && currentGameState == GameState::PLAYING) {
                    SDL_Renderer* renderer = window.getRenderer();
                    if(renderer){ /* Vẽ overlay đen mờ */
                        SDL_Color oldColor; SDL_GetRenderDrawColor(renderer, &oldColor.r, &oldColor.g, &oldColor.b, &oldColor.a);
                        SDL_BlendMode oldBlendMode; SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
                        SDL_Rect pauseOverlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}; SDL_RenderFillRect(renderer, &pauseOverlay);
                        SDL_SetRenderDrawColor(renderer, oldColor.r, oldColor.g, oldColor.b, oldColor.a); SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
                        // Vẽ chữ "PAUSED" nếu có
                    }
                }
                else if (currentGameState == GameState::WON) {
                     SDL_Renderer* renderer = window.getRenderer();
                     if(renderer){
                         // Vẽ overlay xanh lá mờ
                         SDL_Color oldColor; SDL_GetRenderDrawColor(renderer, &oldColor.r, &oldColor.g, &oldColor.b, &oldColor.a);
                         SDL_BlendMode oldBlendMode; SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
                         SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 0, 200, 0, 120);
                         SDL_Rect winOverlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}; SDL_RenderFillRect(renderer, &winOverlay);
                         SDL_SetRenderDrawColor(renderer, oldColor.r, oldColor.g, oldColor.b, oldColor.a); SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);

                         // Vẽ chữ "YOU WIN!" và hướng dẫn
                        SDL_Color winTextColor = { 255, 255, 0, 255 }; // Vàng
                         SDL_Surface* winSurf1 = TTF_RenderText_Solid(menuFont, "YOU WIN!", winTextColor); // Dùng menuFont cho lớn hơn
                         SDL_Surface* winSurf2 = TTF_RenderText_Solid(uiFont, "Press Enter for Menu or ESC to Exit", winTextColor); // Dùng uiFont
                         if(winSurf1 && winSurf2){
                             SDL_Texture* tex1 = SDL_CreateTextureFromSurface(renderer, winSurf1);
                             SDL_Texture* tex2 = SDL_CreateTextureFromSurface(renderer, winSurf2);
                             if(tex1 && tex2){
                                 int w1 = winSurf1->w, h1 = winSurf1->h; int w2 = winSurf2->w, h2 = winSurf2->h;
                                 SDL_Rect r1 = {(SCREEN_WIDTH - w1)/2, SCREEN_HEIGHT/2 - h1 - 10, w1, h1}; // YOU WIN! ở trên
                                 SDL_Rect r2 = {(SCREEN_WIDTH - w2)/2, SCREEN_HEIGHT/2 + 10, w2, h2}; // Hướng dẫn ở dưới
                                 SDL_RenderCopy(renderer, tex1, NULL, &r1); SDL_RenderCopy(renderer, tex2, NULL, &r2);
                                 SDL_DestroyTexture(tex1); SDL_DestroyTexture(tex2);
                             } else { cerr << "Failed to create win text textures! SDL_Error: " << SDL_GetError() << endl; }
                             SDL_FreeSurface(winSurf1); SDL_FreeSurface(winSurf2);
                         } else { cerr << "Failed to render win text surfaces! TTF_Error: " << TTF_GetError() << endl; }
                     }
                }
                break;
            } // Kết thúc case PLAYING/WON
        } // Kết thúc switch vẽ

        window.display(); // Hiển thị frame

        // --- Giới hạn FPS ---
        int frameTicks = SDL_GetTicks() - startTicks;
        int desiredFrameTime = 1000 / refreshRate;
        if(frameTicks < desiredFrameTime) {
            SDL_Delay(desiredFrameTime - frameTicks); // Đợi nếu xong quá nhanh
        }

    } // Kết thúc vòng lặp game chính

    // --- Cleanup ---
    if(player_ptr) delete player_ptr; // Dọn dẹp con trỏ Player
    Mix_FreeMusic(backgroundMusic); Mix_FreeChunk(shootSound); Mix_FreeChunk(gEnemyDeathSound); gEnemyDeathSound = nullptr;
    // if(winSound) Mix_FreeChunk(winSound); if(menuConfirmSound) Mix_FreeChunk(menuConfirmSound);
    SDL_DestroyTexture(menuBackgroundTexture); SDL_DestroyTexture(backgroundTexture); SDL_DestroyTexture(playerRunTexture); SDL_DestroyTexture(playerJumpTexture); SDL_DestroyTexture(playerEnterWaterTexture); SDL_DestroyTexture(playerSwimTexture); SDL_DestroyTexture(playerShootHorizTexture); SDL_DestroyTexture(playerShootUpTexture); SDL_DestroyTexture(playerRunShootHorizTexture); SDL_DestroyTexture(bulletTexture); SDL_DestroyTexture(enemyTexture);
    TTF_CloseFont(uiFont); TTF_CloseFont(menuFont); // <<< DỌN DẸP cả 2 FONT
    TTF_Quit(); // <<< THOÁT TTF
    Mix_CloseAudio(); // Đóng hệ thống âm thanh
    Mix_Quit();       // Thoát SDL_mixer
    window.cleanUp(); // Dọn dẹp cửa sổ và renderer
    IMG_Quit();       // Thoát SDL_image
    SDL_Quit();       // Thoát SDL

    return 0;
}