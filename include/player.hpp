#pragma once

#include <vector>
#include <set>       // <<< THÊM INCLUDE
#include <utility>   // <<< THÊM INCLUDE cho std::pair
#include <SDL2/SDL.h>
#include "entity.hpp"
#include "math.hpp"

class RenderWindow;

enum class PlayerState {
    IDLE, RUNNING, JUMPING, FALLING, DROPPING, // DROPPING là state khi lộn xuống
    ENTERING_WATER, SWIMMING, WATER_JUMP,
    SHOOTING_HORIZ, // Bắn ngang khi đứng yên hoặc trên không
    SHOOTING_UP,
    RUN_SHOOTING_HORIZ // Vừa chạy vừa bắn ngang
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
           SDL_Texture* p_runShootHorizTex, int p_runShootHorizSheetCols,
           int p_frameW, int p_frameH);

    void handleInput(const Uint8* keyStates); // Xử lý giữ phím
    void handleKeyDown(SDL_Keycode key); // Xử lý nhấn phím xuống (cho nhảy, lộn xuống)
    void update(double dt, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight);
    void render(RenderWindow& window, double cameraX, double cameraY);
    int getTileAt(double worldX, double worldY) const; // Lấy loại tile tại tọa độ thế giới

    SDL_Rect getWorldHitbox();
    bool wantsToShoot(vector2d& out_bulletStartPos, vector2d& out_bulletVelocity);

    // Getter để kiểm tra trạng thái (hữu ích cho debug)
    PlayerState getCurrentState() const { return currentState; }
    bool getIsOnGround() const { return isOnGround; }
    bool getIsInWater() const { return isInWaterState; }

private:
    // Textures
    SDL_Texture* runTexture;
    SDL_Texture* jumpTexture;
    SDL_Texture* enterWaterTexture;
    SDL_Texture* swimTexture;
    SDL_Texture* shootHorizTexture;
    SDL_Texture* shootUpTexture;
    SDL_Texture* runShootHorizTexture;

    // Animation properties
    int runSheetColumns;
    int jumpSheetColumns;
    int enterWaterSheetColumns;
    int swimSheetColumns;
    int shootHorizSheetColumns;
    int shootUpSheetColumns;
    int runShootHorizSheetColumns;

    SDL_Rect currentSourceRect;
    int frameWidth;
    int frameHeight;

    // Physics
    vector2d velocity;
    PlayerState currentState;
    FacingDirection facing;
    bool isOnGround;       // Đang đứng trên mặt đất/platform?
    bool isInWaterState;   // Đang ở trong trạng thái liên quan đến nước?
    double waterSurfaceY;  // Tọa độ Y của mặt nước vừa chạm vào
    SDL_Rect hitbox;       // Hitbox tương đối so với vị trí (pos)

    // Animation
    double animTimer;
    int currentAnimFrameIndex;

    // Shooting
    bool shootRequested;
    bool shootUpHeld;
    bool isShootingHeld;
    double shootCooldownTimer;
    const double SHOOT_COOLDOWN = 0.15;

    // Map data (lưu con trỏ để truy cập trong update)
    const std::vector<std::vector<int>>* currentMapData;
    int currentMapRows; // Lưu số hàng của map
    int currentMapCols; // Lưu số cột của map
    int currentTileWidth;
    int currentTileHeight;

    // Set để lưu các tile cỏ bị vô hiệu hóa tạm thời
    std::set<std::pair<int, int>> temporarilyDisabledTiles;

    // Private methods
    void applyGravity(double dt);
    void move(double dt);
    void checkMapCollision(); // Kiểm tra va chạm map
    void updateAnimation(double dt);
    void updateState();       // Cập nhật trạng thái (IDLE, RUNNING, JUMPING, etc.)
    void restoreDisabledTiles(); // Hàm khôi phục tile
    void printTileColumnInfo(); // Hàm debug (tùy chọn)

    // Physics constants
    const double GRAVITY = 980.0;
    const double MOVE_SPEED = 300.0;
    const double JUMP_STRENGTH = 600.0;
    const double MAX_FALL_SPEED = 600.0;
    const double WATER_GRAVITY_MULTIPLIER = 0.3;
    const double WATER_MAX_SPEED_MULTIPLIER = 0.5;
    const double WATER_DRAG_X = 0.85;
    const double WATER_JUMP_STRENGTH = 300.0;
    const double BULLET_SPEED = 600.0;

    // Animation constants
    const double ANIM_SPEED = 0.08;
    const int RUN_FRAMES = 6;
    const int JUMP_FRAMES = 4;
    const int ENTER_WATER_FRAMES = 4;
    const int SWIM_FRAMES = 4;
    const int SHOOT_HORIZ_FRAMES = 3;
    const int SHOOT_UP_FRAMES = 2;
    const int RUN_SHOOT_HORIZ_FRAMES = 3;

    // Giới hạn hàng dưới cùng
    const int BOTTOM_ROW_INDEX = 6; // <<< THAY ĐỔI HÀNG ĐÁY LÀ 6 >>>
};