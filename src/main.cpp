#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm> // For std::max, std::min, std::remove_if
#include <list>
#include <string>    // For std::to_string
#include <memory>    // Có thể dùng sau này

// Bao gồm các header của dự án
#include "RenderWindow.hpp" 
#include "math.hpp"
#include "utils.hpp"
#include "player.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "Turret.hpp"

using namespace std;

// --- Debug Flags ---
//#define DEBUG_DRAW_GRID     
//#define DEBUG_DRAW_COLUMNS 
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
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,4,0,1,1,0,0,4,0,0,1,1,0,0,1,1,4,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,0,4,0,1,1,1,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,4,4,4,1,1,0,1,1,1,1,1,1,1,0,0,0,0,4,0,0,0,0,1,0,1,1,1,0,0,1,1,0,0,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,0,0},
    {0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,1,0},
    {3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,3,3,3,3,3,3,3,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1}
};

// Global sound chunks
Mix_Chunk* gEnemyDeathSound = nullptr;
Mix_Chunk* gPlayerDeathSound = nullptr;
Mix_Chunk* gTurretExplosionSound = nullptr;
Mix_Chunk* gTurretShootSound = nullptr;


// --- Hàm chính ---
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
    SDL_Renderer* renderer = window.getRenderer(); 

    TTF_Font* uiFont = TTF_OpenFont("res/font/kongtext.ttf", 24);
    TTF_Font* menuFont = TTF_OpenFont("res/font/kongtext.ttf", 28);
    TTF_Font* debugFont = TTF_OpenFont("res/font/kongtext.ttf", 16);
    if (!uiFont || !menuFont || !debugFont) { cerr << "Font load error: " << TTF_GetError() << endl; Mix_CloseAudio();TTF_Quit();IMG_Quit();SDL_Quit(); return 1; }
    cout << "Fonts loaded." << endl;

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
    SDL_Texture* lifeMedalTexture = window.loadTexture("res/gfx/life_medal.png"); // ĐÃ THÊM Ở ĐÂY


    Mix_Music* backgroundMusic = Mix_LoadMUS("res/snd/background_music.wav");
    Mix_Chunk* shootSound = Mix_LoadWAV("res/snd/player_shoot.wav");
    gEnemyDeathSound = Mix_LoadWAV("res/snd/enemy_death.wav");
    gPlayerDeathSound = Mix_LoadWAV("res/snd/player_death_sound.wav");
    gTurretExplosionSound = Mix_LoadWAV("res/snd/turret_explosion_sound.wav");
    gTurretShootSound = Mix_LoadWAV("res/snd/turret_shoot_sound.wav");

    bool loadError = false;
    if (!menuBackgroundTexture || !backgroundTexture || !playerRunTexture || !playerJumpTexture ||
        !playerEnterWaterTexture || !playerSwimTexture || !playerStandAimShootHorizTexture ||
        !playerRunAimShootHorizTexture || !playerStandAimShootUpTexture || !playerStandAimShootDiagUpTexture ||
        !playerRunAimShootDiagUpTexture || !playerStandAimShootDiagDownTexture || !playerRunAimShootDiagDownTexture ||
        !playerLyingDownTexture || !playerLyingAimShootTexture ||
        !playerBulletTexture || !turretBulletTexture || !enemyTexture || !gameTurretTexture || !turretExplosionTexture ||
        !backgroundMusic || !shootSound || !gEnemyDeathSound || !gPlayerDeathSound ||
        !gTurretExplosionSound || !gTurretShootSound || !lifeMedalTexture) { // ĐÃ THÊM KIỂM TRA lifeMedalTexture
        loadError = true; cerr << "Error loading one or more resources!" << endl;
    }
    if (loadError) { Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit(); return 1; }
    cout << "Resources loaded." << endl;

    int BG_TEXTURE_WIDTH = 0, BG_TEXTURE_HEIGHT = 0;
    if(backgroundTexture) SDL_QueryTexture(backgroundTexture, NULL, NULL, &BG_TEXTURE_WIDTH, &BG_TEXTURE_HEIGHT);

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
    const int PLAYER_BULLET_RENDER_WIDTH = 12; // Kích thước đạn người chơi
    const int PLAYER_BULLET_RENDER_HEIGHT = 6;  // Kích thước đạn người chơi
    
    const float INITIAL_CAMERA_X = 0.0f, INITIAL_CAMERA_Y = 0.0f;
    const float PLAYER_START_X = 100.0f; const float PLAYER_START_Y = 300.0f; 
    const float PLAYER_RESPAWN_OFFSET_X = 150.0f;

    float cameraX = INITIAL_CAMERA_X, cameraY = INITIAL_CAMERA_Y;
    GameState currentGameState = GameState::MAIN_MENU;
    Player* player_ptr = nullptr; 
    list<Bullet> playerBulletsList; list<Bullet> enemyBulletsList;
    list<Enemy> enemies_list; list<Turret> turrets_list;
    int playerScore = 0; float gameWinConditionX = 0.0f;
    bool gameRunning = true, isPaused = false, gameWonFlag = false, isMusicPlaying = false;
    const float timeStep = 0.01f; float accumulator = 0.0f;
    float currentTime_game = static_cast<float>(utils::hireTimeInSeconds());
    SDL_Event event;

    auto initializeGame = [&]() {
        cout << "Initializing Game State..." << endl;
        playerBulletsList.clear(); enemyBulletsList.clear(); enemies_list.clear(); turrets_list.clear();
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
        gameWonFlag = false; isPaused = false;

        auto spawnEnemy = [&](float wx, int gr){ float eh=72.f; float gy=static_cast<float>(gr*LOGICAL_TILE_HEIGHT); float sy=gy-eh; enemies_list.emplace_back(vector2d{wx, sy}, enemyTexture); };
        spawnEnemy(8.0f*LOGICAL_TILE_WIDTH, 3); spawnEnemy(15.0f*LOGICAL_TILE_WIDTH, 3); spawnEnemy(40.0f*LOGICAL_TILE_WIDTH, 2);
        for (size_t r = 0; r < mapData.size(); ++r) { for (size_t c = 0; c < mapData[r].size(); ++c) { if (mapData[r][c] == 4) { float tx=static_cast<float>(c*LOGICAL_TILE_WIDTH); float ty=static_cast<float>(r*LOGICAL_TILE_HEIGHT); turrets_list.emplace_back(vector2d{tx, ty}, gameTurretTexture, turretExplosionTexture, gTurretExplosionSound, gTurretShootSound, turretBulletTexture, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT); } } }
        cout << "Game Initialized. Spawned " << enemies_list.size() << " troops and " << turrets_list.size() << " turrets." << endl;

        if (!Mix_PlayingMusic()) { if (Mix_PlayMusic(backgroundMusic, -1) == -1) { cerr << "Mix_PlayMusic Error: " << Mix_GetError() << endl; } else isMusicPlaying = true; }
        else if (!isMusicPlaying) { Mix_ResumeMusic(); isMusicPlaying = true; }
    };

    int mapRows = mapData.size();
    int mapCols = (mapRows > 0) ? mapData[0].size() : 0;
    if (mapCols == 0) { cerr << "Error: mapData is empty!" << endl; return 1; }
    cout << "Map: " << mapRows << "x" << mapCols << endl;
    gameWinConditionX = static_cast<float>((mapCols > 3 ? mapCols - 3 : (mapCols > 0 ? mapCols -1 : 0)) * LOGICAL_TILE_WIDTH);
    cout << "Win condition X: " << gameWinConditionX << endl;

    while(gameRunning) {
        int startTicks = SDL_GetTicks();
        float newTime = static_cast<float>(utils::hireTimeInSeconds());
        float frameTime = newTime - currentTime_game; if(frameTime > 0.25f) frameTime = 0.25f; currentTime_game = newTime;

        while(SDL_PollEvent(&event)) {
             if(event.type == SDL_QUIT) { gameRunning = false; }
             switch (currentGameState) {
                case GameState::MAIN_MENU: if (event.type == SDL_KEYDOWN) { if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) { currentGameState = GameState::PLAYING; initializeGame(); } else if (event.key.keysym.sym == SDLK_ESCAPE) { gameRunning = false; } } break;
                case GameState::PLAYING: if (event.type == SDL_KEYDOWN) { if (event.key.keysym.sym == SDLK_p && !event.key.repeat) { isPaused = !isPaused; if (isPaused) { if(isMusicPlaying && Mix_PlayingMusic()) Mix_PauseMusic(); } else { if(isMusicPlaying && Mix_PausedMusic()) Mix_ResumeMusic(); } cout << (isPaused ? "PAUSED" : "RESUMED") << endl; } else if (event.key.keysym.sym == SDLK_m && !event.key.repeat) { isMusicPlaying = !isMusicPlaying; if (isMusicPlaying){ if(!Mix_PlayingMusic()) Mix_PlayMusic(backgroundMusic,-1); else if(Mix_PausedMusic()) Mix_ResumeMusic(); cout<<"Music On"<<endl;} else { if(Mix_PlayingMusic()) Mix_PauseMusic(); cout<<"Music Off"<<endl;} } else if (!isPaused && player_ptr) { player_ptr->handleKeyDown(event.key.keysym.sym); } if (event.key.keysym.sym == SDLK_ESCAPE) { gameRunning = false; } } break;
                 case GameState::WON: case GameState::GAME_OVER: if (event.type == SDL_KEYDOWN) { if (event.key.keysym.sym == SDLK_ESCAPE) { gameRunning = false; } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) { currentGameState = GameState::MAIN_MENU; } } break;
             }
        }

        if (currentGameState == GameState::PLAYING && !isPaused) {
            accumulator += frameTime;
            const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL); if(player_ptr) player_ptr->handleInput(currentKeyStates);
            while(accumulator >= timeStep) {
                 if(player_ptr) { player_ptr->update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT); player_ptr->getPos().x = std::max(cameraX, player_ptr->getPos().x); }
                for (Enemy& e : enemies_list) e.update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                for (Turret& t : turrets_list) t.update(timeStep, player_ptr, enemyBulletsList);
                for (auto it_b = playerBulletsList.begin(); it_b != playerBulletsList.end(); ) { 
                    it_b->update(timeStep); 
                    if (!it_b->isActive()) { 
                        it_b = playerBulletsList.erase(it_b); 
                        continue; 
                    } 
                    SDL_Rect bHB = it_b->getWorldHitbox(); 
                    bool hit = false; 
                    for (auto it_e = enemies_list.begin(); it_e != enemies_list.end(); ++it_e) { 
                        if (it_e->isAlive()) { 
                            SDL_Rect eHB = it_e->getWorldHitbox(); 
                            if (SDL_HasIntersection(&bHB, &eHB)) { 
                                bool wasA = it_e->isAlive(); 
                                it_e->takeHit(); 
                                if(wasA && !it_e->isAlive()) playerScore+=200; 
                                it_b->setActive(false); 
                                hit = true; 
                                break; 
                            } 
                        } 
                    } 
                    if (hit) { 
                        it_b = playerBulletsList.erase(it_b); 
                        continue; 
                    } 
                    for (auto it_t = turrets_list.begin(); it_t != turrets_list.end(); ++it_t) { 
                        if (it_t->getHp() > 0) { // Chỉ va chạm với Turret còn sống
                            SDL_Rect tHB = it_t->getWorldHitbox(); 
                            if (SDL_HasIntersection(&bHB, &tHB)) { 
                                int hpB=it_t->getHp(); 
                                it_t->takeDamage(); 
                                if(hpB>0 && it_t->getHp()<=0) playerScore+=500; 
                                it_b->setActive(false); 
                                hit = true; 
                                break; 
                            } 
                        } 
                    } 
                    if (hit) { 
                        it_b = playerBulletsList.erase(it_b); 
                        continue; 
                    } 
                    // SỬA ĐỔI: XÓA HOẶC COMMENT OUT DÒNG NÀY
                    // if (!hit) { 
                    //    it_b->checkMapCollision(mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT); 
                    // } 
                    
                    if (!it_b->isActive()) { // Kiểm tra lại active sau khi các va chạm (nếu có)
                        it_b = playerBulletsList.erase(it_b); 
                    } else { 
                        ++it_b; 
                    } 
                }

                // Enemy Bullets Collisions...
                for (auto it_eb = enemyBulletsList.begin(); it_eb != enemyBulletsList.end(); ) { 
                    it_eb->update(timeStep); 
                    if (!it_eb->isActive()) { 
                        it_eb = enemyBulletsList.erase(it_eb); 
                        continue; 
                    } 
                    if (player_ptr && !player_ptr->getIsDead() && !player_ptr->isInvulnerable()) { 
                        SDL_Rect ebHB = it_eb->getWorldHitbox(); 
                        SDL_Rect pHB = player_ptr->getWorldHitbox(); 
                        if (SDL_HasIntersection(&ebHB, &pHB)) { 
                            bool wasA=!player_ptr->getIsDead(); 
                            player_ptr->takeHit(false); 
                            if(gPlayerDeathSound && wasA && player_ptr->getIsDead()) Mix_PlayChannel(-1, gPlayerDeathSound, 0); 
                            it_eb->setActive(false); 
                        } 
                    } 
                    // SỬA ĐỔI: XÓA HOẶC COMMENT OUT DÒNG NÀY
                    // if (it_eb->isActive()) { 
                    //    it_eb->checkMapCollision(mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT); 
                    // } 
                    
                    if (!it_eb->isActive()) { // Kiểm tra lại active sau khi các va chạm (nếu có)
                        it_eb = enemyBulletsList.erase(it_eb); 
                    } else { 
                        ++it_eb; 
                    } 
                }
                accumulator -= timeStep;
            }
            enemies_list.remove_if([](const Enemy& e){ return e.isDead(); }); turrets_list.remove_if([](const Turret& t){ return t.isFullyDestroyed(); });

            if (player_ptr && player_ptr->getCurrentState() == PlayerState::DEAD) {
                if (player_ptr->getLives() > 0) {
                    player_ptr->respawn(cameraX, PLAYER_START_Y, PLAYER_RESPAWN_OFFSET_X);
                }
                else { currentGameState = GameState::GAME_OVER; if(isMusicPlaying && Mix_PlayingMusic()) { Mix_HaltMusic(); isMusicPlaying = false; } cout << "--- GAME OVER --- Final Score: " << playerScore << endl; }
            } else if (player_ptr && !player_ptr->getIsDead() && player_ptr->getPos().x + PLAYER_STANDARD_FRAME_W/2.0f >= gameWinConditionX && !gameWonFlag ) {
                 currentGameState = GameState::WON; gameWonFlag = true; if(isMusicPlaying && Mix_PlayingMusic()) { Mix_HaltMusic(); isMusicPlaying = false; } cout << "--- YOU WIN --- Final Score: " << playerScore << endl;
            }
            if (player_ptr) { 
                vector2d bs, bv; 
                if (player_ptr->wantsToShoot(bs, bv)) { 
                    playerBulletsList.emplace_back(bs, bv, playerBulletTexture, 
                                                   PLAYER_BULLET_RENDER_WIDTH, PLAYER_BULLET_RENDER_HEIGHT); 
                    if(shootSound) Mix_PlayChannel(-1, shootSound, 0); 
                } 
            }
            if(player_ptr && !player_ptr->getIsDead()){ SDL_Rect pHB = player_ptr->getWorldHitbox(); float pCX = static_cast<float>(pHB.x + pHB.w / 2.0f); float tCX = pCX - static_cast<float>(SCREEN_WIDTH) / 2.5f; if (tCX > cameraX) { cameraX = tCX; } }
        }

         if(currentGameState != GameState::MAIN_MENU){ cameraX = std::max(0.0f, cameraX); if (BG_TEXTURE_WIDTH > SCREEN_WIDTH) cameraX = std::min(cameraX, static_cast<float>(BG_TEXTURE_WIDTH - SCREEN_WIDTH)); else cameraX = 0.0f; }

        window.clear();
        switch (currentGameState) {
            case GameState::MAIN_MENU: { SDL_RenderCopy(renderer, menuBackgroundTexture, NULL, NULL); SDL_Color tc={255,255,255,255}; string t="PRESS ENTER TO START"; SDL_Surface* s=TTF_RenderText_Solid(menuFont,t.c_str(),tc); if(s){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s); if(tx){ SDL_Rect d={(SCREEN_WIDTH-s->w)/2, SCREEN_HEIGHT-s->h-80, s->w, s->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); } SDL_FreeSurface(s);} } break;
            case GameState::PLAYING: case GameState::WON: case GameState::GAME_OVER: { 
                SDL_Rect bgSrc={static_cast<int>(round(cameraX)), static_cast<int>(round(cameraY)), SCREEN_WIDTH, SCREEN_HEIGHT}; SDL_Rect bgDst={0,0,SCREEN_WIDTH,SCREEN_HEIGHT}; if(backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, &bgSrc, &bgDst);

                #ifdef DEBUG_DRAW_GRID
                if (renderer) { 
                     SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 255, 255, 255, 70);
                     int scg=static_cast<int>(floor(cameraX/LOGICAL_TILE_WIDTH)), ecg=scg+static_cast<int>(ceil((float)SCREEN_WIDTH/LOGICAL_TILE_WIDTH))+1;
                     for(int c=scg; c<ecg; ++c){ int sx=static_cast<int>(round(c*LOGICAL_TILE_WIDTH-cameraX)); SDL_RenderDrawLine(renderer,sx,0,sx,SCREEN_HEIGHT); }
                     for(int r=0; r<mapRows+1; ++r){ int sy=static_cast<int>(round(r*LOGICAL_TILE_HEIGHT-cameraY)); SDL_RenderDrawLine(renderer,0,sy,SCREEN_WIDTH,sy); }
                }
                #endif 

                 #ifdef DEBUG_DRAW_COLUMNS
                if (renderer && debugFont) {
                    SDL_Color textColor = {255, 255, 0, 255}; 
                    int startCol = static_cast<int>(floor(cameraX / LOGICAL_TILE_WIDTH));
                    int endCol = startCol + static_cast<int>(ceil(static_cast<float>(SCREEN_WIDTH) / LOGICAL_TILE_WIDTH)) + 1;
                    endCol = std::min(endCol, mapCols); 

                    for (int r = 0; r < mapRows; ++r) {
                        for (int c = startCol; c < endCol; ++c) {
                            if (c < 0 || c >= mapCols) continue; 
                            if (r < 0 || r >= mapRows) continue; 

                            int screenX = static_cast<int>(round(c * LOGICAL_TILE_WIDTH - cameraX));
                            int screenY = static_cast<int>(round(r * LOGICAL_TILE_HEIGHT - cameraY));

                            if (screenX + LOGICAL_TILE_WIDTH < 0 || screenX > SCREEN_WIDTH ||
                                screenY + LOGICAL_TILE_HEIGHT < 0 || screenY > SCREEN_HEIGHT) {
                                continue;
                            }
                            string tileText = std::to_string(mapData[r][c]); 
                            SDL_Surface* surface = TTF_RenderText_Solid(debugFont, tileText.c_str(), textColor);
                            if (surface) {
                                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                                if (texture) {
                                    int textX = screenX + (LOGICAL_TILE_WIDTH - surface->w) / 2;
                                    int textY = screenY + (LOGICAL_TILE_HEIGHT - surface->h) / 2;
                                    SDL_Rect dstRect = {textX, textY, surface->w, surface->h};
                                    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
                                    SDL_DestroyTexture(texture);
                                }
                                SDL_FreeSurface(surface);
                            }
                        }
                    }
                }
                #endif 

                for (Enemy& e : enemies_list) e.render(window, cameraX, cameraY);
                for (Turret& t : turrets_list) t.render(window, cameraX, cameraY);
                for (Bullet& b : playerBulletsList) b.render(window, cameraX, cameraY);
                for (Bullet& eb : enemyBulletsList) eb.render(window, cameraX, cameraY);
                if (player_ptr) player_ptr->render(window, cameraX, cameraY);

                // Draw UI
                if (uiFont && renderer) { 
                    SDL_Color c = {255,255,255,255}; 
                    string sTxt = "SCORE: " + std::to_string(playerScore); 
                    SDL_Surface* sS = TTF_RenderText_Solid(uiFont, sTxt.c_str(), c); 
                    if(sS){
                        SDL_Texture* tS=SDL_CreateTextureFromSurface(renderer,sS); 
                        SDL_Rect dS={10,10,sS->w,sS->h}; 
                        SDL_RenderCopy(renderer,tS,NULL,&dS); 
                        SDL_DestroyTexture(tS); 
                        SDL_FreeSurface(sS);
                    }
                    
                    // --- PHẦN VẼ HUÂN CHƯƠNG ĐÃ ĐƯỢC THÊM VÀO ĐÂY ---
                    if (player_ptr && lifeMedalTexture) {
                        int livesLeft = player_ptr->getLives();
                        if (livesLeft > 0) { // Chỉ vẽ nếu còn mạng
                            int medalSpriteWidth = 0;  
                            int medalSpriteHeight = 0; 
                            SDL_QueryTexture(lifeMedalTexture, NULL, NULL, &medalSpriteWidth, &medalSpriteHeight);

                            // Kích thước bạn muốn render mỗi huân chương
                            const int MEDAL_RENDER_WIDTH = 20; // Ví dụ: Dùng kích thước gốc của texture
                            const int MEDAL_RENDER_HEIGHT = 40;
                            // Hoặc nếu bạn muốn kích thước cố định, ví dụ:
                            // const int MEDAL_RENDER_WIDTH = 24; 
                            // const int MEDAL_RENDER_HEIGHT = 24;

                            int spacingBetweenMedals = 5; 
                            int topMargin = 10;           
                            int rightMargin = 10;         

                            for (int i = 0; i < livesLeft; ++i) {
                                SDL_Rect destRectMedal;
                                destRectMedal.x = SCREEN_WIDTH - rightMargin - (i + 1) * MEDAL_RENDER_WIDTH - i * spacingBetweenMedals;
                                destRectMedal.y = topMargin;
                                destRectMedal.w = MEDAL_RENDER_WIDTH;
                                destRectMedal.h = MEDAL_RENDER_HEIGHT;
                                SDL_RenderCopy(renderer, lifeMedalTexture, NULL, &destRectMedal); 
                            }
                        }
                    }
                    // --- KẾT THÚC PHẦN VẼ HUÂN CHƯƠNG ---
                } 

                if (isPaused && currentGameState == GameState::PLAYING) { SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer,0,0,0,150); SDL_Rect pO={0,0,SCREEN_WIDTH,SCREEN_HEIGHT}; SDL_RenderFillRect(renderer,&pO); SDL_Color pC={255,255,255,255}; string pT="PAUSED"; SDL_Surface* sP=TTF_RenderText_Solid(menuFont,pT.c_str(),pC); if(sP){SDL_Texture* tP=SDL_CreateTextureFromSurface(renderer,sP); SDL_Rect dP={(SCREEN_WIDTH-sP->w)/2,(SCREEN_HEIGHT-sP->h)/2,sP->w,sP->h}; SDL_RenderCopy(renderer,tP,NULL,&dP); SDL_DestroyTexture(tP); SDL_FreeSurface(sP);} }
                else if (currentGameState == GameState::WON) { SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 0, 180, 0, 170); SDL_Rect r={0,0,SCREEN_WIDTH,SCREEN_HEIGHT}; SDL_RenderFillRect(renderer,&r); SDL_Color c={255,255,0,255}; string t1="YOU WIN!"; string tS="FINAL SCORE: "+std::to_string(playerScore); string t2="Press Enter or ESC"; SDL_Surface* s1=TTF_RenderText_Solid(menuFont,t1.c_str(),c); SDL_Surface* sS=TTF_RenderText_Solid(uiFont,tS.c_str(),c); SDL_Surface* s2=TTF_RenderText_Solid(uiFont,t2.c_str(),c); int yP=SCREEN_HEIGHT/2-(s1?s1->h:0)-(sS?sS->h:0)-15; if(s1){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s1); SDL_Rect d={(SCREEN_WIDTH-s1->w)/2, yP, s1->w,s1->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(s1); yP+=d.h+5;} if(sS){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,sS); SDL_Rect d={(SCREEN_WIDTH-sS->w)/2, yP, sS->w,sS->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(sS); yP+=d.h+15;} if(s2){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s2); SDL_Rect d={(SCREEN_WIDTH-s2->w)/2, yP, s2->w,s2->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(s2);} }
                else if (currentGameState == GameState::GAME_OVER) { SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); SDL_SetRenderDrawColor(renderer, 180, 0, 0, 170); SDL_Rect r={0,0,SCREEN_WIDTH,SCREEN_HEIGHT}; SDL_RenderFillRect(renderer,&r); SDL_Color c={255,255,255,255}; string t1="GAME OVER"; string tS="FINAL SCORE: "+std::to_string(playerScore); string t2="Press Enter or ESC"; SDL_Surface* s1=TTF_RenderText_Solid(menuFont,t1.c_str(),c); SDL_Surface* sS=TTF_RenderText_Solid(uiFont,tS.c_str(),c); SDL_Surface* s2=TTF_RenderText_Solid(uiFont,t2.c_str(),c); int yP=SCREEN_HEIGHT/2-(s1?s1->h:0)-(sS?sS->h:0)-15; if(s1){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s1); SDL_Rect d={(SCREEN_WIDTH-s1->w)/2, yP, s1->w,s1->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(s1); yP+=d.h+5;} if(sS){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,sS); SDL_Rect d={(SCREEN_WIDTH-sS->w)/2, yP, sS->w,sS->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(sS); yP+=d.h+15;} if(s2){SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s2); SDL_Rect d={(SCREEN_WIDTH-s2->w)/2, yP, s2->w,s2->h}; SDL_RenderCopy(renderer,tx,NULL,&d); SDL_DestroyTexture(tx); SDL_FreeSurface(s2);} }
            } break; 
        } 
        window.display();

        float frameTicks_render = static_cast<float>(SDL_GetTicks()) - startTicks;
        float desiredFrameTime_ms = 1000.0f / refreshRate;
        if (frameTicks_render < desiredFrameTime_ms) { SDL_Delay(static_cast<Uint32>(desiredFrameTime_ms - frameTicks_render)); }

    } 

    cout << "Cleaning up resources..." << endl;
    delete player_ptr; player_ptr = nullptr;
    enemies_list.clear(); turrets_list.clear();
    playerBulletsList.clear(); enemyBulletsList.clear();

    Mix_FreeMusic(backgroundMusic); Mix_FreeChunk(shootSound); Mix_FreeChunk(gEnemyDeathSound); Mix_FreeChunk(gPlayerDeathSound); Mix_FreeChunk(gTurretExplosionSound); Mix_FreeChunk(gTurretShootSound);
    SDL_DestroyTexture(menuBackgroundTexture); SDL_DestroyTexture(backgroundTexture); SDL_DestroyTexture(playerRunTexture); SDL_DestroyTexture(playerJumpTexture); SDL_DestroyTexture(playerEnterWaterTexture); SDL_DestroyTexture(playerSwimTexture); SDL_DestroyTexture(playerStandAimShootHorizTexture); SDL_DestroyTexture(playerRunAimShootHorizTexture); SDL_DestroyTexture(playerStandAimShootUpTexture); SDL_DestroyTexture(playerStandAimShootDiagUpTexture); SDL_DestroyTexture(playerRunAimShootDiagUpTexture); SDL_DestroyTexture(playerStandAimShootDiagDownTexture); SDL_DestroyTexture(playerRunAimShootDiagDownTexture); SDL_DestroyTexture(playerLyingDownTexture); SDL_DestroyTexture(playerLyingAimShootTexture); SDL_DestroyTexture(playerBulletTexture); SDL_DestroyTexture(turretBulletTexture); SDL_DestroyTexture(enemyTexture); SDL_DestroyTexture(gameTurretTexture); SDL_DestroyTexture(turretExplosionTexture);
    SDL_DestroyTexture(lifeMedalTexture); // ĐÃ THÊM GIẢI PHÓNG

    TTF_CloseFont(uiFont); TTF_CloseFont(menuFont); TTF_CloseFont(debugFont);

    TTF_Quit(); Mix_CloseAudio(); Mix_Quit(); 
    window.cleanUp(); IMG_Quit(); SDL_Quit();
    cout << "Cleanup complete. Exiting." << endl;
    
    return 0; 
}
