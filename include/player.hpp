#pragma once

#include <vector>
#include <SDL2/SDL.h>
#include "entity.hpp"
#include "math.hpp" // Giả định vector2d dùng double

using namespace std;

class RenderWindow;

enum class PlayerState {
    IDLE, RUNNING, JUMPING, FALLING, DROPPING,
    ENTERING_WATER, SWIMMING, WATER_JUMP
};

enum class FacingDirection { LEFT, RIGHT };

class Player : public entity {
public:
    Player(vector2d p_pos,
           SDL_Texture* p_runTex, int p_runSheetCols,
           SDL_Texture* p_jumpTex, int p_jumpSheetCols,
           SDL_Texture* p_enterWaterTex, int p_enterWaterSheetCols,
           SDL_Texture* p_swimTex, int p_swimSheetCols,
           int p_frameW, int p_frameH);

    void handleInput(const Uint8* keyStates);
    void update(double dt, const vector<vector<int>>& mapData, int tileWidth, int tileHeight);
    void render(RenderWindow& window, double cameraX, double cameraY);

    SDL_Rect getWorldHitbox();

private:
    SDL_Texture* runTexture;
    SDL_Texture* jumpTexture;
    SDL_Texture* enterWaterTexture;
    SDL_Texture* swimTexture;
    int runSheetColumns;
    int jumpSheetColumns;
    int enterWaterSheetColumns;
    int swimSheetColumns;
    SDL_Rect currentSourceRect;
    int frameWidth;
    int frameHeight;

    vector2d velocity; // Giả định vector2d.x, vector2d.y là double
    PlayerState currentState;
    FacingDirection facing;
    bool isOnGround;
    bool requestDrop;
    bool isInWaterState;
    double waterSurfaceY;

    SDL_Rect hitbox;
    double animTimer;
    int currentAnimFrameIndex;

    const double ANIM_SPEED = 0.1;
    const int RUN_FRAMES = 6;
    const int JUMP_FRAMES = 4;
    const int ENTER_WATER_FRAMES = 4;
    const int SWIM_FRAMES = 4;

    void applyGravity(double dt);
    void move(double dt);
    void checkMapCollision(const vector<vector<int>>& mapData, int tileWidth, int tileHeight);
    void updateAnimation(double dt);
    void updateState();
    int getTileAt(double worldX, double worldY, const vector<vector<int>>& mapData, int tileWidth, int tileHeight);

    const double GRAVITY = 980.0;
    const double MOVE_SPEED = 300.0;
    const double JUMP_STRENGTH = 500.0;
    const double MAX_FALL_SPEED = 600.0;
    const double WATER_GRAVITY_MULTIPLIER = 0.3;
    const double WATER_MAX_SPEED_MULTIPLIER = 0.5;
    const double WATER_DRAG_X = 0.85;
    const double WATER_JUMP_STRENGTH = 300.0;
};