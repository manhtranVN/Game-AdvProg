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
#include <memory> // For std::unique_ptr

#include "RenderWindow.hpp"
#include "math.hpp"
#include "utils.hpp"
#include "player.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "Turret.hpp"

using namespace std;

// --- Debug Flags ---
// #define DEBUG_DRAW_GRID
// #define DEBUG_DRAW_COLUMNS
// #define DEBUG_DRAW_PLAYER_HITBOX
// #define DEBUG_DRAW_HITBOXES

// --- Game State Enum ---
enum class GameState { MAIN_MENU, PLAYING, WON, GAME_OVER };

// --- Tile Logic Config ---
const int LOGICAL_TILE_WIDTH = 96;
const int LOGICAL_TILE_HEIGHT = 96;

// --- Map Data ---
vector<vector<int>> mapData = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,4,1,1,0,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,4,0,1,1,0,0,4,0,0,1,1,0,0,1,1,4,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,0,4,0,1,1,1,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,4,4,4,1,1,0,1,1,1,1,1,1,1,0,0,0,0,4,0,0,0,0,1,0,1,1,1,0,0,1,1,0,0,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,0,0},
    {0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,1,0},
    {3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,3,3,3,3,3,3,3,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1}
};

// Global sound chunks
Mix_Chunk* gEnemyDeathSound = nullptr;
Mix_Chunk* gPlayerDeathSound = nullptr;
Mix_Chunk* gTurretExplosionSound = nullptr;
Mix_Chunk* gTurretShootSound = nullptr;
Mix_Chunk* gBossMainExplosionSound = nullptr;

// --- Boss Related Globals ---
std::vector<std::unique_ptr<Turret>> bossPartsList;
const int BOSS_TOTAL_HP = 50;
int bossCurrentHP;
bool bossFightEngaged = false;
bool bossIsActuallyDefeated = false;
bool bossDefeatFullyProcessed = false;
float bossArenaTriggerX = 0.0f;
float bossEnemySpawnTimer = 0.0f;
const float BOSS_ENEMY_SPAWN_INTERVAL = 7.0f;
bool bossMainExplosionSoundPlayed = false;

