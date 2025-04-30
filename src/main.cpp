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

// --- BẬT TÍNH NĂNG DEBUG ---
// #define DEBUG_DRAW_GRID     // Vẽ lưới tilemap
// #define DEBUG_DRAW_COLUMNS  // Vẽ số thứ tự cột tilemap
// --- --- --- --- --- --- ---

// Game State Enum
enum class GameState {
    MAIN_MENU,
    PLAYING,
    WON
};

// --- Logical Tile Configuration ---
const int LOGICAL_TILE_WIDTH = 96;
const int LOGICAL_TILE_HEIGHT = 96;

// --- Map Data (Collision Grid) ---
// Dữ liệu mapData gốc của bạn
vector<vector<int>> mapData = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
    // Hàng 7 (index 7) là hàng đáy
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
    const int SCREEN_WIDTH = 1024; const int SCREEN_HEIGHT = 672;
    RenderWindow window("Contra Clone by Me :)", SCREEN_WIDTH, SCREEN_HEIGHT);
    int refreshRate = window.getRefreshRate(); if (refreshRate <= 0) refreshRate = 60;
    cout << "Refresh Rate: " << refreshRate << endl;

    // --- Nạp Font ---
    TTF_Font* uiFont = TTF_OpenFont("res/font/kongtext.ttf", 24);
    TTF_Font* menuFont = TTF_OpenFont("res/font/kongtext.ttf", 28);
    TTF_Font* debugFont = TTF_OpenFont("res/font/kongtext.ttf", 16);
    if (uiFont == nullptr || menuFont == nullptr || debugFont == nullptr) {
        cerr << "Failed to load font(s)! TTF_Error: " << TTF_GetError() << endl;
        TTF_Quit(); Mix_CloseAudio(); IMG_Quit(); SDL_Quit(); _sleep(10000); return 1;
    }
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
    SDL_Texture* playerRunShootHorizTexture = window.loadTexture("res/gfx/Shooting.png"); // Sử dụng lại Shooting.png cho chạy bắn
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
        // Dọn dẹp tài nguyên đã load thành công
        SDL_DestroyTexture(menuBackgroundTexture); SDL_DestroyTexture(backgroundTexture); SDL_DestroyTexture(playerRunTexture); SDL_DestroyTexture(playerJumpTexture); SDL_DestroyTexture(playerEnterWaterTexture); SDL_DestroyTexture(playerSwimTexture); SDL_DestroyTexture(playerShootHorizTexture); SDL_DestroyTexture(playerShootUpTexture); SDL_DestroyTexture(playerRunShootHorizTexture); SDL_DestroyTexture(bulletTexture); SDL_DestroyTexture(enemyTexture);
        Mix_FreeMusic(backgroundMusic); Mix_FreeChunk(shootSound); Mix_FreeChunk(gEnemyDeathSound);
        TTF_CloseFont(uiFont); TTF_CloseFont(menuFont); TTF_CloseFont(debugFont);
        TTF_Quit(); Mix_CloseAudio(); IMG_Quit(); SDL_Quit(); _sleep(10000); return 1;
    }
    cout << "Resources loaded." << endl;


    // --- Lấy kích thước ảnh nền game ---
    int BG_TEXTURE_WIDTH = 0, BG_TEXTURE_HEIGHT = 0;
    if(backgroundTexture) SDL_QueryTexture(backgroundTexture, NULL, NULL, &BG_TEXTURE_WIDTH, &BG_TEXTURE_HEIGHT);
    cout << "Background Texture Size: " << BG_TEXTURE_WIDTH << "x" << BG_TEXTURE_HEIGHT << endl;

    // --- Cấu hình Player ---
    const int PLAYER_FRAME_W = 40; const int PLAYER_FRAME_H = 78;
    const int PLAYER_RUN_SHEET_COLS = 6; const int PLAYER_JUMP_SHEET_COLS = 4;
    const int PLAYER_ENTER_WATER_SHEET_COLS = 1; const int PLAYER_SWIM_SHEET_COLS = 5;
    const int PLAYER_SHOOT_HORIZ_SHEET_COLS = 3; const int PLAYER_SHOOT_UP_SHEET_COLS = 2;
    const int PLAYER_RUN_SHOOT_HORIZ_SHEET_COLS = 3; // Số cột cho anim chạy bắn

    // --- Trạng thái Game ban đầu ---
    const double INITIAL_CAMERA_X = 0.0; // Bắt đầu camera từ 0
    const double INITIAL_CAMERA_Y = 0.0;
    const double PLAYER_START_X = 100.0; // Vị trí X ban đầu của player
    const double PLAYER_START_Y = 100.0; // Vị trí Y ban đầu của player (sẽ rơi xuống đất)
    double cameraX = INITIAL_CAMERA_X;
    double cameraY = INITIAL_CAMERA_Y;
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
        bullets.clear();
        enemies.clear();
        if (player_ptr) { delete player_ptr; player_ptr = nullptr; }

        cameraX = INITIAL_CAMERA_X; cameraY = INITIAL_CAMERA_Y;
        gameWon = false; isPaused = false;

        player_ptr = new Player({PLAYER_START_X, PLAYER_START_Y}, // Sử dụng vị trí khởi tạo mới
                              playerRunTexture, PLAYER_RUN_SHEET_COLS,
                              playerJumpTexture, PLAYER_JUMP_SHEET_COLS,
                              playerEnterWaterTexture, PLAYER_ENTER_WATER_SHEET_COLS,
                              playerSwimTexture, PLAYER_SWIM_SHEET_COLS,
                              playerShootHorizTexture, PLAYER_SHOOT_HORIZ_SHEET_COLS,
                              playerShootUpTexture, PLAYER_SHOOT_UP_SHEET_COLS,
                              playerRunShootHorizTexture, PLAYER_RUN_SHOOT_HORIZ_SHEET_COLS, // Thêm texture chạy bắn
                              PLAYER_FRAME_W, PLAYER_FRAME_H);

        auto spawnEnemy = [&](double worldX, int groundRow) {
             double enemyHeight = 72; // Chiều cao enemy (ví dụ)
             double groundY = groundRow * LOGICAL_TILE_HEIGHT; // Y của *mặt trên* hàng groundRow
             double spawnY = groundY - enemyHeight; // Đặt chân enemy lên mặt đất
             cout << "[Spawn] Spawning enemy at X=" << worldX << ", targeting groundRow=" << groundRow << " (groundY=" << groundY << "). Calculated spawnY=" << spawnY << endl;
             enemies.emplace_back(vector2d{worldX, spawnY}, enemyTexture);
        };
        // Spawn enemies (điều chỉnh hàng cho phù hợp mapData)
        spawnEnemy(8.0 * LOGICAL_TILE_WIDTH, 3);
        spawnEnemy(10.0 * LOGICAL_TILE_WIDTH, 3);
        spawnEnemy(15.0 * LOGICAL_TILE_WIDTH, 3);
        spawnEnemy(20.0 * LOGICAL_TILE_WIDTH, 3);
        spawnEnemy(25.0 * LOGICAL_TILE_WIDTH, 3);
        spawnEnemy(30.0 * LOGICAL_TILE_WIDTH, 3);
        spawnEnemy(50.0 * LOGICAL_TILE_WIDTH, 2);
        spawnEnemy(55.0 * LOGICAL_TILE_WIDTH, 3);
        // Thêm các enemy khác nếu muốn

        initialEnemyCount = enemies.size();
        cout << "Game Initialized. Spawned " << initialEnemyCount << " enemies." << endl;

        if (!Mix_PlayingMusic()) {
             if (Mix_PlayMusic(backgroundMusic, -1) == -1) { cerr << "Mix_PlayMusic Error: " << Mix_GetError() << endl; }
             else { isMusicPlaying = true; }
        } else if (!isMusicPlaying) { Mix_ResumeMusic(); isMusicPlaying = true; }
        else { isMusicPlaying = true; }
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
        if(frameTime > 0.25) frameTime = 0.25; // Clamp max frame time
        currentTime = newTime;

        // --- Xử lý Sự kiện SDL ---
        while(SDL_PollEvent(&event)) {
             if(event.type == SDL_QUIT) gameRunning = false;

             switch (currentGameState) {
                case GameState::MAIN_MENU:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                            currentGameState = GameState::PLAYING;
                            initializeGame(); // Khởi tạo game khi bắt đầu
                            // if(menuConfirmSound) Mix_PlayChannel(-1, menuConfirmSound, 0);
                        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                            gameRunning = false;
                        }
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
                         else if (!isPaused && player_ptr) {
                              // Xử lý nhấn phím cho player (nhảy, rơi)
                              player_ptr->handleKeyDown(event.key.keysym.sym);
                         }
                         else if (event.key.keysym.sym == SDLK_ESCAPE) { // Thoát game từ màn hình chơi
                             gameRunning = false;
                         }
                     }
                     // Xử lý giữ phím cho player (di chuyển, bắn) -> chuyển ra ngoài vòng lặp event
                     break;

                 case GameState::WON:
                      if (event.type == SDL_KEYDOWN) {
                          if (event.key.keysym.sym == SDLK_ESCAPE) {
                              gameRunning = false;
                          } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                               currentGameState = GameState::MAIN_MENU;
                               // Dừng nhạc thắng (nếu có), chuẩn bị cho nhạc menu/game
                               // Mix_HaltChannel(-1); // Dừng mọi hiệu ứng âm thanh
                          }
                      }
                      break;
             }
        } // Kết thúc SDL_PollEvent

        // --- Cập nhật Logic Game ---
        if (currentGameState == GameState::PLAYING && !isPaused) {
            accumulator += frameTime;

            // Xử lý giữ phím cho player (ngoài vòng lặp event)
            const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
            if(player_ptr) player_ptr->handleInput(currentKeyStates);

            // Fixed Timestep Loop
            while(accumulator >= timeStep) {
                // Cập nhật Player
                if(player_ptr) player_ptr->update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);

                // Ngăn player đi lùi khỏi camera (hoặc màn hình)
                if(player_ptr) {
                    vector2d& playerPos = player_ptr->getPos();
                    playerPos.x = max(cameraX, playerPos.x); // Không cho player đi về bên trái camera
                }

                // Cập nhật Enemies
                for (Enemy& enemy : enemies) {
                    enemy.update(timeStep, mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                }

                // Cập nhật Bullets
                for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); ++it_bullet) {
                    it_bullet->update(timeStep);
                }

                // Xử lý va chạm Đạn với Enemy và Map
                for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); ) {
                     if (!it_bullet->isActive()) {
                         ++it_bullet; // Bỏ qua đạn không hoạt động
                         continue;
                     }

                     SDL_Rect bulletHB = it_bullet->getWorldHitbox();
                     bool bullet_hit_someone = false;

                     // Va chạm Đạn với Enemy
                     for (auto it_enemy = enemies.begin(); it_enemy != enemies.end(); ++it_enemy) {
                         if (it_enemy->isAlive()) {
                             SDL_Rect enemyHB = it_enemy->getWorldHitbox();
                             if (SDL_HasIntersection(&bulletHB, &enemyHB)) {
                                 it_enemy->takeHit(); // Enemy nhận sát thương
                                 it_bullet->setActive(false); // Vô hiệu hóa đạn
                                 bullet_hit_someone = true;
                                 if(gEnemyDeathSound && it_enemy->isDead()) { // Phát âm thanh nếu enemy chết
                                     Mix_PlayChannel(-1, gEnemyDeathSound, 0);
                                 }
                                 break; // Một đạn chỉ trúng một enemy
                             }
                         }
                     }

                     // Nếu đạn không trúng enemy, kiểm tra va chạm với map
                     if (!bullet_hit_someone) {
                         it_bullet->checkMapCollision(mapData, LOGICAL_TILE_WIDTH, LOGICAL_TILE_HEIGHT);
                     }

                     // Xóa đạn nếu không còn hoạt động (đã trúng hoặc ra khỏi màn hình/map)
                     if (!it_bullet->isActive()) {
                         it_bullet = bullets.erase(it_bullet);
                     } else {
                         ++it_bullet;
                     }
                } // Kết thúc vòng lặp xử lý đạn

                accumulator -= timeStep;
            } // Kết thúc Fixed Timestep

            // Xóa Enemies đã chết
            enemies.remove_if([](const Enemy& enemy) { return enemy.isDead(); });

            // Kiểm tra điều kiện thắng
            if (enemies.empty() && initialEnemyCount > 0 && !gameWon) { // Chỉ thắng 1 lần
                cout << "*********************\n***   YOU WIN!    ***\n*********************" << endl;
                currentGameState = GameState::WON;
                gameWon = true; // Đánh dấu đã thắng
                Mix_HaltMusic(); // Dừng nhạc nền
                isMusicPlaying = false;
                // if(winSound) Mix_PlayChannel(-1, winSound, 0); // Phát nhạc thắng
            }

            // Tạo đạn mới nếu Player yêu cầu
            if (player_ptr) {
                 vector2d bulletStartPos, bulletVelocity;
                 if (player_ptr->wantsToShoot(bulletStartPos, bulletVelocity)) {
                      bullets.emplace_back(bulletStartPos, bulletVelocity, bulletTexture);
                      if(shootSound) Mix_PlayChannel(-1, shootSound, 0); // Phát âm thanh bắn
                 }
            }

            // Cập nhật Camera theo Player
            if(player_ptr){
                double playerCenterX = player_ptr->getPos().x + static_cast<double>(PLAYER_FRAME_W) / 2.0;
                // Di chuyển camera sang phải khi player vượt quá giữa màn hình
                double targetCameraX = playerCenterX - static_cast<double>(SCREEN_WIDTH) / 2.0;
                // Chỉ di chuyển camera về phía trước, không lùi lại
                if (targetCameraX > cameraX) {
                    cameraX = targetCameraX;
                }
                 // (Không cần camera Y trong game này)
                 // cameraY = ...;
            }

        } // Kết thúc if(PLAYING && !isPaused)

        // --- Clamp Camera (Giới hạn camera trong biên của map) ---
        if(currentGameState != GameState::MAIN_MENU){
            cameraX = max(0.0, cameraX); // Không cho camera đi nhỏ hơn 0
            cameraY = max(0.0, cameraY);

            // Giới hạn camera bên phải và dưới dựa trên kích thước ảnh nền (BG)
            if (BG_TEXTURE_WIDTH > 0 && BG_TEXTURE_WIDTH > SCREEN_WIDTH) {
                cameraX = min(cameraX, static_cast<double>(BG_TEXTURE_WIDTH - SCREEN_WIDTH));
            } else {
                cameraX = 0.0; // Nếu ảnh nền nhỏ hơn màn hình, camera không di chuyển ngang
            }
            if (BG_TEXTURE_HEIGHT > 0 && BG_TEXTURE_HEIGHT > SCREEN_HEIGHT) {
                cameraY = min(cameraY, static_cast<double>(BG_TEXTURE_HEIGHT - SCREEN_HEIGHT));
            } else {
                cameraY = 0.0; // Nếu ảnh nền nhỏ hơn màn hình, camera không di chuyển dọc
            }
        }

        // --- Vẽ ---
        window.clear(); // Xóa màn hình

        switch (currentGameState) {
            case GameState::MAIN_MENU: {
                // Vẽ nền menu
                SDL_RenderCopy(window.getRenderer(), menuBackgroundTexture, NULL, NULL);
                // Vẽ chữ hướng dẫn
                SDL_Color textColor = {255, 255, 255, 255}; // Màu trắng
                string menuText = "PRESS ENTER TO START";
                SDL_Surface* textSurface = TTF_RenderText_Solid(menuFont, menuText.c_str(), textColor);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(window.getRenderer(), textSurface);
                    if (textTexture) {
                        int textW = textSurface->w;
                        int textH = textSurface->h;
                        SDL_Rect textDestRect = {(SCREEN_WIDTH - textW) / 2, SCREEN_HEIGHT - textH - 60, textW, textH};
                        SDL_RenderCopy(window.getRenderer(), textTexture, NULL, &textDestRect);
                        SDL_DestroyTexture(textTexture);
                    } else { cerr << "Failed to create menu text texture! SDL_Error: " << SDL_GetError() << endl; }
                    SDL_FreeSurface(textSurface);
                } else { cerr << "Failed to render menu text surface! TTF_Error: " << TTF_GetError() << endl; }
                break;
            }

            case GameState::PLAYING:
            case GameState::WON: { // Vẽ màn hình game cho cả PLAYING và WON
                // Render Background (cuộn theo camera)
                SDL_Rect bgSrcRect = {static_cast<int>(round(cameraX)), static_cast<int>(round(cameraY)), SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_Rect bgDestRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                if(backgroundTexture) SDL_RenderCopy(window.getRenderer(), backgroundTexture, &bgSrcRect, &bgDestRect);

                // --- DEBUG RENDERING ---
                SDL_Renderer* renderer = window.getRenderer();
                #ifdef DEBUG_DRAW_GRID
                    if (renderer) { /* ... Code vẽ lưới tilemap ... */ }
                #endif
                #ifdef DEBUG_DRAW_COLUMNS
                    if (renderer && debugFont) { /* ... Code vẽ số cột và tile type ... */ }
                #endif
                // --- --- --- --- --- ---

                // Render Game Objects (theo thứ tự vẽ)
                for (Enemy& enemy : enemies) { enemy.render(window, cameraX, cameraY); }
                for(Bullet& bullet : bullets) { bullet.render(window, cameraX, cameraY); }
                if(player_ptr) player_ptr->render(window, cameraX, cameraY);

                // Render UI (ví dụ: số lượng enemy còn lại)
                if (currentGameState == GameState::PLAYING && uiFont) {
                    SDL_Color uiColor = {255, 255, 255, 255}; // Màu trắng
                    string enemyCountText = "Enemies: " + std::to_string(enemies.size());
                    SDL_Surface* surface = TTF_RenderText_Solid(uiFont, enemyCountText.c_str(), uiColor);
                    if (surface) {
                        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                        if (texture) {
                            int textW = surface->w; int textH = surface->h;
                            SDL_Rect dest = {10, 10, textW, textH}; // Vị trí góc trên trái
                            SDL_RenderCopy(renderer, texture, NULL, &dest);
                            SDL_DestroyTexture(texture);
                        } else { cerr << "Failed create enemy count texture! SDL_Error: " << SDL_GetError() << endl; }
                        SDL_FreeSurface(surface);
                    } else { cerr << "Failed render enemy count surface! TTF_Error: " << TTF_GetError() << endl; }
                }

                // Render Overlay (Pause hoặc Won)
                if (isPaused && currentGameState == GameState::PLAYING) {
                     // Vẽ màn hình Pause (ví dụ: làm tối màn hình và vẽ chữ "PAUSED")
                     if(renderer && menuFont){
                         SDL_Color oldColor; SDL_GetRenderDrawColor(renderer, &oldColor.r, &oldColor.g, &oldColor.b, &oldColor.a);
                         SDL_BlendMode oldBlendMode; SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);

                         SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                         SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // Màu đen trong suốt
                         SDL_Rect pauseOverlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                         SDL_RenderFillRect(renderer, &pauseOverlay);

                         SDL_Color pauseTextColor = { 255, 255, 255, 255 }; // Màu trắng
                         string pauseText = "PAUSED";
                         SDL_Surface* surf = TTF_RenderText_Solid(menuFont, pauseText.c_str(), pauseTextColor);
                         if(surf) {
                             SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                             if(tex) {
                                 int textW = surf->w; int textH = surf->h;
                                 SDL_Rect dst = {(SCREEN_WIDTH - textW) / 2, (SCREEN_HEIGHT - textH) / 2, textW, textH};
                                 SDL_RenderCopy(renderer, tex, NULL, &dst);
                                 SDL_DestroyTexture(tex);
                             }
                             SDL_FreeSurface(surf);
                         }
                         SDL_SetRenderDrawColor(renderer, oldColor.r, oldColor.g, oldColor.b, oldColor.a);
                         SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
                     }
                }
                else if (currentGameState == GameState::WON) {
                     // Vẽ màn hình thắng
                     if(renderer && menuFont && uiFont){
                         SDL_Color oldColor; SDL_GetRenderDrawColor(renderer, &oldColor.r, &oldColor.g, &oldColor.b, &oldColor.a);
                         SDL_BlendMode oldBlendMode; SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
                         SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                         SDL_SetRenderDrawColor(renderer, 0, 200, 0, 120); // Màu xanh lá trong suốt
                         SDL_Rect winOverlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                         SDL_RenderFillRect(renderer, &winOverlay);

                         SDL_Color winTextColor = { 255, 255, 0, 255 }; // Màu vàng
                         string winText1 = "YOU WIN!";
                         string winText2 = "Press Enter for Menu or ESC to Exit";

                         SDL_Surface* surf1 = TTF_RenderText_Solid(menuFont, winText1.c_str(), winTextColor);
                         SDL_Surface* surf2 = TTF_RenderText_Solid(uiFont, winText2.c_str(), winTextColor);

                         if(surf1 && surf2){
                             SDL_Texture* tex1 = SDL_CreateTextureFromSurface(renderer, surf1);
                             SDL_Texture* tex2 = SDL_CreateTextureFromSurface(renderer, surf2);
                             if(tex1 && tex2){
                                 int w1 = surf1->w, h1 = surf1->h;
                                 int w2 = surf2->w, h2 = surf2->h;
                                 SDL_Rect dst1 = {(SCREEN_WIDTH - w1)/2, SCREEN_HEIGHT/2 - h1, w1, h1};
                                 SDL_Rect dst2 = {(SCREEN_WIDTH - w2)/2, SCREEN_HEIGHT/2 + 10, w2, h2};
                                 SDL_RenderCopy(renderer, tex1, NULL, &dst1);
                                 SDL_RenderCopy(renderer, tex2, NULL, &dst2);
                                 SDL_DestroyTexture(tex1);
                                 SDL_DestroyTexture(tex2);
                             }
                         }
                         if(surf1) SDL_FreeSurface(surf1);
                         if(surf2) SDL_FreeSurface(surf2);

                         SDL_SetRenderDrawColor(renderer, oldColor.r, oldColor.g, oldColor.b, oldColor.a);
                         SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
                     }
                }
                break;
            } // Kết thúc case PLAYING/WON
        } // Kết thúc switch vẽ

        window.display(); // Hiển thị mọi thứ đã vẽ lên màn hình

        // --- Limit FPS ---
        int frameTicks = SDL_GetTicks() - startTicks;
        int desiredFrameTime = 1000 / refreshRate;
        if (frameTicks < desiredFrameTime) {
            SDL_Delay(desiredFrameTime - frameTicks);
        }

    } // Kết thúc Vòng lặp Game Chính

    // --- Cleanup ---
    cout << "Cleaning up resources..." << endl;
    if(player_ptr) delete player_ptr;
    enemies.clear(); // Đảm bảo list enemy rỗng
    bullets.clear(); // Đảm bảo list bullet rỗng

    Mix_FreeMusic(backgroundMusic); backgroundMusic = nullptr;
    Mix_FreeChunk(shootSound); shootSound = nullptr;
    Mix_FreeChunk(gEnemyDeathSound); gEnemyDeathSound = nullptr;
    // if(winSound) Mix_FreeChunk(winSound); winSound = nullptr;
    // if(menuConfirmSound) Mix_FreeChunk(menuConfirmSound); menuConfirmSound = nullptr;

    SDL_DestroyTexture(menuBackgroundTexture); menuBackgroundTexture = nullptr;
    SDL_DestroyTexture(backgroundTexture); backgroundTexture = nullptr;
    SDL_DestroyTexture(playerRunTexture); playerRunTexture = nullptr;
    SDL_DestroyTexture(playerJumpTexture); playerJumpTexture = nullptr;
    SDL_DestroyTexture(playerEnterWaterTexture); playerEnterWaterTexture = nullptr;
    SDL_DestroyTexture(playerSwimTexture); playerSwimTexture = nullptr;
    SDL_DestroyTexture(playerShootHorizTexture); playerShootHorizTexture = nullptr;
    SDL_DestroyTexture(playerShootUpTexture); playerShootUpTexture = nullptr;
    SDL_DestroyTexture(playerRunShootHorizTexture); playerRunShootHorizTexture = nullptr;
    SDL_DestroyTexture(bulletTexture); bulletTexture = nullptr;
    SDL_DestroyTexture(enemyTexture); enemyTexture = nullptr;

    TTF_CloseFont(uiFont); uiFont = nullptr;
    TTF_CloseFont(menuFont); menuFont = nullptr;
    TTF_CloseFont(debugFont); debugFont = nullptr;

    TTF_Quit();
    Mix_CloseAudio();
    Mix_Quit(); // Dọn dẹp SDL_mixer subsystems
    window.cleanUp(); // Dọn dẹp renderer và window
    IMG_Quit();
    SDL_Quit();
    cout << "Cleanup complete. Exiting." << endl;
    return 0;
}