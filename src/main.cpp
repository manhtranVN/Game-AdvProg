    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_mixer.h>
    #include <SDL2/SDL_ttf.h>
    #include <iostream>
    #include <vector>
    #include <cmath>
    #include <algorithm>
    #include <list>
    #include <string>

    #include "RenderWindow.hpp"
    #include "entity.hpp"
    #include "math.hpp"
    #include "utils.hpp"
    #include "player.hpp"
    #include "Bullet.hpp"
    #include "Enemy.hpp"

    using namespace std;

    // --- BẬT TÍNH NĂNG DEBUG ---
    // #define DEBUG_DRAW_GRID
    // #define DEBUG_DRAW_COLUMNS
    // --- --- --- --- --- --- ---

    enum class GameState { MAIN_MENU, PLAYING, WON };
    const int LOGICAL_TILE_WIDTH = 96;
    const int LOGICAL_TILE_HEIGHT = 96;
    vector<vector<int>> mapData = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,1,1,0,0},
        {0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0},
        {3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,3,3,3,3,3,3,3,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1}
    };
    Mix_Chunk* gEnemyDeathSound = nullptr;

    int main(int argc, char* args[]) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) > 0) { cerr << "SDL_Init failed: " << SDL_GetError() << endl; return 1; }
        if (!IMG_Init(IMG_INIT_PNG)) { cerr << "IMG_Init failed: " << IMG_GetError() << endl; SDL_Quit(); return 1; }
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << endl; IMG_Quit(); SDL_Quit(); return 1; }
        if (TTF_Init() == -1) { cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << endl; Mix_CloseAudio(); IMG_Quit(); SDL_Quit(); return 1; }
        cout << "SDL, IMG, Mixer, TTF initialized." << endl;

        const int SCREEN_WIDTH = 1024; const int SCREEN_HEIGHT = 672;
        RenderWindow window("Contra Clone Reloaded", SCREEN_WIDTH, SCREEN_HEIGHT);
        int refreshRate = window.getRefreshRate(); if (refreshRate <= 0) refreshRate = 60;
        cout << "Refresh Rate: " << refreshRate << endl;

        TTF_Font* uiFont = TTF_OpenFont("res/font/kongtext.ttf", 24);
        TTF_Font* menuFont = TTF_OpenFont("res/font/kongtext.ttf", 28);
        TTF_Font* debugFont = TTF_OpenFont("res/font/kongtext.ttf", 16);
        if (uiFont == nullptr || menuFont == nullptr || debugFont == nullptr) {
            cerr << "Failed to load font(s)! TTF_Error: " << TTF_GetError() << endl;
            TTF_Quit(); Mix_CloseAudio(); IMG_Quit(); SDL_Quit(); return 1;
        }
        cout << "Fonts loaded." << endl;

        // --- Nạp Textures ---
        SDL_Texture* menuBackgroundTexture = window.loadTexture("res/gfx/menu_background.png");
        SDL_Texture* backgroundTexture = window.loadTexture("res/gfx/ContraMapStage1BG.png");
        // Basic Player Textures
        SDL_Texture* playerRunTexture = window.loadTexture("res/gfx/MainChar2.png");
        SDL_Texture* playerJumpTexture = window.loadTexture("res/gfx/Jumping.png");
        SDL_Texture* playerEnterWaterTexture = window.loadTexture("res/gfx/Watersplash.png");
        SDL_Texture* playerSwimTexture = window.loadTexture("res/gfx/Diving.png");
        // Aiming Textures
        SDL_Texture* playerAimUpTexture = window.loadTexture("res/gfx/Shootingupward.png"); // Cần tạo file
        SDL_Texture* playerAimDiagUpTexture = window.loadTexture("res/gfx/PlayerAimDiagUp.png"); // Cần tạo file
        SDL_Texture* playerAimDiagDownTexture = window.loadTexture("res/gfx/PlayerAimDiagDown.png"); // Cần tạo file
        SDL_Texture* playerRunAimDiagUpTexture = window.loadTexture("res/gfx/PlayerRunAimDiagUp.png"); // Cần tạo file
        SDL_Texture* playerRunAimDiagDownTexture = window.loadTexture("res/gfx/PlayerRunAimDiagDown.png"); // Cần tạo file
        // Shooting Textures
        SDL_Texture* playerStandShootHorizTexture = window.loadTexture("res/gfx/PlayerStandShoot.png");
        SDL_Texture* playerRunShootHorizTexture = window.loadTexture("res/gfx/Shooting.png");
        SDL_Texture* playerShootUpTexture = window.loadTexture("res/gfx/Shootingupward.png");
        SDL_Texture* playerShootDiagUpTexture = window.loadTexture("res/gfx/PlayerShootDiagUp.png"); // Cần tạo file (có thể trùng với AimDiagUp)
        SDL_Texture* playerShootDiagDownTexture = window.loadTexture("res/gfx/PlayerShootDiagDown.png"); // Cần tạo file (có thể trùng với AimDiagDown)
        // Lying Textures
        SDL_Texture* playerLyingDownTexture = window.loadTexture("res/gfx/PlayerLyingDown.png"); // Cần tạo file
        SDL_Texture* playerLyingShootTexture = window.loadTexture("res/gfx/PlayerLyingShoot.png");
        // Other
        SDL_Texture* bulletTexture = window.loadTexture("res/gfx/WBullet.png");
        SDL_Texture* enemyTexture = window.loadTexture("res/gfx/Enemy.png");

        // --- Nạp Âm Thanh ---
        Mix_Music* backgroundMusic = Mix_LoadMUS("res/snd/background_music.wav");
        Mix_Chunk* shootSound = Mix_LoadWAV("res/snd/player_shoot.wav");
        gEnemyDeathSound = Mix_LoadWAV("res/snd/enemy_death.wav");

        // --- Kiểm tra lỗi nạp tài nguyên ---
        bool loadError = false;
        // Kiểm tra tất cả các con trỏ texture và âm thanh
        if (!menuBackgroundTexture || !backgroundTexture || !playerRunTexture || !playerJumpTexture ||
            !playerEnterWaterTexture || !playerSwimTexture || !playerAimUpTexture || !playerAimDiagUpTexture ||
            !playerAimDiagDownTexture || !playerRunAimDiagUpTexture || !playerRunAimDiagDownTexture ||
            !playerStandShootHorizTexture || !playerRunShootHorizTexture || !playerShootUpTexture ||
            !playerShootDiagUpTexture || !playerShootDiagDownTexture || !playerLyingDownTexture ||
            !playerLyingShootTexture || !bulletTexture || !enemyTexture || !backgroundMusic ||
            !shootSound || !gEnemyDeathSound) {
            loadError = true;
            cerr << "Error loading one or more resources! Check individual texture/sound load messages." << endl;
        }
        // Thêm các kiểm tra chi tiết (ví dụ)
        if (!playerAimUpTexture) cerr << "Failed to load PlayerAimUp.png" << endl;
        if (!playerLyingDownTexture) cerr << "Failed to load PlayerLyingDown.png" << endl;
        // ... và các texture mới khác

        if (loadError) {
            // Dọn dẹp TẤT CẢ tài nguyên đã nạp (kể cả cái thành công)
            SDL_DestroyTexture(menuBackgroundTexture); SDL_DestroyTexture(backgroundTexture);
            SDL_DestroyTexture(playerRunTexture); SDL_DestroyTexture(playerJumpTexture);
            SDL_DestroyTexture(playerEnterWaterTexture); SDL_DestroyTexture(playerSwimTexture);
            SDL_DestroyTexture(playerAimUpTexture); SDL_DestroyTexture(playerAimDiagUpTexture);
            SDL_DestroyTexture(playerAimDiagDownTexture); SDL_DestroyTexture(playerRunAimDiagUpTexture);
            SDL_DestroyTexture(playerRunAimDiagDownTexture); SDL_DestroyTexture(playerStandShootHorizTexture);
            SDL_DestroyTexture(playerRunShootHorizTexture); SDL_DestroyTexture(playerShootUpTexture);
            SDL_DestroyTexture(playerShootDiagUpTexture); SDL_DestroyTexture(playerShootDiagDownTexture);
            SDL_DestroyTexture(playerLyingDownTexture); SDL_DestroyTexture(playerLyingShootTexture);
            SDL_DestroyTexture(bulletTexture); SDL_DestroyTexture(enemyTexture);
            Mix_FreeMusic(backgroundMusic); Mix_FreeChunk(shootSound); Mix_FreeChunk(gEnemyDeathSound);
            TTF_CloseFont(uiFont); TTF_CloseFont(menuFont); TTF_CloseFont(debugFont);
            TTF_Quit(); Mix_CloseAudio(); IMG_Quit(); SDL_Quit();
            return 1;
        }
        cout << "Resources loaded." << endl;

        int BG_TEXTURE_WIDTH = 0, BG_TEXTURE_HEIGHT = 0;
        if(backgroundTexture) SDL_QueryTexture(backgroundTexture, NULL, NULL, &BG_TEXTURE_WIDTH, &BG_TEXTURE_HEIGHT);
        cout << "Background Texture Size: " << BG_TEXTURE_WIDTH << "x" << BG_TEXTURE_HEIGHT << endl;

        // --- Cấu hình Player ---
        const int PLAYER_STANDARD_FRAME_W = 40;
        const int PLAYER_STANDARD_FRAME_H = 78;
        const int PLAYER_LYING_FRAME_W = 78;
        const int PLAYER_LYING_FRAME_H = 40;

        // Số cột cho từng spritesheet (CẦN ĐIỀU CHỈNH THEO FILE ẢNH THỰC TẾ)
        const int PLAYER_RUN_SHEET_COLS = 6; const int PLAYER_JUMP_SHEET_COLS = 4;
        const int PLAYER_ENTER_WATER_SHEET_COLS = 1; const int PLAYER_SWIM_SHEET_COLS = 5; // Hoặc 4
        const int PLAYER_AIM_UP_SHEET_COLS = 1; // Ví dụ
        const int PLAYER_AIM_DIAG_UP_SHEET_COLS = 1; // Ví dụ
        const int PLAYER_AIM_DIAG_DOWN_SHEET_COLS = 1; // Ví dụ
        const int PLAYER_RUN_AIM_DIAG_UP_SHEET_COLS = 3; // Ví dụ: giống anim chạy
        const int PLAYER_RUN_AIM_DIAG_DOWN_SHEET_COLS = 3; // Ví dụ: giống anim chạy
        const int PLAYER_STAND_SHOOT_HORIZ_SHEET_COLS = 3; const int PLAYER_RUN_SHOOT_HORIZ_SHEET_COLS = 3;
        const int PLAYER_SHOOT_UP_SHEET_COLS = 2;
        const int PLAYER_SHOOT_DIAG_UP_SHEET_COLS = 2; // Ví dụ: giống PlayerAimDiagUp hoặc PlayerShootUp
        const int PLAYER_SHOOT_DIAG_DOWN_SHEET_COLS = 3; // Ví dụ: giống PlayerAimDiagDown hoặc PlayerStandShoot
        const int PLAYER_LYING_DOWN_SHEET_COLS = 1; // Ví dụ
        const int PLAYER_LYING_SHOOT_SHEET_COLS = 3;

        const double INITIAL_CAMERA_X = 0.0, INITIAL_CAMERA_Y = 0.0;
        const double PLAYER_START_X = 100.0, PLAYER_START_Y = 100.0;
        double cameraX = INITIAL_CAMERA_X, cameraY = INITIAL_CAMERA_Y;
        GameState currentGameState = GameState::MAIN_MENU;
        Player* player_ptr = nullptr;
        list<Bullet> bullets;
        list<Enemy> enemies_list;
        int initialEnemyCount = 0;
        bool gameRunning = true, isPaused = false, gameWon = false, isMusicPlaying = false;
        const double timeStep = 0.01; double accumulator = 0.0;
        double currentTime = utils::hireTimeInSeconds();
        SDL_Event event;

        auto initializeGame = [&]() {
            cout << "Initializing Game State..." << endl;
            bullets.clear(); enemies_list.clear();
            if (player_ptr) { delete player_ptr; player_ptr = nullptr; }
            cameraX = INITIAL_CAMERA_X; cameraY = INITIAL_CAMERA_Y;
            gameWon = false; isPaused = false;

            player_ptr = new Player({PLAYER_START_X, PLAYER_START_Y},
                                  // Basic
                                  playerRunTexture, PLAYER_RUN_SHEET_COLS,
                                  playerJumpTexture, PLAYER_JUMP_SHEET_COLS,
                                  playerEnterWaterTexture, PLAYER_ENTER_WATER_SHEET_COLS,
                                  playerSwimTexture, PLAYER_SWIM_SHEET_COLS,
                                  // Aiming (Standing)
                                  playerAimUpTexture, PLAYER_AIM_UP_SHEET_COLS,
                                  playerAimDiagUpTexture, PLAYER_AIM_DIAG_UP_SHEET_COLS,
                                  playerAimDiagDownTexture, PLAYER_AIM_DIAG_DOWN_SHEET_COLS,
                                  // Aiming (Running)
                                  playerRunAimDiagUpTexture, PLAYER_RUN_AIM_DIAG_UP_SHEET_COLS,
                                  playerRunAimDiagDownTexture, PLAYER_RUN_AIM_DIAG_DOWN_SHEET_COLS,
                                  // Shooting
                                  playerStandShootHorizTexture, PLAYER_STAND_SHOOT_HORIZ_SHEET_COLS,
                                  playerRunShootHorizTexture, PLAYER_RUN_SHOOT_HORIZ_SHEET_COLS,
                                  playerShootUpTexture, PLAYER_SHOOT_UP_SHEET_COLS,
                                  playerShootDiagUpTexture, PLAYER_SHOOT_DIAG_UP_SHEET_COLS,
                                  playerShootDiagDownTexture, PLAYER_SHOOT_DIAG_DOWN_SHEET_COLS,
                                  // Lying
                                  playerLyingDownTexture, PLAYER_LYING_DOWN_SHEET_COLS,
                                  playerLyingShootTexture, PLAYER_LYING_SHOOT_SHEET_COLS,
                                  // Frame Dimensions
                                  PLAYER_STANDARD_FRAME_W, PLAYER_STANDARD_FRAME_H,
                                  PLAYER_LYING_FRAME_W, PLAYER_LYING_FRAME_H);

            auto spawnEnemy = [&](double worldX, int groundRow){
                 double enemyHeight = 72;
                 double groundY = groundRow * LOGICAL_TILE_HEIGHT;
                 double spawnY = groundY - enemyHeight;
                 cout << "[Spawn] Spawning enemy at X=" << worldX << ", Y=" << spawnY << endl;
                 enemies_list.emplace_back(vector2d{worldX, spawnY}, enemyTexture);
            };
            spawnEnemy(8.0 * LOGICAL_TILE_WIDTH, 3);
            spawnEnemy(15.0 * LOGICAL_TILE_WIDTH, 3);
            initialEnemyCount = enemies_list.size();
            cout << "Game Initialized. Spawned " << initialEnemyCount << " enemies." << endl;

            if (!Mix_PlayingMusic()) {
                 if (Mix_PlayMusic(backgroundMusic, -1) == -1) { cerr << "Mix_PlayMusic Error: " << Mix_GetError() << endl; }
                 else { isMusicPlaying = true; }
            } else if(!isMusicPlaying) { Mix_ResumeMusic(); isMusicPlaying = true; }
        };

        int mapRows = mapData.size();
        int mapCols = (mapRows > 0) ? mapData[0].size() : 0;
        if (mapCols == 0) { cerr << "Error: mapData has 0 columns or is empty!" << endl; return 1; }
        cout << "Logical Map Grid: " << mapRows << " rows x " << mapCols << " cols" << endl;
        cout << "Logical Tile Size: " << LOGICAL_TILE_WIDTH << "x" << LOGICAL_TILE_HEIGHT << endl;

        // --- Vòng lặp Game Chính ---
        while(gameRunning) {
            int startTicks = SDL_GetTicks();
            double newTime = utils::hireTimeInSeconds();
            double frameTime = newTime - currentTime;
            if(frameTime > 0.25) frameTime = 0.25;
            currentTime = newTime;

            while(SDL_PollEvent(&event)) {
                 if(event.type == SDL_QUIT) gameRunning = false;
                 switch (currentGameState) {
                    case GameState::MAIN_MENU:
                        if (event.type == SDL_KEYDOWN) {
                            if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                                currentGameState = GameState::PLAYING; initializeGame();
                            } else if (event.key.keysym.sym == SDLK_ESCAPE) { gameRunning = false; }
                        }
                        break;
                    case GameState::PLAYING:
                         if (event.type == SDL_KEYDOWN) {
                             if (event.key.keysym.sym == SDLK_p && !event.key.repeat) {
                                 isPaused = !isPaused;
                                 if (isPaused) { if(isMusicPlaying) Mix_PauseMusic(); } else { if(isMusicPlaying) Mix_ResumeMusic(); }
                                 cout << (isPaused ? "GAME PAUSED" : "GAME RESUMED") << endl;
                             } else if (event.key.keysym.sym == SDLK_m && !event.key.repeat) {
                                 isMusicPlaying = !isMusicPlaying;
                                 if (isMusicPlaying) { if(Mix_PlayingMusic() == 0) Mix_PlayMusic(backgroundMusic, -1); else Mix_ResumeMusic(); cout << "Music Resumed/Started" << endl; }
                                 else { Mix_PauseMusic(); cout << "Music Paused" << endl; }
                             } else if (!isPaused && player_ptr) { player_ptr->handleKeyDown(event.key.keysym.sym); }
                             else if (event.key.keysym.sym == SDLK_ESCAPE) { gameRunning = false; }
                         }
                         break;
                     case GameState::WON:
                          if (event.type == SDL_KEYDOWN) {
                              if (event.key.keysym.sym == SDLK_ESCAPE) { gameRunning = false; }
                              else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) { currentGameState = GameState::MAIN_MENU; }
                          }
                          break;
                 }
            }

            if (currentGameState == GameState::PLAYING && !isPaused) {
                accumulator += frameTime;
                const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
                if(player_ptr) player_ptr->handleInput(currentKeyStates);

                while(accumulator >= timeStep) {
                    if(player_ptr) player_ptr->update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                    if(player_ptr) { vector2d& playerPos = player_ptr->getPos(); playerPos.x = max(cameraX, playerPos.x); }
                    for (Enemy& enemy_obj : enemies_list) { enemy_obj.update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT); }
                    for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); ) {
                         it_bullet->update(timeStep);
                         if (!it_bullet->isActive()) { it_bullet = bullets.erase(it_bullet); continue; }
                         SDL_Rect bulletHB = it_bullet->getWorldHitbox(); bool bullet_hit = false;
                         for (auto it_enemy = enemies_list.begin(); it_enemy != enemies_list.end(); ++it_enemy) {
                             if (it_enemy->isAlive()) {
                                 SDL_Rect enemyHB = it_enemy->getWorldHitbox();
                                 if (SDL_HasIntersection(&bulletHB, &enemyHB)) {
                                     it_enemy->takeHit(); it_bullet->setActive(false); bullet_hit = true;
                                     if(gEnemyDeathSound && it_enemy->isDead()) { Mix_PlayChannel(-1, gEnemyDeathSound, 0); }
                                     break;
                                 }
                             }
                         }
                         if (!bullet_hit) { it_bullet->checkMapCollision(mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT); }
                         if (!it_bullet->isActive()) { it_bullet = bullets.erase(it_bullet); } else { ++it_bullet; }
                    }
                    accumulator -= timeStep;
                }
                enemies_list.remove_if([](const Enemy& e){ return e.isDead(); });
                if (enemies_list.empty() && initialEnemyCount > 0 && !gameWon) {
                    currentGameState = GameState::WON; gameWon = true; Mix_HaltMusic(); isMusicPlaying = false;
                }
                if (player_ptr) {
                     vector2d bulletStartPos, bulletVelocity;
                     if (player_ptr->wantsToShoot(bulletStartPos, bulletVelocity)) {
                          bullets.emplace_back(bulletStartPos, bulletVelocity, bulletTexture);
                          if(shootSound) Mix_PlayChannel(-1, shootSound, 0);
                     }
                }
                if(player_ptr){
                    SDL_Rect playerHB = player_ptr->getWorldHitbox();
                    double playerCenterX = playerHB.x + playerHB.w / 2.0;
                    double targetCameraX = playerCenterX - static_cast<double>(SCREEN_WIDTH) / 2.0;
                    if (targetCameraX > cameraX) { cameraX = targetCameraX; }
                }
            }
             if(currentGameState != GameState::MAIN_MENU){
                cameraX = max(0.0, cameraX); cameraY = max(0.0, cameraY);
                if (BG_TEXTURE_WIDTH > SCREEN_WIDTH) { cameraX = min(cameraX, static_cast<double>(BG_TEXTURE_WIDTH - SCREEN_WIDTH)); } else { cameraX = 0.0; }
                if (BG_TEXTURE_HEIGHT > SCREEN_HEIGHT) { cameraY = min(cameraY, static_cast<double>(BG_TEXTURE_HEIGHT - SCREEN_HEIGHT)); } else { cameraY = 0.0; }
            }

            window.clear();
            switch (currentGameState) {
                case GameState::MAIN_MENU: {
                    SDL_RenderCopy(window.getRenderer(), menuBackgroundTexture, NULL, NULL);
                    SDL_Color textColor = {255, 255, 255, 255}; string menuText = "PRESS ENTER TO START";
                    SDL_Surface* textSurface = TTF_RenderText_Solid(menuFont, menuText.c_str(), textColor);
                    if (textSurface) {
                        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(window.getRenderer(), textSurface);
                        if (textTexture) { SDL_Rect textDestRect = {(SCREEN_WIDTH-textSurface->w)/2, SCREEN_HEIGHT-textSurface->h-60, textSurface->w, textSurface->h}; SDL_RenderCopy(window.getRenderer(), textTexture, NULL, &textDestRect); SDL_DestroyTexture(textTexture); }
                        SDL_FreeSurface(textSurface);
                    }
                    break;
                }
                case GameState::PLAYING: case GameState::WON: {
                    SDL_Rect bgSrcRect = {static_cast<int>(round(cameraX)), static_cast<int>(round(cameraY)), SCREEN_WIDTH, SCREEN_HEIGHT};
                    SDL_Rect bgDestRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                    if(backgroundTexture) SDL_RenderCopy(window.getRenderer(), backgroundTexture, &bgSrcRect, &bgDestRect);
                    SDL_Renderer* renderer = window.getRenderer();
                    #ifdef DEBUG_DRAW_GRID
                    if (renderer) {
                            SDL_Color oldColor_grid; SDL_GetRenderDrawColor(renderer, &oldColor_grid.r, &oldColor_grid.g, &oldColor_grid.b, &oldColor_grid.a);
                            SDL_BlendMode oldBlendMode_grid; SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode_grid);
                            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 255, 255, 255, 70);
                            int startCol_dbg_grid = static_cast<int>(floor(cameraX/LOGICAL_TILE_WIDTH)); int endCol_dbg_grid = startCol_dbg_grid + static_cast<int>(ceil((double)SCREEN_WIDTH/LOGICAL_TILE_WIDTH))+1;
                            int startRow_dbg_grid = static_cast<int>(floor(cameraY/LOGICAL_TILE_HEIGHT)); int endRow_dbg_grid = startRow_dbg_grid + static_cast<int>(ceil((double)SCREEN_HEIGHT/LOGICAL_TILE_HEIGHT))+1;
                            endCol_dbg_grid=min(endCol_dbg_grid, mapCols+1); endRow_dbg_grid=min(endRow_dbg_grid, mapRows+1);
                            for (int c=startCol_dbg_grid; c<endCol_dbg_grid; ++c) { int sx = static_cast<int>(round(c*LOGICAL_TILE_WIDTH-cameraX)); if(sx>=-LOGICAL_TILE_WIDTH && sx<=SCREEN_WIDTH+LOGICAL_TILE_WIDTH) SDL_RenderDrawLine(renderer,sx,0,sx,SCREEN_HEIGHT); }
                            for (int r=startRow_dbg_grid; r<endRow_dbg_grid; ++r) { int sy = static_cast<int>(round(r*LOGICAL_TILE_HEIGHT-cameraY)); if(sy>=-LOGICAL_TILE_HEIGHT && sy<=SCREEN_HEIGHT+LOGICAL_TILE_HEIGHT) SDL_RenderDrawLine(renderer,0,sy,SCREEN_WIDTH,sy); }
                            SDL_SetRenderDrawColor(renderer,oldColor_grid.r,oldColor_grid.g,oldColor_grid.b,oldColor_grid.a); SDL_SetRenderDrawBlendMode(renderer,oldBlendMode_grid);
                        }
                    #endif
                    #ifdef DEBUG_DRAW_COLUMNS
                    if (renderer && debugFont) {
                            SDL_Color oldC; SDL_GetRenderDrawColor(renderer,&oldC.r,&oldC.g,&oldC.b,&oldC.a);
                            SDL_Color colNumC={255,255,0,255}; SDL_Color tileTypeC={180,220,255,255};
                            int sCol=static_cast<int>(floor(cameraX/LOGICAL_TILE_WIDTH)); int eCol=sCol+static_cast<int>(ceil((double)SCREEN_WIDTH/LOGICAL_TILE_WIDTH));
                            int sRow=static_cast<int>(floor(cameraY/LOGICAL_TILE_HEIGHT)); int eRow=sRow+static_cast<int>(ceil((double)SCREEN_HEIGHT/LOGICAL_TILE_HEIGHT));
                            sCol=max(0,sCol);sRow=max(0,sRow);eCol=min(eCol,mapCols-1);eRow=min(eRow,mapRows-1);
                            for(int c=sCol;c<=eCol;++c){
                                int scrX_colS=static_cast<int>(round(c*LOGICAL_TILE_WIDTH-cameraX)); int scrX_txtOff=scrX_colS+5;
                                if(scrX_colS+LOGICAL_TILE_WIDTH>0 && scrX_colS<SCREEN_WIDTH){
                                    string cStr=std::to_string(c); SDL_Surface* surfC=TTF_RenderText_Solid(debugFont,cStr.c_str(),colNumC);
                                    if(surfC){SDL_Texture* texC=SDL_CreateTextureFromSurface(renderer,surfC); if(texC){SDL_Rect dC={scrX_txtOff,5,surfC->w,surfC->h}; SDL_RenderCopy(renderer,texC,NULL,&dC); SDL_DestroyTexture(texC);} SDL_FreeSurface(surfC);}
                                    for(int r=sRow;r<=eRow;++r){
                                        int scrY_rowS=static_cast<int>(round(r*LOGICAL_TILE_HEIGHT-cameraY)); int scrY_txtOff=scrY_rowS+5;
                                        if(scrY_rowS+LOGICAL_TILE_HEIGHT>0 && scrY_rowS<SCREEN_HEIGHT){
                                            int tType=-1; if(r<mapData.size()&&c<mapData[r].size()) tType=mapData[r][c];
                                            string tStr=std::to_string(tType); SDL_Surface* surfT=TTF_RenderText_Solid(debugFont,tStr.c_str(),tileTypeC);
                                            if(surfT){SDL_Texture* texT=SDL_CreateTextureFromSurface(renderer,surfT); if(texT){SDL_Rect dT={scrX_txtOff,scrY_txtOff,surfT->w,surfT->h}; SDL_RenderCopy(renderer,texT,NULL,&dT); SDL_DestroyTexture(texT);} SDL_FreeSurface(surfT);}
                                        }
                                    }
                                }
                            }SDL_SetRenderDrawColor(renderer,oldC.r,oldC.g,oldC.b,oldC.a);
                        }
                    #endif
                    for (Enemy& enemy_obj : enemies_list) { enemy_obj.render(window, cameraX, cameraY); }
                    for(Bullet& bullet : bullets) { bullet.render(window, cameraX, cameraY); }
                    if(player_ptr) player_ptr->render(window, cameraX, cameraY);
                    if (currentGameState == GameState::PLAYING && uiFont && renderer) {
                        SDL_Color uiC={255,255,255,255}; string ecTxt="Enemies: "+std::to_string(enemies_list.size());
                        SDL_Surface* s=TTF_RenderText_Solid(uiFont,ecTxt.c_str(),uiC);
                        if(s){SDL_Texture* t=SDL_CreateTextureFromSurface(renderer,s); if(t){SDL_Rect d={10,10,s->w,s->h}; SDL_RenderCopy(renderer,t,NULL,&d); SDL_DestroyTexture(t);} SDL_FreeSurface(s);}
                    }
                    if (isPaused && currentGameState == GameState::PLAYING && renderer && menuFont) {
                        SDL_Color ocp; SDL_GetRenderDrawColor(renderer,&ocp.r,&ocp.g,&ocp.b,&ocp.a); SDL_BlendMode obp; SDL_GetRenderDrawBlendMode(renderer,&obp);
                        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer,0,0,0,150);
                        SDL_Rect pO={0,0,SCREEN_WIDTH,SCREEN_HEIGHT}; SDL_RenderFillRect(renderer,&pO);
                        SDL_Color pTxtC={255,255,255,255}; string pTxt="PAUSED";
                        SDL_Surface* sp=TTF_RenderText_Solid(menuFont,pTxt.c_str(),pTxtC);
                        if(sp){SDL_Texture* tp=SDL_CreateTextureFromSurface(renderer,sp); if(tp){SDL_Rect dp={(SCREEN_WIDTH-sp->w)/2,(SCREEN_HEIGHT-sp->h)/2,sp->w,sp->h}; SDL_RenderCopy(renderer,tp,NULL,&dp); SDL_DestroyTexture(tp);}SDL_FreeSurface(sp);}
                        SDL_SetRenderDrawColor(renderer,ocp.r,ocp.g,ocp.b,ocp.a); SDL_SetRenderDrawBlendMode(renderer,obp);
                    } else if (currentGameState == GameState::WON && renderer && menuFont && uiFont) {
                        SDL_Color ocw; SDL_GetRenderDrawColor(renderer,&ocw.r,&ocw.g,&ocw.b,&ocw.a); SDL_BlendMode obw; SDL_GetRenderDrawBlendMode(renderer,&obw);
                        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer,0,200,0,120);
                        SDL_Rect wO={0,0,SCREEN_WIDTH,SCREEN_HEIGHT}; SDL_RenderFillRect(renderer,&wO);
                        SDL_Color wTxtC={255,255,0,255}; string wTxt1="YOU WIN!"; string wTxt2="Press Enter for Menu or ESC to Exit";
                        SDL_Surface* s1w=TTF_RenderText_Solid(menuFont,wTxt1.c_str(),wTxtC); SDL_Surface* s2w=TTF_RenderText_Solid(uiFont,wTxt2.c_str(),wTxtC);
                        if(s1w){SDL_Texture* t1w=SDL_CreateTextureFromSurface(renderer,s1w); if(t1w){SDL_Rect d1w={(SCREEN_WIDTH-s1w->w)/2,SCREEN_HEIGHT/2-s1w->h-10,s1w->w,s1w->h}; SDL_RenderCopy(renderer,t1w,NULL,&d1w); SDL_DestroyTexture(t1w);}SDL_FreeSurface(s1w);}
                        if(s2w){SDL_Texture* t2w=SDL_CreateTextureFromSurface(renderer,s2w); if(t2w){SDL_Rect d2w={(SCREEN_WIDTH-s2w->w)/2,SCREEN_HEIGHT/2+10,s2w->w,s2w->h}; SDL_RenderCopy(renderer,t2w,NULL,&d2w); SDL_DestroyTexture(t2w);}SDL_FreeSurface(s2w);}
                        SDL_SetRenderDrawColor(renderer,ocw.r,ocw.g,ocw.b,ocw.a); SDL_SetRenderDrawBlendMode(renderer,obw);
                    }
                    break;
                }
            }
            window.display();
            int frameTicks = SDL_GetTicks() - startTicks;
            if (frameTicks < 1000 / refreshRate) { SDL_Delay(1000 / refreshRate - frameTicks); }
        }

        cout << "Cleaning up resources..." << endl;
        if(player_ptr) delete player_ptr; enemies_list.clear(); bullets.clear();
        Mix_FreeMusic(backgroundMusic); Mix_FreeChunk(shootSound); Mix_FreeChunk(gEnemyDeathSound);
        SDL_DestroyTexture(menuBackgroundTexture); SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyTexture(playerRunTexture); SDL_DestroyTexture(playerJumpTexture);
        SDL_DestroyTexture(playerEnterWaterTexture); SDL_DestroyTexture(playerSwimTexture);
        SDL_DestroyTexture(playerAimUpTexture); SDL_DestroyTexture(playerAimDiagUpTexture);
        SDL_DestroyTexture(playerAimDiagDownTexture); SDL_DestroyTexture(playerRunAimDiagUpTexture);
        SDL_DestroyTexture(playerRunAimDiagDownTexture); SDL_DestroyTexture(playerStandShootHorizTexture);
        SDL_DestroyTexture(playerRunShootHorizTexture); SDL_DestroyTexture(playerShootUpTexture);
        SDL_DestroyTexture(playerShootDiagUpTexture); SDL_DestroyTexture(playerShootDiagDownTexture);
        SDL_DestroyTexture(playerLyingDownTexture); SDL_DestroyTexture(playerLyingShootTexture);
        SDL_DestroyTexture(bulletTexture); SDL_DestroyTexture(enemyTexture);
        TTF_CloseFont(uiFont); TTF_CloseFont(menuFont); TTF_CloseFont(debugFont);
        TTF_Quit(); Mix_CloseAudio(); Mix_Quit(); window.cleanUp(); IMG_Quit(); SDL_Quit();
        cout << "Cleanup complete. Exiting." << endl;
        return 0;
    }