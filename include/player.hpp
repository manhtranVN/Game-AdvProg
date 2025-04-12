#pragma once

#include<vector>
#include "entity.hpp"
#include "math.hpp"

using namespace std;

class RenderWindow;

enum class PlayerState
{
	IDLE,
	RUNNING,
	JUMPING,
	FALLING,
	DROPPING
};

enum class FacingDirection
{
	LEFT,
	RIGHT
};

class Player : public entity
{
	public:
    Player(vector2d p_pos,
           SDL_Texture* p_runTex, int p_runSheetCols, 
           SDL_Texture* p_jumpTex, int p_jumpSheetCols, 
           int p_frameW, int p_frameH);

    void handleInput(const Uint8* keyStates);
    void update(double dt, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight);
    void render(RenderWindow& window, double cameraX, double cameraY);

    SDL_Rect getWorldHitbox();

private:
    SDL_Texture* runTexture;    // Texture cho chạy/đứng yên
    SDL_Texture* jumpTexture;   // Texture cho nhảy/rơi
    int runSheetColumns;
    int jumpSheetColumns;
    SDL_Rect currentSourceRect; // Rect nguồn hiện tại, thay thế việc dùng currentFrame của entity
    int frameWidth;            // Lưu trữ kích thước frame
    int frameHeight;

    vector2d velocity;
    PlayerState currentState;
    FacingDirection facing;
    bool isOnGround;
    bool requestDrop;
    SDL_Rect hitbox;
    double animTimer;
    int currentAnimFrameIndex;
    const double ANIM_SPEED = 0.1d;
    const int RUN_FRAMES = 6;
    const int JUMP_FRAMES = 4;

    void applyGravity(double dt);
    void move(double dt);
    void checkMapCollision(const vector<vector<int>>& mapData, int tileWidth, int tileHeight);
    void updateAnimation(double dt); 
    void updateState();
    int getTileAt(double worldX, double worldY, const vector<vector<int>>& mapData, int tileWidth, int tileHeight);

    const double GRAVITY = 980.0f;
    const double MOVE_SPEED = 250.0f;
    const double JUMP_STRENGTH = 500.0f;
    const double MAX_FALL_SPEED = 600.0f;
};