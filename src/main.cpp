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
#include <memory>

#include "RenderWindow.hpp"
#include "math.hpp"
#include "utils.hpp"
#include "player.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "Turret.hpp"

using namespace std;

enum class GameState { MAIN_MENU, PLAYING, POST_BOSS_EXPLOSION, WON, GAME_OVER, INSTRUCTIONS};

const int LOGICAL_TILE_WIDTH = 96;
const int LOGICAL_TILE_HEIGHT = 96;

vector<vector<int>> mapData = {
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,4,1,1,0,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,4,0,1,1,0,0,4,0,0,1,1,0,0,1,1,4,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,5},
{0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,0,4,0,1,1,1,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,4,4,4,1,1,0,1,1,1,1,1,1,1,0,0,0,0,4,0,0,0,0,1,0,1,1,1,0,0,1,1,0,0,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,0,0,5},
{0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,1,0,5},
{3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,3,3,3,3,3,3,3,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,5}
};

Mix_Chunk* gEnemyDeathSound = nullptr;
Mix_Chunk* gPlayerDeathSound = nullptr;
Mix_Chunk* gTurretExplosionSound = nullptr;
Mix_Chunk* gTurretShootSound = nullptr;
Mix_Chunk* gBossMainExplosionSound = nullptr;
Mix_Chunk* gVictoryMusicChunk = nullptr;
Mix_Chunk* gScoreTickSound = nullptr;

std::vector<std::unique_ptr<Turret>> bossPartsList;
const int BOSS_TOTAL_HP = 50;
int bossCurrentHP;
bool bossFightEngaged = false;
bool bossIsActuallyDefeated = false;
float bossArenaTriggerX = 0.0f;
bool bossMainExplosionSoundPlayed = false;
int victoryMusicChannel = -1;

const float BOSS_EXPLOSION_SEQUENCE_DURATION = 7.0f;
float bossExplosionSequenceTimer = 0.0f;

const int MAX_PLAY_TIME_SECONDS = 180;
float currentGameTimeRemaining = MAX_PLAY_TIME_SECONDS;

const int INITIAL_PLAYER_LIVES = 4;

enum class WinAnimationPhase { SCORE, TIME_BONUS, LIVES_BONUS, TOTAL, DONE };
WinAnimationPhase currentWinAnimPhase = WinAnimationPhase::SCORE;
float winScreenAnimAccumulator = 0.0f;
const float WIN_ANIM_TICK_INTERVAL = 0.02f;

const float WIN_SCORE_POINTS_PER_TICK = 50.0f;
const float WIN_BONUS_POINTS_PER_TICK = 20.0f;
const int WIN_TIME_SECONDS_PER_TICK = 1;

int displayedScore = 0;
int displayedTimeBonus = 0;
int displayedLivesBonus = 0;
int displayedTotalScore = 0;

int actualTimeLeftForBonus_capture = 0;
int timeBonusValueToAnimate_display = 0;
int actualLivesForBonus_capture = 0;
bool isPerfectWin = false;

int finalScore_val = 0;
int finalTimeBonus_val = 0;
int finalLivesBonus_val = 0;

const int POINTS_PER_SECOND_REMAINING = 50;
const int POINTS_PER_LIFE = 100;
const int PERFECT_BONUS = 1000;

