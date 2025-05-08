#pragma once

#include <vector>
#include <set>
#include <utility>
#include <string>
#include <SDL2/SDL.h>
#include "math.hpp"

class RenderWindow; // Forward declaration

enum class PlayerState {
    IDLE, RUNNING, JUMPING, FALLING, DROPPING, ENTERING_WATER, SWIMMING, WATER_JUMP,
    STAND_AIM_HORIZ, STAND_AIM_DIAG_UP, STAND_AIM_DIAG_DOWN, RUN_AIM_HORIZ, RUN_AIM_DIAG_UP, RUN_AIM_DIAG_DOWN,
    STAND_AIM_UP, LYING_DOWN, LYING_AIM_SHOOT,
    DYING, DEAD
};
enum class FacingDirection { LEFT, RIGHT };

class Player {
public:
    // --- Constants (ĐẦY ĐỦ) ---
    const float GRAVITY = 980.0f; const float MOVE_SPEED = 300.0f;
    const float JUMP_STRENGTH = 600.0f; const float MAX_FALL_SPEED = 600.0f;
    const float WATER_GRAVITY_MULTIPLIER = 0.3f; const float WATER_MAX_SPEED_MULTIPLIER = 0.5f;
    const float WATER_DRAG_X = 0.85f; const float WATER_JUMP_STRENGTH = 300.0f;
    // const float KNOCKBACK_FORCE_X = 150.0f; // Đã xóa knockback
    // const float KNOCKBACK_FORCE_Y = 250.0f; // Đã xóa knockback
    const float BULLET_SPEED = 600.0f;
    const float BULLET_SPEED_DIAG_COMPONENT = BULLET_SPEED * 0.70710678118f;
    const float ANIM_SPEED = 0.08f; // Tốc độ anim mặc định
    // const float DEATH_ANIM_SPEED = 0.12f; // Không cần nếu chỉ nhấp nháy
    // const int DEATH_ANIM_FRAMES = 4;      // Không cần nếu chỉ nhấp nháy
    const float INVULNERABLE_DURATION = 2.0f;
    const float SHOOT_COOLDOWN = 0.15f;
    const float DYING_DURATION = 0.8f;    // Thời gian nhấp nháy
    const float BLINK_INTERVAL = 0.1f;    // Tốc độ nhấp nháy
    // --- Frame counts (ĐẦY ĐỦ) ---
    const int RUN_FRAMES = 6; const int JUMP_FRAMES = 4;
    const int ENTER_WATER_FRAMES = 1; const int SWIM_FRAMES = 5;
    const int STAND_AIM_SHOOT_HORIZ_FRAMES = 3; const int RUN_AIM_SHOOT_HORIZ_FRAMES = 3;
    const int STAND_AIM_SHOOT_UP_FRAMES = 2;
    const int STAND_AIM_SHOOT_DIAG_UP_FRAMES = 1; const int RUN_AIM_SHOOT_DIAG_UP_FRAMES = 3;
    const int STAND_AIM_SHOOT_DIAG_DOWN_FRAMES = 1; const int RUN_AIM_SHOOT_DIAG_DOWN_FRAMES = 3;
    const int LYING_DOWN_FRAMES = 1; const int LYING_AIM_SHOOT_FRAMES = 3;

    // Constructor (Đã xóa tham số death texture)
    Player(vector2d p_pos,
           SDL_Texture* p_runTex, int p_runSheetCols, SDL_Texture* p_jumpTex, int p_jumpSheetCols,
           SDL_Texture* p_enterWaterTex, int p_enterWaterSheetCols, SDL_Texture* p_swimTex, int p_swimSheetCols,
           SDL_Texture* p_standAimShootUpTex, int p_standAimShootUpSheetCols, SDL_Texture* p_standAimShootDiagUpTex, int p_standAimShootDiagUpSheetCols,
           SDL_Texture* p_standAimShootDiagDownTex, int p_standAimShootDiagDownSheetCols, SDL_Texture* p_runAimShootDiagUpTex, int p_runAimShootDiagUpSheetCols,
           SDL_Texture* p_runAimShootDiagDownTex, int p_runAimShootDiagDownSheetCols, SDL_Texture* p_standAimShootHorizTex, int p_standAimShootHorizSheetCols,
           SDL_Texture* p_runAimShootHorizTex, int p_runAimShootHorizSheetCols, SDL_Texture* p_lyingDownTex, int p_lyingDownSheetCols,
           SDL_Texture* p_lyingAimShootTex, int p_lyingAimShootSheetCols,
           // SDL_Texture* p_deathTex, int p_deathSheetCols, // Đã xóa
           int p_standardFrameW, int p_standardFrameH, int p_lyingFrameW, int p_lyingFrameH);

