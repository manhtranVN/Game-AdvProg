#pragma once
#include <vector>
#include <set>
#include <utility>
#include <SDL2/SDL.h>
#include "entity.hpp"
#include "math.hpp"

class RenderWindow;

enum class PlayerState {
    IDLE, RUNNING, JUMPING, FALLING, DROPPING,
    ENTERING_WATER, SWIMMING, WATER_JUMP,
    AIMING_UP, AIMING_DIAG_UP, AIMING_DIAG_DOWN,
    RUN_AIMING_DIAG_UP, RUN_AIMING_DIAG_DOWN,
    SHOOTING_HORIZ, RUN_SHOOTING_HORIZ,
    SHOOTING_UP, SHOOTING_DIAG_UP, SHOOTING_DIAG_DOWN,
    LYING_DOWN, LYING_SHOOTING
};

enum class FacingDirection { LEFT, RIGHT };

class Player : public entity {
public:
    Player(vector2d p_pos,
           SDL_Texture* p_runTex, int p_runSheetCols,
           SDL_Texture* p_jumpTex, int p_jumpSheetCols,
           SDL_Texture* p_enterWaterTex, int p_enterWaterSheetCols,
           SDL_Texture* p_swimTex, int p_swimSheetCols,
           SDL_Texture* p_aimUpTex, int p_aimUpSheetCols,
           SDL_Texture* p_aimDiagUpTex, int p_aimDiagUpSheetCols,
           SDL_Texture* p_aimDiagDownTex, int p_aimDiagDownSheetCols,
           SDL_Texture* p_runAimDiagUpTex, int p_runAimDiagUpSheetCols,
           SDL_Texture* p_runAimDiagDownTex, int p_runAimDiagDownSheetCols,
           SDL_Texture* p_standShootHorizTex, int p_standShootHorizSheetCols,
           SDL_Texture* p_runShootHorizTex, int p_runShootHorizSheetCols,
           SDL_Texture* p_shootUpTex, int p_shootUpSheetCols,
           SDL_Texture* p_shootDiagUpTex, int p_shootDiagUpSheetCols,
           SDL_Texture* p_shootDiagDownTex, int p_shootDiagDownSheetCols,
           SDL_Texture* p_lyingDownTex, int p_lyingDownSheetCols,
           SDL_Texture* p_lyingShootTex, int p_lyingShootSheetCols,
           int p_standardFrameW, int p_standardFrameH,
           int p_lyingFrameW, int p_lyingFrameH);

    void handleInput(const Uint8* keyStates);
    void handleKeyDown(SDL_Keycode key);
    void update(double dt, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight);
    void render(RenderWindow& window, double cameraX, double cameraY);
    int getTileAt(double worldX, double worldY) const;

    SDL_Rect getWorldHitbox();
    bool wantsToShoot(vector2d& out_bulletStartPos, vector2d& out_bulletVelocity);

    PlayerState getCurrentState() const { return currentState; }
    bool getIsOnGround() const { return isOnGround; }
    bool getIsInWater() const { return isInWaterState; }
    bool getIsLyingDown() const { return isLyingDownState; }

private:
    // Textures
    SDL_Texture* runTexture; SDL_Texture* jumpTexture;
    SDL_Texture* enterWaterTexture; SDL_Texture* swimTexture;
    SDL_Texture* aimUpTexture; SDL_Texture* aimDiagUpTexture; SDL_Texture* aimDiagDownTexture;
    SDL_Texture* runAimDiagUpTexture; SDL_Texture* runAimDiagDownTexture;
    SDL_Texture* standShootHorizTexture; SDL_Texture* runShootHorizTexture;
    SDL_Texture* shootUpTexture; SDL_Texture* shootDiagUpTexture; SDL_Texture* shootDiagDownTexture;
    SDL_Texture* lyingDownTexture; SDL_Texture* lyingShootTexture;