enum class MainMenuOption { START, INSTRUCTIONS, QUIT_OPTION };
MainMenuOption currentMenuOption = MainMenuOption::START;
TTF_Font* menuOptionFont = nullptr;

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) > 0)
    {
        cerr << "SDL_Init failed: " << SDL_GetError() << endl;
        return 1;
    }
    if (!IMG_Init(IMG_INIT_PNG))
    {
        cerr << "IMG_Init failed: " << IMG_GetError() << endl;
        SDL_Quit();
        return 1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    if (TTF_Init() == -1)
    {
        cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << endl;
        Mix_CloseAudio();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    cout << "SDL, IMG, Mixer, TTF initialized." << endl;

    const int SCREEN_WIDTH = 1024;
    const int SCREEN_HEIGHT = 672;
    RenderWindow window("Contra Clone Reloaded - Boss Fight", SCREEN_WIDTH, SCREEN_HEIGHT);
    int refreshRate = window.getRefreshRate();
    if (refreshRate <= 0)
    {
        refreshRate = 60;
    }
    SDL_Renderer* renderer = window.getRenderer();

    const int UI_FONT_SIZE_NEW = 14;
    const int MENU_OPTION_FONT_SIZE_NEW = 20;

    TTF_Font* uiFont = TTF_OpenFont("res/font/kongtext.ttf", UI_FONT_SIZE_NEW);
    TTF_Font* menuFont = TTF_OpenFont("res/font/kongtext.ttf", 28);
    menuOptionFont = TTF_OpenFont("res/font/kongtext.ttf", MENU_OPTION_FONT_SIZE_NEW);
    TTF_Font* debugFont = TTF_OpenFont("res/font/kongtext.ttf", 16);
    if (!uiFont || !menuFont || !debugFont || !menuOptionFont)
    {
    cerr << "Font load error: " << TTF_GetError() << endl;
    Mix_CloseAudio();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
    return 1;
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
    gBossMainExplosionSound = Mix_LoadWAV("res/snd/turret_explosion_sound.wav");
    gVictoryMusicChunk = Mix_LoadWAV("res/snd/victory_music.wav");
    gScoreTickSound = Mix_LoadWAV("res/snd/score_tick.wav");

    bool loadError = false;
    if (!menuBackgroundTexture || !backgroundTexture || !playerRunTexture || !playerJumpTexture ||
        !playerEnterWaterTexture || !playerSwimTexture || !playerStandAimShootHorizTexture ||
        !playerRunAimShootHorizTexture || !playerStandAimShootUpTexture || !playerStandAimShootDiagUpTexture ||
        !playerRunAimShootDiagUpTexture || !playerStandAimShootDiagDownTexture || !playerRunAimShootDiagDownTexture ||
        !playerLyingDownTexture || !playerLyingAimShootTexture || !playerBulletTexture || !turretBulletTexture ||
        !enemyTexture || !gameTurretTexture || !turretExplosionTexture || !lifeMedalTexture ||
        !backgroundMusic || !shootSound || !gEnemyDeathSound || !gPlayerDeathSound ||
        !gTurretExplosionSound || !gTurretShootSound || !gBossMainExplosionSound )
    {
        loadError = true;
    cerr << "Error loading one or more essential resources!" << endl;
    }
    if (!gVictoryMusicChunk) { cerr << "Warning: Failed to load 'res/snd/victory_music.wav'. Victory sound will be absent. Mix_Error: " << Mix_GetError() << endl; }
    if (!gScoreTickSound) { cerr << "Warning: Failed to load 'res/snd/score_tick.wav'. Score tick sound will be absent. Mix_Error: " << Mix_GetError() << endl; }

    if (loadError)
    {
        TTF_CloseFont(uiFont);
        TTF_CloseFont(menuFont);
        TTF_CloseFont(debugFont);
        if(menuOptionFont)
        {
            TTF_CloseFont(menuOptionFont);
        }
        Mix_FreeMusic(backgroundMusic);
        Mix_FreeChunk(shootSound);
        Mix_FreeChunk(gEnemyDeathSound);
        Mix_FreeChunk(gPlayerDeathSound);
        Mix_FreeChunk(gTurretExplosionSound);
        Mix_FreeChunk(gTurretShootSound);
        Mix_FreeChunk(gBossMainExplosionSound);
        Mix_FreeChunk(gVictoryMusicChunk);
        Mix_FreeChunk(gScoreTickSound);

        SDL_DestroyTexture(menuBackgroundTexture);
        SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyTexture(playerRunTexture);
        SDL_DestroyTexture(playerJumpTexture);
        SDL_DestroyTexture(playerEnterWaterTexture);
        SDL_DestroyTexture(playerSwimTexture);
        SDL_DestroyTexture(playerStandAimShootHorizTexture);
        SDL_DestroyTexture(playerRunAimShootHorizTexture);
        SDL_DestroyTexture(playerStandAimShootUpTexture);
        SDL_DestroyTexture(playerStandAimShootDiagUpTexture);
        SDL_DestroyTexture(playerRunAimShootDiagUpTexture);
        SDL_DestroyTexture(playerStandAimShootDiagDownTexture);
        SDL_DestroyTexture(playerRunAimShootDiagDownTexture);
        SDL_DestroyTexture(playerLyingDownTexture);
        SDL_DestroyTexture(playerLyingAimShootTexture);
        SDL_DestroyTexture(playerBulletTexture);
        SDL_DestroyTexture(turretBulletTexture);
        SDL_DestroyTexture(enemyTexture);
        SDL_DestroyTexture(gameTurretTexture);
        SDL_DestroyTexture(turretExplosionTexture);
        SDL_DestroyTexture(lifeMedalTexture);

        Mix_CloseAudio();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    cout << "Resources loaded." << endl;

    int BG_TEXTURE_WIDTH = 0;
    int BG_TEXTURE_HEIGHT = 0;
    if (backgroundTexture)
    {
        SDL_QueryTexture(backgroundTexture, NULL, NULL, &BG_TEXTURE_WIDTH, &BG_TEXTURE_HEIGHT);
    }

    const int PLAYER_STANDARD_FRAME_W = 40;
    const int PLAYER_STANDARD_FRAME_H = 78;
    const int PLAYER_LYING_FRAME_W = 78;
    const int PLAYER_LYING_FRAME_H = 40;
    const int PLAYER_RUN_SHEET_COLS = 6;
    const int PLAYER_JUMP_SHEET_COLS = 4;
    const int PLAYER_ENTER_WATER_SHEET_COLS = 1;
    const int PLAYER_SWIM_SHEET_COLS = 5;
    const int PLAYER_STAND_AIM_SHOOT_HORIZ_SHEET_COLS = 1;
    const int PLAYER_RUN_AIM_SHOOT_HORIZ_SHEET_COLS = 3;
    const int PLAYER_STAND_AIM_SHOOT_UP_SHEET_COLS = 2;
    const int PLAYER_STAND_AIM_SHOOT_DIAG_UP_SHEET_COLS = 1;
    const int PLAYER_RUN_AIM_SHOOT_DIAG_UP_SHEET_COLS = 3;
    const int PLAYER_STAND_AIM_SHOOT_DIAG_DOWN_SHEET_COLS = 1;
    const int PLAYER_RUN_AIM_SHOOT_DIAG_DOWN_SHEET_COLS = 3;
    const int PLAYER_LYING_DOWN_SHEET_COLS = 1;
    const int PLAYER_LYING_AIM_SHOOT_SHEET_COLS = 1;
    const int PLAYER_BULLET_RENDER_WIDTH = 12;
    const int PLAYER_BULLET_RENDER_HEIGHT = 6;


    const float INITIAL_CAMERA_X = 0.0f;
    const float INITIAL_CAMERA_Y = 0.0f;
    const float PLAYER_START_X = 100.0f;
    const float PLAYER_START_Y = 300.0f;
    const float PLAYER_RESPAWN_OFFSET_X = 150.0f;

    float cameraX = INITIAL_CAMERA_X;
    float cameraY = INITIAL_CAMERA_Y;
    GameState currentGameState = GameState::MAIN_MENU;
    Player* player_ptr = nullptr;
    list<Bullet> playerBulletsList;
    list<Bullet> enemyBulletsList;
    list<Enemy> enemies_list;
    list<Turret> regularTurrets_list;
    int playerScore = 0;
    bool gameRunning = true;
    bool isPaused = false;
    bool isMusicEnabled = true;
    const float timeStep = 0.01f;
    float accumulator = 0.0f;
    float currentTime_game = utils::hireTimeInSeconds();
    SDL_Event event;

    int mapRows = mapData.size();
    int mapCols = (mapRows > 0) ? mapData[0].size() : 0;
    if (mapCols == 0)
    {
        cerr << "Error: mapData is empty!" << endl;
        return 1;
    }
    cout << "Map: " << mapRows << "x" << mapCols << endl;

    int lastMapCol = mapCols > 0 ? mapCols - 1 : -1;
    bossArenaTriggerX = static_cast<float>((mapCols > 6 ? mapCols - 6 : 0) * LOGICAL_TILE_WIDTH);
    cout << "Boss Arena Trigger X: " << bossArenaTriggerX << endl;

    auto initializeGame = [&]() {
        cout << "Initializing Game State..." << endl;
        playerBulletsList.clear();
        enemyBulletsList.clear();
        enemies_list.clear();
        regularTurrets_list.clear();
        bossPartsList.clear();
        playerScore = 0;
        vector2d initialPos = {PLAYER_START_X, PLAYER_START_Y};
        if (player_ptr)
        {
            player_ptr->resetPlayerStateForNewGame();
            player_ptr->setPos(initialPos);
            player_ptr->setInvulnerable(false);
        }
        else
        {
            player_ptr = new Player(initialPos,
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
            PLAYER_STANDARD_FRAME_W, PLAYER_STANDARD_FRAME_H, PLAYER_LYING_FRAME_W, PLAYER_LYING_FRAME_H
            );
            player_ptr->setInvulnerable(false);
        }

        cameraX = INITIAL_CAMERA_X;
        cameraY = INITIAL_CAMERA_Y;
        isPaused = false;
        currentGameTimeRemaining = MAX_PLAY_TIME_SECONDS;
        currentWinAnimPhase = WinAnimationPhase::SCORE;
        winScreenAnimAccumulator = 0.0f;
        displayedScore = 0;
        displayedTimeBonus = 0;
        displayedLivesBonus = 0;
        displayedTotalScore = 0;
        actualTimeLeftForBonus_capture = 0;
        timeBonusValueToAnimate_display = 0;
        actualLivesForBonus_capture = 0;
        isPerfectWin = false;
        finalScore_val = 0;
        finalTimeBonus_val = 0;
        finalLivesBonus_val = 0;

        bossCurrentHP = BOSS_TOTAL_HP;
        bossFightEngaged = false;
        bossIsActuallyDefeated = false;
        bossMainExplosionSoundPlayed = false;
        bossExplosionSequenceTimer = 0.0f;
        victoryMusicChannel = -1;

        auto spawnEnemyFunc = [&](float wx, int gr)
        {
            float eh=72.f;
            float gy=static_cast<float>(gr*LOGICAL_TILE_HEIGHT);
            float sy=gy-eh;
            enemies_list.emplace_back(vector2d{wx, sy}, enemyTexture);
        };
        spawnEnemyFunc(8.0f*LOGICAL_TILE_WIDTH, 3);
        spawnEnemyFunc(15.0f*LOGICAL_TILE_WIDTH, 3);
        spawnEnemyFunc(30.0f*LOGICAL_TILE_WIDTH, 2);
        spawnEnemyFunc(40.0f*LOGICAL_TILE_WIDTH, 4);
        spawnEnemyFunc(45.0f*LOGICAL_TILE_WIDTH, 3);
        spawnEnemyFunc(55.0f*LOGICAL_TILE_WIDTH, 4);
        spawnEnemyFunc(67.0f*LOGICAL_TILE_WIDTH, 2);
        spawnEnemyFunc(73.0f*LOGICAL_TILE_WIDTH, 4);
        spawnEnemyFunc(77.0f*LOGICAL_TILE_WIDTH, 3);
        spawnEnemyFunc(89.0f*LOGICAL_TILE_WIDTH, 3);
        spawnEnemyFunc(92.0f*LOGICAL_TILE_WIDTH, 5);
        spawnEnemyFunc(95.0f*LOGICAL_TILE_WIDTH, 3);
        for (int r = 0; r < mapRows; ++r) {
            for (int c = 0; c < mapCols; ++c) {
                if (mapData[r][c] == 5 && c == lastMapCol) {
                    float partX = static_cast<float>(c * LOGICAL_TILE_WIDTH);
                    float partY = static_cast<float>(r * LOGICAL_TILE_HEIGHT);
                    bossPartsList.emplace_back(std::make_unique<Turret>(
                        vector2d{partX, partY},
                        nullptr,
                        turretExplosionTexture,
                        gTurretExplosionSound,
                        gTurretShootSound,
                        turretBulletTexture,
                        LOGICAL_TILE_WIDTH,
                        LOGICAL_TILE_HEIGHT
                    ));
                    if (!bossPartsList.empty()) {
                        bossPartsList.back()->setHp(BOSS_TOTAL_HP + 999);
                    }
                }
                else if (mapData[r][c] == 4) {
                    float tx = static_cast<float>(c * LOGICAL_TILE_WIDTH);
                    float ty = static_cast<float>(r * LOGICAL_TILE_HEIGHT);
                    regularTurrets_list.emplace_back(
                        vector2d{tx, ty},
                        gameTurretTexture,
                        turretExplosionTexture,
                        gTurretExplosionSound,
                        gTurretShootSound,
                        turretBulletTexture,
                        LOGICAL_TILE_WIDTH,
                        LOGICAL_TILE_HEIGHT
                    );
                }
            }
        }
        cout << "Game Initialized. Spawned " << enemies_list.size() << " troops and " << regularTurrets_list.size() << " regular turrets." << endl;
        cout << "Boss has " << bossPartsList.size() << " vulnerable points." << endl;

        if (isMusicEnabled)
        {
            if (Mix_PlayingMusic() == 0)
            {
                if (Mix_PlayMusic(backgroundMusic, -1) == -1)
                {
                    cerr << "Mix_PlayMusic Error: " << Mix_GetError() << endl;
                }
            }
            else if (Mix_PausedMusic() == 1)
            {
                Mix_ResumeMusic();
            }
        }
        else
        {
            Mix_HaltMusic();
        }
    };

    while(gameRunning)
    {
        int startTicks = SDL_GetTicks();
        float newTime = static_cast<float>(utils::hireTimeInSeconds());
        float frameTime = newTime - currentTime_game;
        if(frameTime > 0.25f)
        {
            frameTime = 0.25f;
        }
        currentTime_game = newTime;

        while(SDL_PollEvent(&event))
        {
             if(event.type == SDL_QUIT)
             {
                 gameRunning = false;
             }
             switch (currentGameState)
             {
                case GameState::MAIN_MENU:
                    if (event.type == SDL_KEYDOWN) {
                        SDL_Keycode keyPressed = event.key.keysym.sym;
                        if (keyPressed == SDLK_UP) {
                            if (currentMenuOption == MainMenuOption::INSTRUCTIONS) currentMenuOption = MainMenuOption::START;
                            else if (currentMenuOption == MainMenuOption::QUIT_OPTION) currentMenuOption = MainMenuOption::INSTRUCTIONS;
                        } else if (keyPressed == SDLK_DOWN) {
                            if (currentMenuOption == MainMenuOption::START) currentMenuOption = MainMenuOption::INSTRUCTIONS;
                            else if (currentMenuOption == MainMenuOption::INSTRUCTIONS) currentMenuOption = MainMenuOption::QUIT_OPTION;
                        } else if (keyPressed == SDLK_RETURN || keyPressed == SDLK_KP_ENTER) {
                            if (currentMenuOption == MainMenuOption::START) {
                                currentGameState = GameState::PLAYING;
                                initializeGame();
                            } else if (currentMenuOption == MainMenuOption::INSTRUCTIONS) {
                                currentGameState = GameState::INSTRUCTIONS;
                            } else if (currentMenuOption == MainMenuOption::QUIT_OPTION) {
                                gameRunning = false;
                            }
                        } else if (keyPressed == SDLK_ESCAPE) {
                            gameRunning = false;
                        }
                    }
                    break;
                case GameState::PLAYING:
                case GameState::POST_BOSS_EXPLOSION:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_p && !event.key.repeat && currentGameState == GameState::PLAYING && !bossIsActuallyDefeated ) {
                            isPaused = !isPaused;
                            if (isPaused)
                            {
                                if (isMusicEnabled && Mix_PlayingMusic()) Mix_PauseMusic();
                            }
                            else
                            {
                                if (isMusicEnabled && Mix_PausedMusic()) Mix_ResumeMusic();
                            }
                            cout << (isPaused ? "PAUSED" : "RESUMED") << endl;
                        }
                        else if (event.key.keysym.sym == SDLK_m && !event.key.repeat) {
                            isMusicEnabled = !isMusicEnabled;
                            if (isMusicEnabled) {
                                if (currentGameState == GameState::PLAYING && !bossIsActuallyDefeated)
                                {
                                   if (!Mix_PlayingMusic() && backgroundMusic) Mix_PlayMusic(backgroundMusic, -1);
                                   else if (Mix_PausedMusic()) Mix_ResumeMusic();
                                   cout << "Music ON (Background)" << endl;
                                }
                                else if (currentGameState == GameState::POST_BOSS_EXPLOSION && gVictoryMusicChunk)
                                {
                                     if (victoryMusicChannel != -1 && Mix_Playing(victoryMusicChannel))
                                     {
                                         cout << "Music ON (Victory Sequence - already playing)" << endl;
                                     }
                                     else if (gVictoryMusicChunk)
                                     {
                                         victoryMusicChannel = Mix_PlayChannel(-1, gVictoryMusicChunk, 0);
                                         if(victoryMusicChannel == -1)
                                         {
                                             cerr << "Failed to play victory music on M toggle: " << Mix_GetError() << endl;
                                         }
                                         cout << "Music ON (Victory Sequence - started)" << endl;
                                     }
                                }
                            }
                            else
                            {
                                if (Mix_PlayingMusic()) Mix_PauseMusic();
                                if (victoryMusicChannel != -1 && Mix_Playing(victoryMusicChannel))
                                {
                                    Mix_Pause(victoryMusicChannel);
                                }
                                cout << "Music OFF (Paused)" << endl;
                            }
                        }
                        else if (!isPaused && player_ptr && currentGameState == GameState::PLAYING && !bossIsActuallyDefeated) {
                            player_ptr->handleKeyDown(event.key.keysym.sym);
                        }
                        if (event.key.keysym.sym == SDLK_ESCAPE)
                        {
                             gameRunning = false;
                        }
                    }
                    break;
                case GameState::WON:
                case GameState::GAME_OVER:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_ESCAPE)
                        {
                             gameRunning = false;
                        }
                        else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
                        {
                             if (currentGameState == GameState::WON && currentWinAnimPhase != WinAnimationPhase::DONE)
                             {
                                displayedScore = finalScore_val;
                                timeBonusValueToAnimate_display = 0;
                                displayedTimeBonus = finalTimeBonus_val;
                                displayedLivesBonus = finalLivesBonus_val;
                                displayedTotalScore = finalScore_val + finalTimeBonus_val + finalLivesBonus_val;
                                currentWinAnimPhase = WinAnimationPhase::DONE;
                             }
                             else
                             {
                                currentGameState = GameState::MAIN_MENU;
                                if (isMusicEnabled && victoryMusicChannel != -1) Mix_HaltChannel(victoryMusicChannel);
                                Mix_HaltMusic();
                                victoryMusicChannel = -1;
                             }
                        }
                    }
                    break;
                case GameState::INSTRUCTIONS:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_ESCAPE) {
                            currentGameState = GameState::MAIN_MENU;
                            currentMenuOption = MainMenuOption::INSTRUCTIONS;
                        }
                    }
                    break;
             }
        }

        if ((currentGameState == GameState::PLAYING || currentGameState == GameState::POST_BOSS_EXPLOSION) && !isPaused) {
            accumulator += frameTime;
            const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
            if(player_ptr && currentGameState == GameState::PLAYING && !bossIsActuallyDefeated)
            {
                player_ptr->handleInput(currentKeyStates);
            }

            while(accumulator >= timeStep) {
                if (currentGameState == GameState::POST_BOSS_EXPLOSION) {
                    bossExplosionSequenceTimer -= timeStep;

                    for (auto& part : bossPartsList) {
                        part->update(timeStep, nullptr, enemyBulletsList);
                    }
                    if (player_ptr) {
                        player_ptr->update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                    }

                    bool canTransitionToWon = false;
                    if (gVictoryMusicChunk && victoryMusicChannel != -1) {
                        if (Mix_Playing(victoryMusicChannel) == 0) {
                            canTransitionToWon = true;
                        }
                    } else {
                        if (bossExplosionSequenceTimer <= 0.0f) {
                            canTransitionToWon = true;
                        }
                    }

                    if (canTransitionToWon) {
                        cout << "Post-defeat explosion sequence finished. Transitioning to VICTORY screen." << endl;
                        currentGameState = GameState::WON;
                        if (victoryMusicChannel != -1)
                        {
                            Mix_HaltChannel(victoryMusicChannel);
                        }
                        victoryMusicChannel = -1;

                        finalScore_val = playerScore;
                        actualTimeLeftForBonus_capture = static_cast<int>(ceil(std::max(0.0f, currentGameTimeRemaining)));
                        timeBonusValueToAnimate_display = actualTimeLeftForBonus_capture;
                        finalTimeBonus_val = actualTimeLeftForBonus_capture * POINTS_PER_SECOND_REMAINING;
                        actualLivesForBonus_capture = player_ptr ? player_ptr->getLives() : 0;
                        isPerfectWin = (player_ptr && player_ptr->getLives() == INITIAL_PLAYER_LIVES);
                        finalLivesBonus_val = isPerfectWin ? PERFECT_BONUS : actualLivesForBonus_capture * POINTS_PER_LIFE;
                        currentWinAnimPhase = WinAnimationPhase::SCORE;
                        winScreenAnimAccumulator = 0.0f;
                        displayedScore = 0;
                        displayedTimeBonus = 0;
                        displayedLivesBonus = 0;

                        accumulator = 0;
                        break;
                    }
                }
                else if (currentGameState == GameState::PLAYING) {
                    if(player_ptr)
                    {
                        player_ptr->update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                        player_ptr->getPos().x = std::max(cameraX, player_ptr->getPos().x);
                    }
                    for (Enemy& e : enemies_list)
                    {
                        e.update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                    }
                    for (Turret& t : regularTurrets_list)
                    {
                        t.update(timeStep, player_ptr, enemyBulletsList);
                    }

                    if (player_ptr && !player_ptr->getIsDead()) {
                        currentGameTimeRemaining -= timeStep;
                        if (currentGameTimeRemaining <= 0.0f)
                        {
                            currentGameTimeRemaining = 0.0f;
                            currentGameState = GameState::GAME_OVER;
                            if(isMusicEnabled && Mix_PlayingMusic()) Mix_HaltMusic();
                            cout << "--- TIME'S UP! GAME OVER --- Final Score: " << playerScore << endl;
                            accumulator = 0;
                            break;
                        }
                    }

                    if (bossFightEngaged && !bossIsActuallyDefeated) {
                        for (auto& part : bossPartsList)
                        {
                            part->update(timeStep, player_ptr, enemyBulletsList);
                        }
                    }

                    for (auto it_b = playerBulletsList.begin(); it_b != playerBulletsList.end(); ) {
                        it_b->update(timeStep);
                        if (!it_b->isActive())
                        {
                            it_b = playerBulletsList.erase(it_b);
                            continue;
                        }
                        SDL_Rect bHB = it_b->getWorldHitbox();
                        bool hit = false;
                        for (auto it_e = enemies_list.begin(); it_e != enemies_list.end(); ++it_e)
                        {
                            if (it_e->isAlive())
                            {
                                SDL_Rect eHB = it_e->getWorldHitbox();
                                if (SDL_HasIntersection(&bHB, &eHB))
                                {
                                    bool wasA = it_e->isAlive();
                                    it_e->takeHit();
                                    if(wasA && !it_e->isAlive())
                                    {
                                        playerScore+=200;
                                    }
                                    it_b->setActive(false);
                                    hit = true;
                                    break;
                                }
                            }
                        }
                        if (hit)
                        {
                            it_b = playerBulletsList.erase(it_b);
                            continue;
                        }
                        for (auto it_t = regularTurrets_list.begin(); it_t != regularTurrets_list.end(); ++it_t)
                        {
                            if (it_t->getHp() > 0)
                            {
                                SDL_Rect tHB = it_t->getWorldHitbox();
                                if (SDL_HasIntersection(&bHB, &tHB))
                                {
                                    int hpB=it_t->getHp();
                                    it_t->takeDamage();
                                    if(hpB>0 && it_t->getHp()<=0)
                                    {
                                        playerScore+=500;
                                    }
                                    it_b->setActive(false);
                                    hit = true;
                                    break;
                                }
                            }
                        }
                        if (hit)
                        {
                            it_b = playerBulletsList.erase(it_b);
                            continue;
                        }

                        if (bossFightEngaged && !bossIsActuallyDefeated) {
                            for (auto& part_turret : bossPartsList)
                            {
                                SDL_Rect partHB = part_turret->getWorldHitbox();
                                if (SDL_HasIntersection(&bHB, &partHB))
                                {
                                    bossCurrentHP--;
                                    playerScore += 50;
                                    it_b->setActive(false);
                                    hit = true;
                                    if (bossCurrentHP <= 0) {
                                        bossCurrentHP = 0;
                                        if (!bossIsActuallyDefeated) {
                                            bossIsActuallyDefeated = true;
                                            playerScore += 1500;
                                            cout << "BOSS HP DEPLETED! Starting post-defeat explosion sequence." << endl;
                                            if (gBossMainExplosionSound && !bossMainExplosionSoundPlayed)
                                            {
                                                Mix_PlayChannel(-1, gBossMainExplosionSound, 0);
                                                bossMainExplosionSoundPlayed = true;
                                            }
                                            for (auto& exploding_part : bossPartsList) {
                                                exploding_part->forceExplode();
                                            }
                                            bossExplosionSequenceTimer = BOSS_EXPLOSION_SEQUENCE_DURATION;
                                            currentGameState = GameState::POST_BOSS_EXPLOSION;
                                            if (isMusicEnabled) {
                                                Mix_HaltMusic();
                                                if (gVictoryMusicChunk) {
                                                    victoryMusicChannel = Mix_PlayChannel(-1, gVictoryMusicChunk, 0);
                                                    if (victoryMusicChannel == -1) {
                                                        cerr << "Failed to play victory music on boss defeat: " << Mix_GetError() << endl;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        if (hit)
                        {
                            it_b = playerBulletsList.erase(it_b);
                            continue;
                        }
                        if (currentGameState != GameState::PLAYING)
                        {
                            accumulator = 0;
                            break;
                        }
                        ++it_b;
                    }
                    if (currentGameState != GameState::PLAYING)
                    {
                        accumulator = 0;
                        break;
                    }

                    for (auto it_eb = enemyBulletsList.begin(); it_eb != enemyBulletsList.end(); ) {
                        it_eb->update(timeStep);
                        if (!it_eb->isActive())
                        {
                            it_eb = enemyBulletsList.erase(it_eb);
                            continue;
                        }
                        if (player_ptr && !player_ptr->getIsDead() && !player_ptr->isInvulnerable() && currentGameState == GameState::PLAYING && !bossIsActuallyDefeated)
                        {
                            SDL_Rect ebHB = it_eb->getWorldHitbox();
                            SDL_Rect pHB = player_ptr->getWorldHitbox();
                            if (SDL_HasIntersection(&ebHB, &pHB))
                            {
                                bool wasA=!player_ptr->getIsDead();
                                player_ptr->takeHit(false);
                                if(gPlayerDeathSound && wasA && player_ptr->getIsDead())
                                {
                                    Mix_PlayChannel(-1, gPlayerDeathSound, 0);
                                }
                                it_eb->setActive(false);
                            }
                        }
                        if (!it_eb->isActive())
                        {
                            it_eb = enemyBulletsList.erase(it_eb);
                        }
                        else
                        {
                            ++it_eb;
                        }
                    }
                }
                accumulator -= timeStep;
            }

            if (currentGameState == GameState::PLAYING)
            {
                enemies_list.remove_if([](const Enemy& e){ return e.isDead(); });
                regularTurrets_list.remove_if([](const Turret& t){ return t.isFullyDestroyed(); });

                if (player_ptr && player_ptr->getCurrentState() == PlayerState::DEAD)
                {
                    if (player_ptr->getLives() > 0)
                    {
                        player_ptr->respawn(cameraX, PLAYER_START_Y, PLAYER_RESPAWN_OFFSET_X);
                    }
                    else
                    {
                        currentGameState = GameState::GAME_OVER;
                        if(isMusicEnabled && Mix_PlayingMusic()) Mix_HaltMusic();
                        cout << "--- GAME OVER --- Final Score: " << playerScore << endl;
                    }
                }
                if (player_ptr && !player_ptr->getIsDead() && !bossFightEngaged && player_ptr->getPos().x + PLAYER_STANDARD_FRAME_W / 2.0f >= bossArenaTriggerX)
                {
                    bossFightEngaged = true;
                    cout << "BOSS FIGHT ENGAGED!" << endl;
                }
                if (player_ptr && !player_ptr->getIsDead() && currentGameState == GameState::PLAYING && !bossIsActuallyDefeated)
                {
                    vector2d bs, bv;
                    if (player_ptr->wantsToShoot(bs, bv))
                    {
                        playerBulletsList.emplace_back(bs, bv, playerBulletTexture, PLAYER_BULLET_RENDER_WIDTH, PLAYER_BULLET_RENDER_HEIGHT);
                        if(shootSound)
                        {
                             Mix_PlayChannel(-1, shootSound, 0);
                        }
                    }
                }
            }
        }

        if(player_ptr && !player_ptr->getIsDead() && currentGameState != GameState::WON )
        {
             float pCX = static_cast<float>(player_ptr->getWorldHitbox().x + player_ptr->getWorldHitbox().w / 2.0f);
             float targetCamX = pCX - static_cast<float>(SCREEN_WIDTH) / 2.5f;
             if (targetCamX > cameraX)
             {
                 cameraX = targetCamX;
             }
        }


        if (currentGameState == GameState::WON && currentWinAnimPhase != WinAnimationPhase::DONE) {
            winScreenAnimAccumulator += frameTime;
            while (winScreenAnimAccumulator >= WIN_ANIM_TICK_INTERVAL) {
                winScreenAnimAccumulator -= WIN_ANIM_TICK_INTERVAL;
                bool changedScore = false;
                switch (currentWinAnimPhase) {
                    case WinAnimationPhase::SCORE:
                        if (displayedScore < finalScore_val)
                        {
                            int o=displayedScore;
                            displayedScore+=static_cast<int>(WIN_SCORE_POINTS_PER_TICK);
                            if(displayedScore>=finalScore_val)
                            {
                                displayedScore=finalScore_val;
                                currentWinAnimPhase=WinAnimationPhase::TIME_BONUS;
                            }
                            if(o!=displayedScore)
                            {
                                changedScore=true;
                            }
                        }
                        else
                        {
                            displayedScore=finalScore_val;
                            currentWinAnimPhase=WinAnimationPhase::TIME_BONUS;
                        }
                        break;
                    case WinAnimationPhase::TIME_BONUS:
                        if (timeBonusValueToAnimate_display > 0)
                        {
                            int o=displayedTimeBonus;
                            int d=std::min(timeBonusValueToAnimate_display,WIN_TIME_SECONDS_PER_TICK);
                            timeBonusValueToAnimate_display-=d;
                            displayedTimeBonus+=d*POINTS_PER_SECOND_REMAINING;
                            if(timeBonusValueToAnimate_display<=0)
                            {
                                displayedTimeBonus=finalTimeBonus_val;
                                currentWinAnimPhase=WinAnimationPhase::LIVES_BONUS;
                            }
                            if(o!=displayedTimeBonus)
                            {
                                changedScore=true;
                            }
                        }
                        else
                        {
                            displayedTimeBonus=finalTimeBonus_val;
                            currentWinAnimPhase=WinAnimationPhase::LIVES_BONUS;
                        }
                        break;
                    case WinAnimationPhase::LIVES_BONUS:
                        if (displayedLivesBonus < finalLivesBonus_val)
                        {
                            int o=displayedLivesBonus;
                            displayedLivesBonus+=static_cast<int>(WIN_BONUS_POINTS_PER_TICK);
                            if(displayedLivesBonus>=finalLivesBonus_val)
                            {
                                displayedLivesBonus=finalLivesBonus_val;
                                currentWinAnimPhase=WinAnimationPhase::TOTAL;
                            }
                            if(o!=displayedLivesBonus)
                            {
                                changedScore=true;
                            }
                        }
                        else
                        {
                            displayedLivesBonus=finalLivesBonus_val;
                            currentWinAnimPhase=WinAnimationPhase::TOTAL;
                        }
                        break;
                    case WinAnimationPhase::TOTAL: currentWinAnimPhase = WinAnimationPhase::DONE; break;
                    case WinAnimationPhase::DONE: break;
                }
                if (changedScore && gScoreTickSound)
                {
                     Mix_PlayChannel(-1, gScoreTickSound, 0);
                }
                if (currentWinAnimPhase == WinAnimationPhase::DONE)
                {
                     break;
                }
            }
            displayedTotalScore = displayedScore + displayedTimeBonus + displayedLivesBonus;
        }

        if(currentGameState != GameState::MAIN_MENU)
        {
            cameraX = std::max(0.0f, cameraX);
            if (BG_TEXTURE_WIDTH > SCREEN_WIDTH)
            {
                cameraX = std::min(cameraX, static_cast<float>(BG_TEXTURE_WIDTH - SCREEN_WIDTH));
            }
            else
            {
                cameraX = 0.0f;
            }
        }

        window.clear();
        switch (currentGameState) {
            case GameState::MAIN_MENU: {
                SDL_RenderCopy(renderer, menuBackgroundTexture, NULL, NULL);
                SDL_Color optionColor = {255, 255, 255, 255};
                SDL_Color selectedColor = {0, 255, 0, 255};

                int menuStartY = SCREEN_HEIGHT / 3+100;
                int optionSpacing = 0;
                if (menuOptionFont) {
                    optionSpacing = TTF_FontHeight(menuOptionFont) + 15;
                } else {
                    optionSpacing = 35;
                }

                SDL_Surface* startSurface = TTF_RenderText_Solid(menuOptionFont, "START GAME", (currentMenuOption == MainMenuOption::START ? selectedColor : optionColor));
                if (startSurface) {
                    SDL_Texture* startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
                    SDL_Rect startRect = {(SCREEN_WIDTH - startSurface->w) / 2, menuStartY, startSurface->w, startSurface->h};
                    SDL_RenderCopy(renderer, startTexture, NULL, &startRect);
                    SDL_DestroyTexture(startTexture);
                    SDL_FreeSurface(startSurface);
                }

                SDL_Surface* instrSurface = TTF_RenderText_Solid(menuOptionFont, "INSTRUCTIONS", (currentMenuOption == MainMenuOption::INSTRUCTIONS ? selectedColor : optionColor));
                if (instrSurface) {
                    SDL_Texture* instrTexture = SDL_CreateTextureFromSurface(renderer, instrSurface);
                    SDL_Rect instrRect = {(SCREEN_WIDTH - instrSurface->w) / 2, menuStartY + optionSpacing, instrSurface->w, instrSurface->h};
                    SDL_RenderCopy(renderer, instrTexture, NULL, &instrRect);
                    SDL_DestroyTexture(instrTexture);
                    SDL_FreeSurface(instrSurface);
                }

                SDL_Surface* quitSurface = TTF_RenderText_Solid(menuOptionFont, "QUIT", (currentMenuOption == MainMenuOption::QUIT_OPTION ? selectedColor : optionColor));
                if (quitSurface) {
                    SDL_Texture* quitTexture = SDL_CreateTextureFromSurface(renderer, quitSurface);
                    SDL_Rect quitRect = {(SCREEN_WIDTH - quitSurface->w) / 2, menuStartY + optionSpacing * 2, quitSurface->w, quitSurface->h};
                    SDL_RenderCopy(renderer, quitTexture, NULL, &quitRect);
                    SDL_DestroyTexture(quitTexture);
                    SDL_FreeSurface(quitSurface);
                }

            } break;
            case GameState::PLAYING:
            case GameState::POST_BOSS_EXPLOSION:
            case GameState::GAME_OVER:
            {
                SDL_Rect bgSrc={static_cast<int>(round(cameraX)), static_cast<int>(round(cameraY)), SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_Rect bgDst={0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
                if(backgroundTexture)
                {
                    SDL_RenderCopy(renderer, backgroundTexture, &bgSrc, &bgDst);
                }

                for (Enemy& e : enemies_list)
                {
                    e.render(window, cameraX, cameraY);
                }
                for (Turret& t : regularTurrets_list)
                {
                    t.render(window, cameraX, cameraY);
                }

                if (bossFightEngaged || currentGameState == GameState::POST_BOSS_EXPLOSION) {
                    for (auto& part_turret : bossPartsList) {
                        part_turret->render(window, cameraX, cameraY);
                    }
                }
                for (Bullet& b : playerBulletsList)
                {
                    b.render(window, cameraX, cameraY);
                }
                for (Bullet& eb : enemyBulletsList)
                {
                    eb.render(window, cameraX, cameraY);
                }
                if (player_ptr)
                {
                    player_ptr->render(window, cameraX, cameraY);
                }

                if ((currentGameState == GameState::PLAYING || currentGameState == GameState::POST_BOSS_EXPLOSION) && uiFont && renderer)
                {
                    SDL_Color uiTextColor = {255,255,255,255};
                    int currentY_offset = 10;
                    int textHeight;
                    string scoreStr = "SCORE: " + std::to_string(playerScore);
                    SDL_Surface* scoreSurface = TTF_RenderText_Solid(uiFont, scoreStr.c_str(), uiTextColor);
                    if(scoreSurface)
                    {
                        SDL_Texture* scoreTex = SDL_CreateTextureFromSurface(renderer,scoreSurface);
                        SDL_Rect scoreRect = {10, currentY_offset, scoreSurface->w, scoreSurface->h};
                        SDL_RenderCopy(renderer,scoreTex,NULL,&scoreRect);
                        textHeight = scoreSurface->h;
                        SDL_DestroyTexture(scoreTex);
                        SDL_FreeSurface(scoreSurface);
                        currentY_offset += textHeight + 2;
                    }
                    else
                    {
                        currentY_offset += TTF_FontHeight(uiFont) + 2;
                    }

                    string timeStr = "TIME: " + std::to_string(static_cast<int>(ceil(std::max(0.0f, currentGameTimeRemaining))));
                    SDL_Surface* timeSurface = TTF_RenderText_Solid(uiFont, timeStr.c_str(), uiTextColor);
                    if(timeSurface)
                    {
                        SDL_Texture* timeTex = SDL_CreateTextureFromSurface(renderer, timeSurface);
                        SDL_Rect timeRect = {10, currentY_offset, timeSurface->w, timeSurface->h};
                        SDL_RenderCopy(renderer, timeTex, NULL, &timeRect);
                        textHeight = timeSurface->h;
                        SDL_DestroyTexture(timeTex);
                        SDL_FreeSurface(timeSurface);
                        currentY_offset += textHeight + 5;
                    }
                    else
                    {
                        currentY_offset += TTF_FontHeight(uiFont) + 5;
                    }

                    if (player_ptr && lifeMedalTexture)
                    {
                        int livesLeft = player_ptr->getLives();
                        if (livesLeft > 0)
                        {
                            const int MEDAL_RENDER_WIDTH = 20;
                            const int MEDAL_RENDER_HEIGHT = 40;
                            int spacing = 5;
                            int topM = 10;
                            int rightM = 10;
                            for (int i = 0; i < livesLeft; ++i)
                            {
                                SDL_Rect dRect = {SCREEN_WIDTH - rightM - (i + 1) * MEDAL_RENDER_WIDTH - i * spacing, topM, MEDAL_RENDER_WIDTH, MEDAL_RENDER_HEIGHT};
                                SDL_RenderCopy(renderer, lifeMedalTexture, NULL, &dRect);
                            }
                        }
                    }

                    if (bossFightEngaged && !bossIsActuallyDefeated && currentGameState == GameState::PLAYING)
                    {
                        SDL_Color cGood = {0,200,0,255};
                        SDL_Color cMid = {255,200,0,255};
                        SDL_Color cBad = {200,0,0,255};
                        SDL_Color cBg = {50,50,50,220};
                        SDL_Color cBorder = {220,220,220,255};
                        int barW = SCREEN_WIDTH/2;
                        int barH = 20;
                        int barX = (SCREEN_WIDTH-barW)/2;
                        int barY_boss = currentY_offset;
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        SDL_Rect bgR = {barX, barY_boss, barW, barH};
                        SDL_SetRenderDrawColor(renderer, cBg.r, cBg.g, cBg.b, cBg.a);
                        SDL_RenderFillRect(renderer, &bgR);
                        float hpP = (BOSS_TOTAL_HP > 0) ? (static_cast<float>(std::max(0, bossCurrentHP)) / BOSS_TOTAL_HP) : 0.0f;
                        SDL_Rect fgR = {barX, barY_boss, static_cast<int>(barW * hpP), barH};
                        SDL_Color curC = (hpP > 0.6f) ? cGood : (hpP > 0.3f ? cMid : cBad);
                        SDL_SetRenderDrawColor(renderer, curC.r, curC.g, curC.b, curC.a);
                        SDL_RenderFillRect(renderer, &fgR);
                        SDL_SetRenderDrawColor(renderer, cBorder.r, cBorder.g, cBorder.b, cBorder.a);
                        SDL_RenderDrawRect(renderer, &bgR);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                        string bossName = "FINAL GUARDIAN";
                        SDL_Surface* bnSurf = TTF_RenderText_Solid(uiFont, bossName.c_str(), cBorder);
                        if(bnSurf)
                        {
                            SDL_Texture* bnTex = SDL_CreateTextureFromSurface(renderer, bnSurf);
                            SDL_Rect bnR = {barX + (barW - bnSurf->w)/2 , barY_boss + (barH - bnSurf->h)/2, bnSurf->w, bnSurf->h};
                            SDL_RenderCopy(renderer, bnTex, NULL, &bnR);
                            SDL_DestroyTexture(bnTex);
                            SDL_FreeSurface(bnSurf);
                        }
                    }
                }

                if (isPaused && currentGameState == GameState::PLAYING && !bossIsActuallyDefeated )
                {
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer,0,0,0,150);
                    SDL_Rect pO={0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
                    SDL_RenderFillRect(renderer,&pO);
                    SDL_Color pC={255,255,255,255};
                    string pT="PAUSED";
                    SDL_Surface* sP=TTF_RenderText_Solid(menuFont,pT.c_str(),pC);
                    if(sP)
                    {
                        SDL_Texture* tP=SDL_CreateTextureFromSurface(renderer,sP);
                        SDL_Rect dP={(SCREEN_WIDTH-sP->w)/2,(SCREEN_HEIGHT-sP->h)/2,sP->w,sP->h};
                        SDL_RenderCopy(renderer,tP,NULL,&dP);
                        SDL_DestroyTexture(tP);
                        SDL_FreeSurface(sP);
                    }
                }
                else if (currentGameState == GameState::GAME_OVER)
                {
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, 180, 0, 0, 170);
                    SDL_Rect r={0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
                    SDL_RenderFillRect(renderer,&r);
                    SDL_Color c={255,255,255,255};
                    string t1="GAME OVER";
                    string tS="FINAL SCORE: "+std::to_string(playerScore);
                    string t2="Press Enter or ESC";
                    SDL_Surface* s1=TTF_RenderText_Solid(menuFont,t1.c_str(),c);
                    SDL_Surface* sS_surf=TTF_RenderText_Solid(uiFont,tS.c_str(),c);
                    SDL_Surface* s2=TTF_RenderText_Solid(uiFont,t2.c_str(),c);
                    int yP=SCREEN_HEIGHT/2-((s1?s1->h:0)+(sS_surf?sS_surf->h:0)+(s2?s2->h:0)+20)/2;
                    if(s1)
                    {
                        SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s1);
                        SDL_Rect d={(SCREEN_WIDTH-s1->w)/2, yP, s1->w,s1->h};
                        SDL_RenderCopy(renderer,tx,NULL,&d);
                        SDL_DestroyTexture(tx);
                        SDL_FreeSurface(s1);
                        yP+=d.h+10;
                    }
                    if(sS_surf)
                    {
                        SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,sS_surf);
                        SDL_Rect d={(SCREEN_WIDTH-sS_surf->w)/2, yP, sS_surf->w,sS_surf->h};
                        SDL_RenderCopy(renderer,tx,NULL,&d);
                        SDL_DestroyTexture(tx);
                        SDL_FreeSurface(sS_surf);
                        yP+=d.h+10;
                    }
                    if(s2)
                    {
                        SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,s2);
                        SDL_Rect d={(SCREEN_WIDTH-s2->w)/2, yP, s2->w,s2->h};
                        SDL_RenderCopy(renderer,tx,NULL,&d);
                        SDL_DestroyTexture(tx);
                        SDL_FreeSurface(s2);
                    }
                }
            } break;
            case GameState::WON: {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                SDL_Color textColor = {255,255,255,255};
                SDL_Color bonusColor = {220,220,100,255};
                SDL_Color perfectColor = {100,255,100,255};
                int yPos = SCREEN_HEIGHT/5;
                int lineHeightMenu = (menuFont ? TTF_FontHeight(menuFont) : 30)+10;
                int lineHeightUI = (uiFont ? TTF_FontHeight(uiFont) : 20)+8;
                int indentX = SCREEN_WIDTH/6;
                int valueX = SCREEN_WIDTH/2 + indentX/2;

                auto renderTextCentered = [&](TTF_Font* font, const string& text, int y, SDL_Color color, int customWidth = -1) { if(!font) return 0; SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), color); if(surf){ SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf); SDL_Rect dst = {(SCREEN_WIDTH - surf->w)/2, y, surf->w, surf->h}; SDL_RenderCopy(renderer, tex, NULL, &dst); int h = surf->h; SDL_DestroyTexture(tex); SDL_FreeSurface(surf); return h; } return 0; };
                auto renderTextPair = [&](TTF_Font* font, const string& label, const string& value, int y, SDL_Color lblCol, SDL_Color valCol) { if(!font) return; SDL_Surface* sLbl = TTF_RenderText_Solid(font, label.c_str(), lblCol); SDL_Surface* sVal = TTF_RenderText_Solid(font, value.c_str(), valCol); if(sLbl){ SDL_Texture* tLbl = SDL_CreateTextureFromSurface(renderer, sLbl); SDL_Rect rLbl = {indentX, y, sLbl->w, sLbl->h}; SDL_RenderCopy(renderer, tLbl, NULL, &rLbl); SDL_DestroyTexture(tLbl); SDL_FreeSurface(sLbl); } if(sVal){ SDL_Texture* tVal = SDL_CreateTextureFromSurface(renderer, sVal); SDL_Rect rVal = {valueX, y, sVal->w, sVal->h}; SDL_RenderCopy(renderer, tVal, NULL, &rVal); SDL_DestroyTexture(tVal); SDL_FreeSurface(sVal); }};

                renderTextCentered(menuFont, "VICTORY!", yPos, {0,255,0,255});
                yPos += lineHeightMenu + 10;
                renderTextPair(menuFont, "SCORE:", std::to_string(displayedScore), yPos, textColor, textColor);
                yPos += lineHeightMenu;
                renderTextPair(menuFont, "BONUS:", "", yPos, bonusColor, bonusColor);
                yPos += lineHeightMenu;
                string timeBonusStr_label = "  TIME: " + std::to_string(timeBonusValueToAnimate_display) + "s";
                renderTextPair(uiFont, timeBonusStr_label, "+ " + std::to_string(displayedTimeBonus), yPos, bonusColor, bonusColor);
                yPos += lineHeightUI;
                string livesBonusStr_label;
                SDL_Color livesDisplayColor=bonusColor;
                if(isPerfectWin)
                {
                     livesBonusStr_label = "  LIVES: PERFECT!";
                     livesDisplayColor=perfectColor;
                }
                else
                {
                     livesBonusStr_label = "  LIVES: " + std::to_string(actualLivesForBonus_capture) + " x " + std::to_string(POINTS_PER_LIFE);
                }
                renderTextPair(uiFont, livesBonusStr_label, "+ " + std::to_string(displayedLivesBonus), yPos, livesDisplayColor, livesDisplayColor);
                yPos += lineHeightUI;
                yPos += lineHeightMenu/2;
                renderTextPair(menuFont, "TOTAL:", std::to_string(displayedTotalScore), yPos, textColor, textColor);
                yPos += lineHeightMenu * 1.5;
                if (currentWinAnimPhase == WinAnimationPhase::DONE)
                {
                    renderTextCentered(uiFont, "Press Enter or ESC to Continue", yPos, textColor);
                }
            } break;
            case GameState::INSTRUCTIONS: {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);

                SDL_Color textColor = {255, 255, 255, 255};
                SDL_Color titleHighlightColor = {255, 255, 0, 255};
                SDL_Color goodLuckColor = {0, 255, 0, 255};
                SDL_Color returnPromptColor = {200, 200, 200, 255};

                int currentY_instr = 50;
                int lineHeight_instr = 0;
                if (uiFont) {
                    lineHeight_instr = TTF_FontHeight(uiFont) + 8;
                } else {
                    lineHeight_instr = 25;
                }

                int titleLineHeight_instr = 0;
                if (menuFont) {
                    titleLineHeight_instr = TTF_FontHeight(menuFont) + 10;
                } else {
                    titleLineHeight_instr = 35;
                }

                auto renderTextLine_instr_left = [&](const string& text, int x, int y, TTF_Font* font, SDL_Color color) {
                    if (!font) return;
                    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
                    if (surface) {
                        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                        SDL_Rect destRect = {x, y, surface->w, surface->h};
                        SDL_RenderCopy(renderer, texture, NULL, &destRect);
                        SDL_DestroyTexture(texture);
                        SDL_FreeSurface(surface);
                    } else {
                        cerr << "Failed to render text: " << text << " - " << TTF_GetError() << endl;
                    }
                };

                auto renderTextLine_instr_centered = [&](const string& text, int y, TTF_Font* font, SDL_Color color) {
                    if (!font) return;
                    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
                    if (surface) {
                        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                        SDL_Rect destRect = {(SCREEN_WIDTH - surface->w) / 2, y, surface->w, surface->h};
                        SDL_RenderCopy(renderer, texture, NULL, &destRect);
                        SDL_DestroyTexture(texture);
                        SDL_FreeSurface(surface);
                    } else {
                        cerr << "Failed to render centered text: " << text << " - " << TTF_GetError() << endl;
                    }
                };

                renderTextLine_instr_centered("--- INSTRUCTIONS ---", currentY_instr, menuFont, titleHighlightColor);
                currentY_instr += titleLineHeight_instr + 10;

                renderTextLine_instr_left("Controls:", 50, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr;
                renderTextLine_instr_left("- Arrow Keys: Move Left/Right, Aim Up/Down", 70, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr;
                renderTextLine_instr_left("- Space: Jump", 70, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr;
                renderTextLine_instr_left("- F: Shoot", 70, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr;
                renderTextLine_instr_left("- D (on ground): Drop down through platform", 70, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr;
                renderTextLine_instr_left("- C (on ground): Toggle Lie Down / Stand Up", 70, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr;
                renderTextLine_instr_left("- E (on ground, not lying): Toggle Aim Straight Up", 70, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr;
                renderTextLine_instr_left("- P: Pause/Resume Game", 70, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr;
                renderTextLine_instr_left("- M: Toggle Music", 70, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr * 2;

                renderTextLine_instr_left("Objective:", 50, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr;
                renderTextLine_instr_left("- Reach the end of the stage and defeat the Final Guardian!", 70, currentY_instr, uiFont, textColor);
                currentY_instr += lineHeight_instr * 2;

                renderTextLine_instr_centered("Good Luck, Soldier!", currentY_instr, uiFont, goodLuckColor);
                currentY_instr += lineHeight_instr * 2;

                int promptY = SCREEN_HEIGHT - lineHeight_instr - 30;
                renderTextLine_instr_centered("Press ENTER or ESC to return to Main Menu", promptY, uiFont, returnPromptColor);

            } break;
        }
        window.display();

        float frameTicks_render = static_cast<float>(SDL_GetTicks()) - startTicks;
        float desiredFrameTime_ms = 1000.0f / refreshRate;
        if (frameTicks_render < desiredFrameTime_ms)
        {
            SDL_Delay(static_cast<Uint32>(desiredFrameTime_ms - frameTicks_render));
        }
    }

    cout << "Cleaning up resources..." << endl;
    delete player_ptr;
    player_ptr = nullptr;
    enemies_list.clear();
    regularTurrets_list.clear();
    bossPartsList.clear();
    playerBulletsList.clear();
    enemyBulletsList.clear();

    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(shootSound);
    Mix_FreeChunk(gEnemyDeathSound);
    Mix_FreeChunk(gPlayerDeathSound);
    Mix_FreeChunk(gTurretExplosionSound);
    Mix_FreeChunk(gTurretShootSound);
    Mix_FreeChunk(gBossMainExplosionSound);
    Mix_FreeChunk(gVictoryMusicChunk);
    Mix_FreeChunk(gScoreTickSound);

    SDL_DestroyTexture(menuBackgroundTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(playerRunTexture);
    SDL_DestroyTexture(playerJumpTexture);
    SDL_DestroyTexture(playerEnterWaterTexture);
    SDL_DestroyTexture(playerSwimTexture);
    SDL_DestroyTexture(playerStandAimShootHorizTexture);
    SDL_DestroyTexture(playerRunAimShootHorizTexture);
    SDL_DestroyTexture(playerStandAimShootUpTexture);
    SDL_DestroyTexture(playerStandAimShootDiagUpTexture);
    SDL_DestroyTexture(playerRunAimShootDiagUpTexture);
    SDL_DestroyTexture(playerStandAimShootDiagDownTexture);
    SDL_DestroyTexture(playerRunAimShootDiagDownTexture);
    SDL_DestroyTexture(playerLyingDownTexture);
    SDL_DestroyTexture(playerLyingAimShootTexture);
    SDL_DestroyTexture(playerBulletTexture);
    SDL_DestroyTexture(turretBulletTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(gameTurretTexture);
    SDL_DestroyTexture(turretExplosionTexture);
    SDL_DestroyTexture(lifeMedalTexture);

    TTF_CloseFont(uiFont);
    TTF_CloseFont(menuFont);
    TTF_CloseFont(debugFont);
    TTF_CloseFont(menuOptionFont);
    TTF_Quit();
    Mix_CloseAudio();
    Mix_Quit();
    window.cleanUp();
    IMG_Quit();
    SDL_Quit();
    cout << "Cleanup complete. Exiting." << endl;
    return 0;


}