    // Public methods
    void update(float dt, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight);
    void render(RenderWindow& window, float cameraX, float cameraY);
    void handleInput(const Uint8* keyStates);
    void handleKeyDown(SDL_Keycode key);
    int getTileAt(float worldX, float worldY) const;
    SDL_Rect getWorldHitbox();
    bool wantsToShoot(vector2d& out_bulletStartPos, vector2d& out_bulletVelocity);
    void takeHit(bool isFallDamage);
    void respawn(float p_camX, float initialPlayerY_top, float playerStartXOffset);
    void resetPlayerStateForNewGame();
    vector2d& getPos() { return pos; }
    const vector2d& getPos() const { return pos; }
    void setPos(const vector2d& p_pos) { pos = p_pos; }
    PlayerState getCurrentState() const { return currentState; }
    bool getIsOnGround() const { return isOnGround; }
    bool getIsInWater() const { return isInWaterState; }
    bool getIsLyingDown() const { return isLyingDownState; }
    bool getIsAimingStraightUp() const { return isAimingStraightUpState; }
    bool getIsDead() const { return currentState == PlayerState::DEAD || currentState == PlayerState::DYING; }
    bool isTrulyOutOfLives() const { return lives <= 0 && currentState == PlayerState::DEAD; }
    int getLives() const { return lives; }
    bool isInvulnerable() const { return invulnerable; }
    void setInvulnerable(bool value);

private:
    // Khai báo thành viên theo thứ tự khởi tạo mong muốn
    vector2d pos;

    // --- Textures (ĐẦY ĐỦ) ---
    SDL_Texture *runTexture;
    SDL_Texture *jumpTexture;
    SDL_Texture *enterWaterTexture;
    SDL_Texture *swimTexture;
    SDL_Texture *standAimShootHorizTexture;
    SDL_Texture *runAimShootHorizTexture;
    SDL_Texture *standAimShootUpTexture;
    SDL_Texture *standAimShootDiagUpTexture;
    SDL_Texture *runAimShootDiagUpTexture;
    SDL_Texture *standAimShootDiagDownTexture;
    SDL_Texture *runAimShootDiagDownTexture;
    SDL_Texture *lyingDownTexture;
    SDL_Texture *lyingAimShootTexture;
    // SDL_Texture *deathTexture; // Đã xóa

    // --- Sheet Columns (ĐẦY ĐỦ) ---
    int runSheetColumns;
    int jumpSheetColumns;
    int enterWaterSheetColumns;
    int swimSheetColumns;
    int standAimShootHorizSheetColumns;
    int runAimShootHorizSheetColumns;
    int standAimShootUpSheetColumns;
    int standAimShootDiagUpSheetColumns;
    int runAimShootDiagUpSheetColumns;
    int standAimShootDiagDownSheetColumns;
    int runAimShootDiagDownSheetColumns;
    int lyingDownSheetColumns;
    int lyingAimShootSheetColumns;
    // int deathSheetColumns; // Đã xóa

    // Frame Dimensions & Animation
    SDL_Rect currentSourceRect;
    int standardFrameWidth, standardFrameHeight;
    int lyingFrameWidth, lyingFrameHeight;
    float animTimer;
    int currentAnimFrameIndex;

    // Physics & Collision
    vector2d velocity;
    SDL_Rect hitbox, originalStandingHitboxDef;
    bool isOnGround, isInWaterState;
    float waterSurfaceY;
    std::set<std::pair<int, int>> temporarilyDisabledTiles;

    // Game State & Input
    PlayerState currentState;
    FacingDirection facing;
    bool shootRequested, aimUpHeld, aimDownHeld, isShootingHeld;
    bool isLyingDownState, isAimingStraightUpState;
    bool wantsToLieDown, wantsToStandUp, wantsToAimStraightUp, wantsToStopAimStraightUp;
    float shootCooldownTimer;
    int lives;
    bool invulnerable;
    float invulnerableTimer;
    bool isVisible;      // Cờ nhấp nháy khi chết
    float dyingTimer;   // Timer nhấp nháy

    // Map Data (con trỏ)
    const std::vector<std::vector<int>>* currentMapData;
    int currentMapRows, currentMapCols, currentTileWidth, currentTileHeight;

    // Private Methods
    void applyGravity(float dt); void movePlayer(float dt); void checkMapCollision();
    void updateCurrentState(); void updatePlayerAnimation(float dt); void restoreDisabledTiles();
    void applyStateBasedMovementRestrictions(); PlayerState determineAimingOrShootingState() const;
};