    // Spritesheet Columns
    int runSheetColumns; int jumpSheetColumns;
    int enterWaterSheetColumns; int swimSheetColumns;
    int aimUpSheetColumns; int aimDiagUpSheetColumns; int aimDiagDownSheetColumns;
    int runAimDiagUpSheetColumns; int runAimDiagDownSheetColumns;
    int standShootHorizSheetColumns; int runShootHorizSheetColumns;
    int shootUpSheetColumns; int shootDiagUpSheetColumns; int shootDiagDownSheetColumns;
    int lyingDownSheetColumns; int lyingShootSheetColumns;

    SDL_Rect currentSourceRect;
    int standardFrameWidth; int standardFrameHeight;
    int lyingFrameWidth; int lyingFrameHeight;

    vector2d velocity;
    PlayerState currentState;
    FacingDirection facing;
    bool isOnGround; bool isInWaterState; double waterSurfaceY;
    SDL_Rect hitbox; SDL_Rect originalStandingHitboxDef;

    double animTimer; int currentAnimFrameIndex;

    // Input Flags
    bool shootRequested;
    bool aimUpHeld;         // Giữ phím Lên
    bool aimDownHeld;       // Giữ phím Xuống
    bool aimStraightUpHeld; // Giữ phím E
    bool isShootingHeld;    // Giữ phím F
    bool isLyingDownState;  // Trạng thái đang nằm (do phím C)
    bool wantsToLieDown;    // Yêu cầu nằm xuống từ phím C
    bool wantsToStandUp;    // Yêu cầu đứng dậy từ phím C

    double shootCooldownTimer; const double SHOOT_COOLDOWN = 0.15;

    const std::vector<std::vector<int>>* currentMapData;
    int currentMapRows; int currentMapCols;
    int currentTileWidth; int currentTileHeight;
    std::set<std::pair<int, int>> temporarilyDisabledTiles;

    void applyGravity(double dt);
    void move(double dt);
    void checkMapCollision();
    void updateAnimation(double dt);
    void updateState();
    void restoreDisabledTiles();
    void applyStateBasedMovementRestrictions(); // Hàm mới để quản lý chặn di chuyển
    PlayerState determineShootingState() const; // Hàm mới để xác định hướng bắn

    // Constants
    const double GRAVITY = 980.0; const double MOVE_SPEED = 300.0;
    const double JUMP_STRENGTH = 600.0; const double MAX_FALL_SPEED = 600.0;
    const double WATER_GRAVITY_MULTIPLIER = 0.3; const double WATER_MAX_SPEED_MULTIPLIER = 0.5;
    const double WATER_DRAG_X = 0.85; const double WATER_JUMP_STRENGTH = 300.0;
    const double BULLET_SPEED = 600.0; const double BULLET_SPEED_DIAG_COMPONENT = BULLET_SPEED * 0.70710678118;
    const double ANIM_SPEED = 0.08;

    // Frame counts (cần điều chỉnh theo spritesheet thực tế)
    const int RUN_FRAMES = 6; const int JUMP_FRAMES = 4;
    const int ENTER_WATER_FRAMES = 4; const int SWIM_FRAMES = 4;
    const int AIM_UP_FRAMES = 1; const int AIM_DIAG_UP_FRAMES = 1; const int AIM_DIAG_DOWN_FRAMES = 1;
    const int RUN_AIM_DIAG_UP_FRAMES = 3; const int RUN_AIM_DIAG_DOWN_FRAMES = 3;
    const int STAND_SHOOT_HORIZ_FRAMES = 3; const int RUN_SHOOT_HORIZ_FRAMES = 3;
    const int SHOOT_UP_FRAMES = 2; const int SHOOT_DIAG_UP_FRAMES = 2; const int SHOOT_DIAG_DOWN_FRAMES = 3;
    const int LYING_DOWN_FRAMES = 1;
    const int LYING_SHOOT_FRAMES = 3;
};