// --- Hàm chính ---
int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) > 0) { cerr << "SDL_Init failed: " << SDL_GetError() << endl; return 1; }
    if (!IMG_Init(IMG_INIT_PNG)) { cerr << "IMG_Init failed: " << IMG_GetError() << endl; SDL_Quit(); return 1; }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << endl; IMG_Quit(); SDL_Quit(); return 1; }
    if (TTF_Init() == -1) { cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << endl; Mix_CloseAudio(); IMG_Quit(); SDL_Quit(); return 1; }
    cout << "SDL, IMG, Mixer, TTF initialized." << endl;

    const int SCREEN_WIDTH = 1024; const int SCREEN_HEIGHT = 672;
    RenderWindow window("Contra Clone Reloaded - Boss Fight", SCREEN_WIDTH, SCREEN_HEIGHT);
    int refreshRate = window.getRefreshRate(); if (refreshRate <= 0) refreshRate = 60;
    SDL_Renderer* renderer = window.getRenderer();

    TTF_Font* uiFont = TTF_OpenFont("res/font/kongtext.ttf", 24);
    TTF_Font* menuFont = TTF_OpenFont("res/font/kongtext.ttf", 28);
    TTF_Font* debugFont = TTF_OpenFont("res/font/kongtext.ttf", 16);
    if (!uiFont || !menuFont || !debugFont) { cerr << "Font load error: " << TTF_GetError() << endl; Mix_CloseAudio();TTF_Quit();IMG_Quit();SDL_Quit(); return 1; }

    if (!mapData.empty() && !mapData[0].empty()) {
        int lastCol = mapData[0].size() - 1;
        if (lastCol >=0) {
             cout << "Modifying map: last column (" << lastCol << ") for boss parts (0 -> 5)." << endl;
            for (size_t r = 0; r < mapData.size(); ++r) {
                if (r < mapData.size() && static_cast<size_t>(lastCol) < mapData[r].size()) {
                     if (mapData[r][lastCol] == 0) { mapData[r][lastCol] = 5; }
                }
            }
        }
    }

    SDL_Texture* menuBackgroundTexture = window.loadTexture("res/gfx/menu_background.png");
    SDL_Texture* backgroundTexture = window.loadTexture("res/gfx/ContraMapStage1BG.png");
    SDL_Texture* playerRunTexture = window.loadTexture("res/gfx/MainChar2.png");
    SDL_Texture* playerJumpTexture = window.loadTexture("res/gfx/Jumping.png");
    SDL_Texture* playerEnterWaterTexture = window.loadTexture("res/gfx/Watersplash.png");
    SDL_Texture* playerSwimTexture = window.loadTexture("res/gfx/Diving.png");
    SDL_Texture* playerStandAimShootHorizTexture = window.loadTexture("res/gfx/PlayerStandShoot.png");
    SDL_Texture* playerRunAimShootHorizTexture = window.loadTexture("res/gfx/Shooting.png");
    SDL_Texture* playerStandAimShootUpTexture = window.loadTexture("res/gfx/Shootingupward.png");
    SDL_Texture* playerStandAimShootDiagUpTexture = window.loadTexture("res/gfx/PlayerAimDiagUp.png");
    SDL_Texture* playerRunAimShootDiagUpTexture = window.loadTexture("res/gfx/PlayerShootDiagUp.png");
    SDL_Texture* playerStandAimShootDiagDownTexture = window.loadTexture("res/gfx/PlayerAimDiagDown.png");
    SDL_Texture* playerRunAimShootDiagDownTexture = window.loadTexture("res/gfx/PlayerShootDiagDown.png");
    SDL_Texture* playerLyingDownTexture = window.loadTexture("res/gfx/PlayerLyingShoot.png");
    SDL_Texture* playerLyingAimShootTexture = window.loadTexture("res/gfx/PlayerLyingShoot.png");
    SDL_Texture* playerBulletTexture = window.loadTexture("res/gfx/WBullet.png");
    SDL_Texture* turretBulletTexture = window.loadTexture("res/gfx/turret_bullet_sprite.png");
    SDL_Texture* enemyTexture = window.loadTexture("res/gfx/Enemy.png");
    SDL_Texture* gameTurretTexture = window.loadTexture("res/gfx/turret_texture.png");
    SDL_Texture* turretExplosionTexture = window.loadTexture("res/gfx/turret_explosion_texture.png");
    SDL_Texture* lifeMedalTexture = window.loadTexture("res/gfx/life_medal.png");

    Mix_Music* backgroundMusic = Mix_LoadMUS("res/snd/background_music.wav");
    Mix_Chunk* shootSound = Mix_LoadWAV("res/snd/player_shoot.wav");
    gEnemyDeathSound = Mix_LoadWAV("res/snd/enemy_death.wav");
    gPlayerDeathSound = Mix_LoadWAV("res/snd/player_death_sound.wav");
    gTurretExplosionSound = Mix_LoadWAV("res/snd/turret_explosion_sound.wav");
    gTurretShootSound = Mix_LoadWAV("res/snd/turret_shoot_sound.wav");
    gBossMainExplosionSound = Mix_LoadWAV("res/snd/turret_explosion_sound.wav"); // Đảm bảo file này tồn tại

    bool loadError = false;
    if (!menuBackgroundTexture || !backgroundTexture || !playerRunTexture || !playerJumpTexture ||
        !playerEnterWaterTexture || !playerSwimTexture || !playerStandAimShootHorizTexture ||
        !playerRunAimShootHorizTexture || !playerStandAimShootUpTexture || !playerStandAimShootDiagUpTexture ||
        !playerRunAimShootDiagUpTexture || !playerStandAimShootDiagDownTexture || !playerRunAimShootDiagDownTexture ||
        !playerLyingDownTexture || !playerLyingAimShootTexture ||
        !playerBulletTexture || !turretBulletTexture || !enemyTexture || !gameTurretTexture || !turretExplosionTexture ||
        !backgroundMusic || !shootSound || !gEnemyDeathSound || !gPlayerDeathSound ||
        !gTurretExplosionSound || !gTurretShootSound || !lifeMedalTexture || !gBossMainExplosionSound ) {
        loadError = true; cerr << "Error loading one or more resources!" << endl;
    }
    if (loadError) { /* Cleanup and exit */ return 1; }
    cout << "Resources loaded." << endl;

    int BG_TEXTURE_WIDTH = 0, BG_TEXTURE_HEIGHT = 0;
    if (backgroundTexture) SDL_QueryTexture(backgroundTexture, NULL, NULL, &BG_TEXTURE_WIDTH, &BG_TEXTURE_HEIGHT);

    const int PLAYER_STANDARD_FRAME_W = 40; const int PLAYER_STANDARD_FRAME_H = 78;
    const int PLAYER_LYING_FRAME_W = 78; const int PLAYER_LYING_FRAME_H = 40;
    const int PLAYER_RUN_SHEET_COLS = 6; const int PLAYER_JUMP_SHEET_COLS = 4;
    const int PLAYER_ENTER_WATER_SHEET_COLS = 1; const int PLAYER_SWIM_SHEET_COLS = 5;
    const int PLAYER_STAND_AIM_SHOOT_HORIZ_SHEET_COLS = 1; const int PLAYER_RUN_AIM_SHOOT_HORIZ_SHEET_COLS = 3;
    const int PLAYER_STAND_AIM_SHOOT_UP_SHEET_COLS = 2;
    const int PLAYER_STAND_AIM_SHOOT_DIAG_UP_SHEET_COLS = 1;
    const int PLAYER_RUN_AIM_SHOOT_DIAG_UP_SHEET_COLS = 3;
    const int PLAYER_STAND_AIM_SHOOT_DIAG_DOWN_SHEET_COLS = 1;
    const int PLAYER_RUN_AIM_SHOOT_DIAG_DOWN_SHEET_COLS = 3;
    const int PLAYER_LYING_DOWN_SHEET_COLS = 1; const int PLAYER_LYING_AIM_SHOOT_SHEET_COLS = 3;
    const int PLAYER_BULLET_RENDER_WIDTH = 12; const int PLAYER_BULLET_RENDER_HEIGHT = 6;

    const float INITIAL_CAMERA_X = 0.0f, INITIAL_CAMERA_Y = 0.0f;
    const float PLAYER_START_X = 100.0f; const float PLAYER_START_Y = 300.0f;
    const float PLAYER_RESPAWN_OFFSET_X = 150.0f;

    float cameraX = INITIAL_CAMERA_X, cameraY = INITIAL_CAMERA_Y;
    GameState currentGameState = GameState::MAIN_MENU;
    Player* player_ptr = nullptr;
    list<Bullet> playerBulletsList; list<Bullet> enemyBulletsList;
    list<Enemy> enemies_list; list<Turret> regularTurrets_list;
    int playerScore = 0;
    bool gameRunning = true, isPaused = false, isMusicEnabled = true; // Thêm isMusicEnabled
    const float timeStep = 0.01f; float accumulator = 0.0f;
    float currentTime_game = utils::hireTimeInSeconds();
    SDL_Event event;

    int mapRows = mapData.size();
    int mapCols = (mapRows > 0) ? mapData[0].size() : 0;
    if (mapCols == 0) { cerr << "Error: mapData is empty!" << endl; return 1; }
    cout << "Map: " << mapRows << "x" << mapCols << endl;

    int lastMapCol = mapCols > 0 ? mapCols - 1 : -1; // Khai báo ở đây để dùng chung

    bossArenaTriggerX = static_cast<float>((mapCols > 6 ? mapCols - 6 : 0) * LOGICAL_TILE_WIDTH);
    cout << "Boss Arena Trigger X: " << bossArenaTriggerX << endl;

    auto initializeGame = [&]() {
        cout << "Initializing Game State..." << endl;
        playerBulletsList.clear(); enemyBulletsList.clear(); enemies_list.clear(); regularTurrets_list.clear();
        bossPartsList.clear();

        playerScore = 0;
        vector2d initialPos = {PLAYER_START_X, PLAYER_START_Y};

        if (player_ptr) {
            player_ptr->resetPlayerStateForNewGame();
            player_ptr->setPos(initialPos);
            player_ptr->setInvulnerable(false);
        } else {
            player_ptr = new Player(
                initialPos,
                playerRunTexture, PLAYER_RUN_SHEET_COLS, playerJumpTexture, PLAYER_JUMP_SHEET_COLS,
                playerEnterWaterTexture, PLAYER_ENTER_WATER_SHEET_COLS, playerSwimTexture, PLAYER_SWIM_SHEET_COLS,
                playerStandAimShootUpTexture, PLAYER_STAND_AIM_SHOOT_UP_SHEET_COLS,
                playerStandAimShootDiagUpTexture, PLAYER_STAND_AIM_SHOOT_DIAG_UP_SHEET_COLS,
                playerStandAimShootDiagDownTexture, PLAYER_STAND_AIM_SHOOT_DIAG_DOWN_SHEET_COLS,
                playerRunAimShootDiagUpTexture, PLAYER_RUN_AIM_SHOOT_DIAG_UP_SHEET_COLS,
                playerRunAimShootDiagDownTexture, PLAYER_RUN_AIM_SHOOT_DIAG_DOWN_SHEET_COLS,
                playerStandAimShootHorizTexture, PLAYER_STAND_AIM_SHOOT_HORIZ_SHEET_COLS,
                playerRunAimShootHorizTexture, PLAYER_RUN_AIM_SHOOT_HORIZ_SHEET_COLS,
                playerLyingDownTexture, PLAYER_LYING_DOWN_SHEET_COLS,
                playerLyingAimShootTexture, PLAYER_LYING_AIM_SHOOT_SHEET_COLS,
                PLAYER_STANDARD_FRAME_W, PLAYER_STANDARD_FRAME_H,
                PLAYER_LYING_FRAME_W, PLAYER_LYING_FRAME_H
            );
            player_ptr->setInvulnerable(false);
        }

        cameraX = INITIAL_CAMERA_X; cameraY = INITIAL_CAMERA_Y;
        isPaused = false;
        isMusicEnabled = true; // Nhạc bật theo mặc định khi game mới bắt đầu

        bossCurrentHP = BOSS_TOTAL_HP;
        bossFightEngaged = false;
        bossIsActuallyDefeated = false;
        bossDefeatFullyProcessed = false;
        bossEnemySpawnTimer = BOSS_ENEMY_SPAWN_INTERVAL;
        bossMainExplosionSoundPlayed = false;

        auto spawnEnemyFunc = [&](float wx, int gr){ float eh=72.f; float gy=static_cast<float>(gr*LOGICAL_TILE_HEIGHT); float sy=gy-eh; enemies_list.emplace_back(vector2d{wx, sy}, enemyTexture); };
        spawnEnemyFunc(8.0f*LOGICAL_TILE_WIDTH, 3);
        spawnEnemyFunc(15.0f*LOGICAL_TILE_WIDTH, 3);
        spawnEnemyFunc(40.0f*LOGICAL_TILE_WIDTH, 2);

        for (int r = 0; r < mapRows; ++r) {
            for (int c = 0; c < mapCols; ++c) {
                if (mapData[r][c] == 5 && c == lastMapCol) {
                    float partX = static_cast<float>(c * LOGICAL_TILE_WIDTH);
                    float partY = static_cast<float>(r * LOGICAL_TILE_HEIGHT);
                    bossPartsList.emplace_back(std::make_unique<Turret>(
                        vector2d{partX, partY}, nullptr, turretExplosionTexture,
                        gTurretExplosionSound, gTurretShootSound, turretBulletTexture,
                        LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT
                    ));
                    if (!bossPartsList.empty()) bossPartsList.back()->setHp(9999);
                } else if (mapData[r][c] == 4) {
                    float tx = static_cast<float>(c * LOGICAL_TILE_WIDTH);
                    float ty = static_cast<float>(r * LOGICAL_TILE_HEIGHT);
                    regularTurrets_list.emplace_back(vector2d{tx, ty}, gameTurretTexture, turretExplosionTexture, gTurretExplosionSound, gTurretShootSound, turretBulletTexture, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                }
            }
        }
        cout << "Game Initialized. Spawned " << enemies_list.size() << " troops and " << regularTurrets_list.size() << " regular turrets." << endl;
        cout << "Boss has " << bossPartsList.size() << " vulnerable points." << endl;

        if (isMusicEnabled) {
            if (Mix_PlayingMusic() == 0) {
                if (Mix_PlayMusic(backgroundMusic, -1) == -1) { cerr << "Mix_PlayMusic Error: " << Mix_GetError() << endl; }
            } else if (Mix_PausedMusic() == 1) {
                Mix_ResumeMusic();
            }
        } else {
            Mix_PauseMusic(); // Nếu isMusicEnabled là false, đảm bảo nhạc bị pause
        }
    };

    // --- Game Loop ---
    while(gameRunning)
    {
        int startTicks = SDL_GetTicks();
        float newTime = static_cast<float>(utils::hireTimeInSeconds());
        float frameTime = newTime - currentTime_game; if(frameTime > 0.25f) frameTime = 0.25f; currentTime_game = newTime;

        while(SDL_PollEvent(&event))
        {
             if(event.type == SDL_QUIT) { gameRunning = false; }
             switch (currentGameState)
             {
                case GameState::MAIN_MENU:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                            currentGameState = GameState::PLAYING;
                            initializeGame();
                        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                            gameRunning = false;
                        }
                    }
                    break;
                case GameState::PLAYING:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_p && !event.key.repeat) {
                            isPaused = !isPaused;
                            if (isPaused) {
                                if (isMusicEnabled && Mix_PlayingMusic()) Mix_PauseMusic();
                            } else {
                                if (isMusicEnabled && Mix_PausedMusic()) Mix_ResumeMusic();
                            }
                            cout << (isPaused ? "PAUSED" : "RESUMED") << endl;
                        }
                        else if (event.key.keysym.sym == SDLK_m && !event.key.repeat) {
                            isMusicEnabled = !isMusicEnabled;
                            if (isMusicEnabled) {
                                if (!Mix_PlayingMusic() && backgroundMusic) {
                                     Mix_PlayMusic(backgroundMusic, -1);
                                     cout << "Music ON" << endl;
                                } else if (Mix_PausedMusic()) {
                                     Mix_ResumeMusic();
                                     cout << "Music Resumed (Music ON)" << endl;
                                } else {
                                    cout << "Music is already ON" << endl;
                                }
                            } else {
                                if (Mix_PlayingMusic()) {
                                    Mix_PauseMusic();
                                    cout << "Music OFF (Paused)" << endl;
                                } else {
                                    cout << "Music is already OFF" << endl;
                                }
                            }
                        }
                        else if (!isPaused && player_ptr) {
                            player_ptr->handleKeyDown(event.key.keysym.sym);
                        }
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            gameRunning = false;
                        }
                    }
                    break;
                case GameState::WON:
                case GameState::GAME_OVER:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            gameRunning = false;
                        } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                            currentGameState = GameState::MAIN_MENU;
                            // Khi quay lại menu, có thể muốn dừng nhạc game và phát nhạc menu (nếu có)
                            // Hoặc để initializeGame() xử lý nhạc
                            if (isMusicEnabled) Mix_HaltMusic(); // Dừng hẳn nhạc game
                        }
                    }
                    break;
             }
        }

        if (currentGameState == GameState::PLAYING && !isPaused)
        {
            accumulator += frameTime;
            const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL); if(player_ptr) player_ptr->handleInput(currentKeyStates);

            while(accumulator >= timeStep) {
                if(player_ptr) { player_ptr->update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT); player_ptr->getPos().x = std::max(cameraX, player_ptr->getPos().x); }
                for (Enemy& e : enemies_list) e.update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                for (Turret& t : regularTurrets_list) t.update(timeStep, player_ptr, enemyBulletsList);

                if (bossFightEngaged) {
                    if (!bossIsActuallyDefeated) {
                        for (auto& part : bossPartsList) { part->update(timeStep, player_ptr, enemyBulletsList); }
                        bossEnemySpawnTimer -= timeStep;
                        if (bossEnemySpawnTimer <= 0.0f && !bossPartsList.empty()) {
                            int randPartIdx = rand() % bossPartsList.size();
                            const auto& spawn_part_turret = bossPartsList[randPartIdx];
                            SDL_Rect part_hb = spawn_part_turret->getWorldHitbox();
                            vector2d spawnPos = {static_cast<float>(part_hb.x + part_hb.w / 2 - 20), static_cast<float>(part_hb.y - 10)};
                            enemies_list.emplace_back(spawnPos, enemyTexture);
                            bossEnemySpawnTimer = BOSS_ENEMY_SPAWN_INTERVAL + (static_cast<float>(rand() % 30 - 15) / 10.0f);
                        }
                    } else {
                        bool all_parts_exploded_finished = true;
                        for (auto& part : bossPartsList) {
                            part->update(timeStep, nullptr, enemyBulletsList);
                            if (!part->isFullyDestroyed()) { all_parts_exploded_finished = false; }
                        }
                        if (all_parts_exploded_finished && !bossDefeatFullyProcessed) {
                            bossDefeatFullyProcessed = true;
                            currentGameState = GameState::WON;
                            if(isMusicEnabled && Mix_PlayingMusic()) { Mix_HaltMusic(); } // Dừng nhạc khi thắng
                            cout << "--- BOSS FULLY DESTROYED! YOU WIN --- Final Score: " << playerScore << endl;
                        }
                    }
                }

                for (auto it_b = playerBulletsList.begin(); it_b != playerBulletsList.end(); ) {
                    it_b->update(timeStep);
                    if (!it_b->isActive()) { it_b = playerBulletsList.erase(it_b); continue; }
                    SDL_Rect bHB = it_b->getWorldHitbox(); bool hit = false;
                    for (auto it_e = enemies_list.begin(); it_e != enemies_list.end(); ++it_e) {
                        if (it_e->isAlive()) { SDL_Rect eHB = it_e->getWorldHitbox(); if (SDL_HasIntersection(&bHB, &eHB)) { bool wasA = it_e->isAlive(); it_e->takeHit(); if(wasA && !it_e->isAlive()) playerScore+=200; it_b->setActive(false); hit = true; break; } }
                    }
                    if (hit) { it_b = playerBulletsList.erase(it_b); continue; }
                    for (auto it_t = regularTurrets_list.begin(); it_t != regularTurrets_list.end(); ++it_t) {
                        if (it_t->getHp() > 0) { SDL_Rect tHB = it_t->getWorldHitbox(); if (SDL_HasIntersection(&bHB, &tHB)) { int hpB=it_t->getHp(); it_t->takeDamage(); if(hpB>0 && it_t->getHp()<=0) playerScore+=500; it_b->setActive(false); hit = true; break; } }
                    }
                    if (hit) { it_b = playerBulletsList.erase(it_b); continue; }
                    if (bossFightEngaged && !bossIsActuallyDefeated) {
                        for (auto& part_turret : bossPartsList) {
                            SDL_Rect partHB = part_turret->getWorldHitbox();
                            if (SDL_HasIntersection(&bHB, &partHB)) {
                                bossCurrentHP--; playerScore += 50; it_b->setActive(false); hit = true;
                                if (bossCurrentHP <= 0) {
                                    bossCurrentHP = 0; bossIsActuallyDefeated = true; playerScore += 1500;
                                    cout << "BOSS HP DEPLETED! Triggering part explosions." << endl;
                                    if (gBossMainExplosionSound && !bossMainExplosionSoundPlayed) { Mix_PlayChannel(-1, gBossMainExplosionSound, 0); bossMainExplosionSoundPlayed = true; }
                                    for (auto& exploding_part : bossPartsList) { exploding_part->forceExplode(); }
                                }
                                break;
                            }
                        }
                    }
                    if (hit) { it_b = playerBulletsList.erase(it_b); continue; }
                    ++it_b;
                }

                for (auto it_eb = enemyBulletsList.begin(); it_eb != enemyBulletsList.end(); ) {
                    it_eb->update(timeStep);
                    if (!it_eb->isActive()) { it_eb = enemyBulletsList.erase(it_eb); continue; }
                    if (player_ptr && !player_ptr->getIsDead() && !player_ptr->isInvulnerable()) { SDL_Rect ebHB = it_eb->getWorldHitbox(); SDL_Rect pHB = player_ptr->getWorldHitbox(); if (SDL_HasIntersection(&ebHB, &pHB)) { bool wasA=!player_ptr->getIsDead(); player_ptr->takeHit(false); if(gPlayerDeathSound && wasA && player_ptr->getIsDead()) Mix_PlayChannel(-1, gPlayerDeathSound, 0); it_eb->setActive(false);}}
                    if (!it_eb->isActive()) { it_eb = enemyBulletsList.erase(it_eb); } else { ++it_eb; }
                }
                accumulator -= timeStep;
            }

            enemies_list.remove_if([](const Enemy& e){ return e.isDead(); });
            regularTurrets_list.remove_if([](const Turret& t){ return t.isFullyDestroyed(); });

            if (player_ptr && player_ptr->getCurrentState() == PlayerState::DEAD) {
                if (player_ptr->getLives() > 0) { player_ptr->respawn(cameraX, PLAYER_START_Y, PLAYER_RESPAWN_OFFSET_X); }
                else { currentGameState = GameState::GAME_OVER; if(isMusicEnabled && Mix_PlayingMusic()) { Mix_HaltMusic(); } cout << "--- GAME OVER --- Final Score: " << playerScore << endl; }
            }

            if (player_ptr && !player_ptr->getIsDead() && !bossFightEngaged &&
                player_ptr->getPos().x + PLAYER_STANDARD_FRAME_W / 2.0f >= bossArenaTriggerX) {
                bossFightEngaged = true;
                cout << "BOSS FIGHT ENGAGED!" << endl;
            }

            if (player_ptr) {
                vector2d bs, bv; if (player_ptr->wantsToShoot(bs, bv)) { playerBulletsList.emplace_back(bs, bv, playerBulletTexture, PLAYER_BULLET_RENDER_WIDTH, PLAYER_BULLET_RENDER_HEIGHT); if(shootSound) Mix_PlayChannel(-1, shootSound, 0); }
            }

            if(player_ptr && !player_ptr->getIsDead()){
                SDL_Rect pHB = player_ptr->getWorldHitbox();
                float pCX = static_cast<float>(pHB.x + pHB.w / 2.0f);
                float targetCamX = pCX - static_cast<float>(SCREEN_WIDTH) / 2.5f;
                if (targetCamX > cameraX) cameraX = targetCamX;
            }
        }

        if(currentGameState != GameState::MAIN_MENU){
            cameraX = std::max(0.0f, cameraX);
            if (BG_TEXTURE_WIDTH > SCREEN_WIDTH) cameraX = std::min(cameraX, static_cast<float>(BG_TEXTURE_WIDTH - SCREEN_WIDTH));
            else cameraX = 0.0f;
        }

        window.clear();
        switch (currentGameState) {
            case GameState::MAIN_MENU: {
                SDL_RenderCopy(renderer, menuBackgroundTexture, NULL, NULL); SDL_Color tc={255,255,255,255}; string t="PRESS ENTER TO START"; SDL_Surface* s=TTF_RenderText_Solid(menuFont,t.c_str(),tc); if(s){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s); if(tx){ SDL_Rect d={(SCREEN_WIDTH-s->w)/2, SCREEN_HEIGHT-s->h-80, s->w, s->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); } SDL_FreeSurface(s);}
            } break;
            case GameState::PLAYING:
            case GameState::WON:
            case GameState::GAME_OVER: {
                SDL_Rect bgSrc={static_cast<int>(round(cameraX)), static_cast<int>(round(cameraY)), SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_Rect bgDst={0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
                if(backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, &bgSrc, &bgDst);

                #ifdef DEBUG_DRAW_GRID
                // ...
                #endif
                #ifdef DEBUG_DRAW_COLUMNS
                // ...
                #endif

                for (Enemy& e : enemies_list) e.render(window, cameraX, cameraY);
                for (Turret& t : regularTurrets_list) t.render(window, cameraX, cameraY);
                if (bossFightEngaged || bossIsActuallyDefeated) {
                    for (auto& part_turret : bossPartsList) { part_turret->render(window, cameraX, cameraY); }
                }
                for (Bullet& b : playerBulletsList) b.render(window, cameraX, cameraY);
                for (Bullet& eb : enemyBulletsList) eb.render(window, cameraX, cameraY);
                if (player_ptr) player_ptr->render(window, cameraX, cameraY);

                if (uiFont && renderer) {
                    SDL_Color uiTextColor = {255,255,255,255};
                    string scoreStr = "SCORE: " + std::to_string(playerScore);
                    SDL_Surface* scoreSurface = TTF_RenderText_Solid(uiFont, scoreStr.c_str(), uiTextColor);
                    int scoreTextHeight = 0;
                    if(scoreSurface){ SDL_Texture* scoreTex = SDL_CreateTextureFromSurface(renderer,scoreSurface); SDL_Rect scoreRect = {10,10,scoreSurface->w,scoreSurface->h}; SDL_RenderCopy(renderer,scoreTex,NULL,&scoreRect); scoreTextHeight = scoreSurface->h; SDL_DestroyTexture(scoreTex); SDL_FreeSurface(scoreSurface); }
                    if (player_ptr && lifeMedalTexture) { int livesLeft = player_ptr->getLives(); if (livesLeft > 0) { const int MEDAL_RENDER_WIDTH = 20; const int MEDAL_RENDER_HEIGHT = 40; int spacing = 5, topM = 10, rightM = 10; for (int i = 0; i < livesLeft; ++i) { SDL_Rect dRect = {SCREEN_WIDTH - rightM - (i + 1) * MEDAL_RENDER_WIDTH - i * spacing, topM, MEDAL_RENDER_WIDTH, MEDAL_RENDER_HEIGHT}; SDL_RenderCopy(renderer, lifeMedalTexture, NULL, &dRect); }}}
                    if (bossFightEngaged && !bossDefeatFullyProcessed && !bossIsActuallyDefeated) {
                        SDL_Color cGood = {0, 200, 0, 255}, cMid = {255, 200, 0, 255}, cBad = {200, 0, 0, 255};
                        SDL_Color cBg = {50, 50, 50, 220}, cBorder = {220, 220, 220, 255};
                        int barW = SCREEN_WIDTH / 2, barH = 20;
                        int barX = (SCREEN_WIDTH - barW) / 2, barY = 10 + scoreTextHeight + 5;
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        SDL_Rect bgR = {barX, barY, barW, barH}; SDL_SetRenderDrawColor(renderer, cBg.r, cBg.g, cBg.b, cBg.a); SDL_RenderFillRect(renderer, &bgR);
                        float hpP = (BOSS_TOTAL_HP > 0) ? (static_cast<float>(std::max(0, bossCurrentHP)) / BOSS_TOTAL_HP) : 0.0f;
                        SDL_Rect fgR = {barX, barY, static_cast<int>(barW * hpP), barH};
                        SDL_Color curC = (hpP > 0.6f) ? cGood : (hpP > 0.3f ? cMid : cBad);
                        SDL_SetRenderDrawColor(renderer, curC.r, curC.g, curC.b, curC.a); SDL_RenderFillRect(renderer, &fgR);
                        SDL_SetRenderDrawColor(renderer, cBorder.r, cBorder.g, cBorder.b, cBorder.a); SDL_RenderDrawRect(renderer, &bgR);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                        string bossName = "FINAL GUARDIAN";
                        SDL_Surface* bnSurf = TTF_RenderText_Solid(uiFont, bossName.c_str(), cBorder);
                        if(bnSurf){ SDL_Texture* bnTex = SDL_CreateTextureFromSurface(renderer, bnSurf); SDL_Rect bnR = {barX + (barW - bnSurf->w)/2 , barY + (barH - bnSurf->h)/2, bnSurf->w, bnSurf->h}; SDL_RenderCopy(renderer, bnTex, NULL, &bnR); SDL_DestroyTexture(bnTex); SDL_FreeSurface(bnSurf); }
                    }
                }

                if (isPaused && currentGameState == GameState::PLAYING) { SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer,0,0,0,150); SDL_Rect pO={0,0,SCREEN_WIDTH,SCREEN_HEIGHT}; SDL_RenderFillRect(renderer,&pO); SDL_Color pC={255,255,255,255}; string pT="PAUSED"; SDL_Surface* sP=TTF_RenderText_Solid(menuFont,pT.c_str(),pC); if(sP){SDL_Texture* tP=SDL_CreateTextureFromSurface(renderer,sP); SDL_Rect dP={(SCREEN_WIDTH-sP->w)/2,(SCREEN_HEIGHT-sP->h)/2,sP->w,sP->h}; SDL_RenderCopy(renderer,tP,NULL,&dP); SDL_DestroyTexture(tP); SDL_FreeSurface(sP);} }
                else if (currentGameState == GameState::WON) { SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 0, 180, 0, 170); SDL_Rect r={0,0,SCREEN_WIDTH,SCREEN_HEIGHT}; SDL_RenderFillRect(renderer,&r); SDL_Color c={255,255,0,255}; string t1="VICTORY!"; string tS="FINAL SCORE: "+std::to_string(playerScore); string t2="Press Enter or ESC"; SDL_Surface* s1=TTF_RenderText_Solid(menuFont,t1.c_str(),c); SDL_Surface* sS_surf=TTF_RenderText_Solid(uiFont,tS.c_str(),c); SDL_Surface* s2=TTF_RenderText_Solid(uiFont,t2.c_str(),c); int yP=SCREEN_HEIGHT/2-((s1?s1->h:0)+(sS_surf?sS_surf->h:0)+(s2?s2->h:0)+20)/2; if(s1){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s1); SDL_Rect d={(SCREEN_WIDTH-s1->w)/2, yP, s1->w,s1->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(s1); yP+=d.h+10;} if(sS_surf){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,sS_surf); SDL_Rect d={(SCREEN_WIDTH-sS_surf->w)/2, yP, sS_surf->w,sS_surf->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(sS_surf); yP+=d.h+10;} if(s2){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s2); SDL_Rect d={(SCREEN_WIDTH-s2->w)/2, yP, s2->w,s2->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(s2);} }
                else if (currentGameState == GameState::GAME_OVER) { SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 180, 0, 0, 170); SDL_Rect r={0,0,SCREEN_WIDTH,SCREEN_HEIGHT}; SDL_RenderFillRect(renderer,&r); SDL_Color c={255,255,255,255}; string t1="GAME OVER"; string tS="FINAL SCORE: "+std::to_string(playerScore); string t2="Press Enter or ESC"; SDL_Surface* s1=TTF_RenderText_Solid(menuFont,t1.c_str(),c); SDL_Surface* sS_surf=TTF_RenderText_Solid(uiFont,tS.c_str(),c); SDL_Surface* s2=TTF_RenderText_Solid(uiFont,t2.c_str(),c); int yP=SCREEN_HEIGHT/2-((s1?s1->h:0)+(sS_surf?sS_surf->h:0)+(s2?s2->h:0)+20)/2; if(s1){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s1); SDL_Rect d={(SCREEN_WIDTH-s1->w)/2, yP, s1->w,s1->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(s1); yP+=d.h+10;} if(sS_surf){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,sS_surf); SDL_Rect d={(SCREEN_WIDTH-sS_surf->w)/2, yP, sS_surf->w,sS_surf->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(sS_surf); yP+=d.h+10;} if(s2){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s2); SDL_Rect d={(SCREEN_WIDTH-s2->w)/2, yP, s2->w,s2->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(s2);} }
            } break;
        }
        window.display();

        float frameTicks_render = static_cast<float>(SDL_GetTicks()) - startTicks;
        float desiredFrameTime_ms = 1000.0f / refreshRate;
        if (frameTicks_render < desiredFrameTime_ms) { SDL_Delay(static_cast<Uint32>(desiredFrameTime_ms - frameTicks_render)); }
    }

    cout << "Cleaning up resources..." << endl;
    delete player_ptr; player_ptr = nullptr;
    enemies_list.clear(); regularTurrets_list.clear();
    bossPartsList.clear();
    playerBulletsList.clear(); enemyBulletsList.clear();

    Mix_FreeMusic(backgroundMusic); Mix_FreeChunk(shootSound); Mix_FreeChunk(gEnemyDeathSound);
    Mix_FreeChunk(gPlayerDeathSound); Mix_FreeChunk(gTurretExplosionSound); Mix_FreeChunk(gTurretShootSound);
    Mix_FreeChunk(gBossMainExplosionSound);

    SDL_DestroyTexture(menuBackgroundTexture); SDL_DestroyTexture(backgroundTexture); SDL_DestroyTexture(playerRunTexture); SDL_DestroyTexture(playerJumpTexture); SDL_DestroyTexture(playerEnterWaterTexture); SDL_DestroyTexture(playerSwimTexture); SDL_DestroyTexture(playerStandAimShootHorizTexture); SDL_DestroyTexture(playerRunAimShootHorizTexture); SDL_DestroyTexture(playerStandAimShootUpTexture); SDL_DestroyTexture(playerStandAimShootDiagUpTexture); SDL_DestroyTexture(playerRunAimShootDiagUpTexture); SDL_DestroyTexture(playerStandAimShootDiagDownTexture); SDL_DestroyTexture(playerRunAimShootDiagDownTexture); SDL_DestroyTexture(playerLyingDownTexture); SDL_DestroyTexture(playerLyingAimShootTexture); SDL_DestroyTexture(playerBulletTexture); SDL_DestroyTexture(turretBulletTexture); SDL_DestroyTexture(enemyTexture); SDL_DestroyTexture(gameTurretTexture); SDL_DestroyTexture(turretExplosionTexture);
    SDL_DestroyTexture(lifeMedalTexture);

    TTF_CloseFont(uiFont); TTF_CloseFont(menuFont); TTF_CloseFont(debugFont);

    TTF_Quit(); Mix_CloseAudio(); Mix_Quit();
    window.cleanUp(); IMG_Quit(); SDL_Quit();
    cout << "Cleanup complete. Exiting." << endl;

    return 0;
}