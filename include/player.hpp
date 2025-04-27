#pragma once

#include <vector>
#include <SDL2/SDL.h>
#include "entity.hpp"
#include "math.hpp"

class RenderWindow;

enum class PlayerState {
    IDLE, RUNNING, JUMPING, FALLING, DROPPING,
    ENTERING_WATER, SWIMMING, WATER_JUMP,
    SHOOTING_HORIZ, // Bắn ngang khi đứng yên hoặc trên không
    SHOOTING_UP,
    RUN_SHOOTING_HORIZ // <<< STATE MỚI: Vừa chạy vừa bắn ngang
};

enum class FacingDirection { LEFT, RIGHT };

class Player : public entity {
public:
    Player(vector2d p_pos,
           SDL_Texture* p_runTex, int p_runSheetCols,
           SDL_Texture* p_jumpTex, int p_jumpSheetCols,
           SDL_Texture* p_enterWaterTex, int p_enterWaterSheetCols,
           SDL_Texture* p_swimTex, int p_swimSheetCols,
           SDL_Texture* p_shootHorizTex, int p_shootHorizSheetCols,
           SDL_Texture* p_shootUpTex, int p_shootUpSheetCols,
           SDL_Texture* p_runShootHorizTex, int p_runShootHorizSheetCols, // <<< THÊM MỚI
           int p_frameW, int p_frameH);

    void handleInput(const Uint8* keyStates);
    void handleKeyDown(SDL_Keycode key); // Vẫn cần cho nhảy, rơi platform
    void update(double dt, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight);
    void render(RenderWindow& window, double cameraX, double cameraY);
    int getTileAt(double worldX, double worldY) const;

    SDL_Rect getWorldHitbox();
    bool wantsToShoot(vector2d& out_bulletStartPos, vector2d& out_bulletVelocity);

private:
    // Textures
    SDL_Texture* runTexture;
    SDL_Texture* jumpTexture;
    SDL_Texture* enterWaterTexture;
    SDL_Texture* swimTexture;
    SDL_Texture* shootHorizTexture; // Bắn ngang khi đứng yên / trên không
    SDL_Texture* shootUpTexture;
    SDL_Texture* runShootHorizTexture; // <<< THÊM MỚI: Vừa chạy vừa bắn

    // Animation properties
    int runSheetColumns;
    int jumpSheetColumns;
    int enterWaterSheetColumns;
    int swimSheetColumns;
    int shootHorizSheetColumns;
    int shootUpSheetColumns;
    int runShootHorizSheetColumns; // <<< THÊM MỚI

    SDL_Rect currentSourceRect;
    int frameWidth;
    int frameHeight;

    // Physics
    vector2d velocity;
    PlayerState currentState;
    FacingDirection facing;
    bool isOnGround;
    bool isInWaterState;
    double waterSurfaceY;
    SDL_Rect hitbox;

    // Animation
    double animTimer;
    int currentAnimFrameIndex;

    // Shooting
    bool shootRequested; // Vẫn cần để báo cho vòng lặp game tạo đạn
    bool shootUpHeld;
    bool isShootingHeld; // <<< THÊM MỚI: Lưu trạng thái phím bắn đang giữ
    double shootCooldownTimer;
    const double SHOOT_COOLDOWN = 0.15; // Giảm cooldown một chút cho bắn liên tục?

    // Map data
    const std::vector<std::vector<int>>* currentMapData;
    int currentTileWidth;
    int currentTileHeight;

    // Private methods
    void applyGravity(double dt);
    void move(double dt);
    void checkMapCollision();
    void updateAnimation(double dt);
    void updateState(); // Sẽ sửa lại logic
    void printTileColumnInfo();

    // Physics constants
    const double GRAVITY = 980.0;
    const double MOVE_SPEED = 300.0;
    const double JUMP_STRENGTH = 500.0;
    const double DROP_STRENGTH = 50.0;
    const double MAX_FALL_SPEED = 600.0;
    const double WATER_GRAVITY_MULTIPLIER = 0.3;
    const double WATER_MAX_SPEED_MULTIPLIER = 0.5;
    const double WATER_DRAG_X = 0.85;
    const double WATER_JUMP_STRENGTH = 300.0;
    const double BULLET_SPEED = 600.0;

    // Animation constants
    const double ANIM_SPEED = 0.08; // Có thể tăng tốc animation một chút
    const int RUN_FRAMES = 6;
    const int JUMP_FRAMES = 4;
    const int ENTER_WATER_FRAMES = 4;
    const int SWIM_FRAMES = 4;
    const int SHOOT_HORIZ_FRAMES = 3; // Animation bắn khi đứng yên
    const int SHOOT_UP_FRAMES = 2;
    const int RUN_SHOOT_HORIZ_FRAMES = 3; // <<< THÊM MỚI: Số frame chạy bắn ngang
};