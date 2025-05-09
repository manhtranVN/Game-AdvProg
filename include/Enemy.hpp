#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "math.hpp"
#include "RenderWindow.hpp"
#include <vector>

extern Mix_Chunk* gEnemyDeathSound;

enum class EnemyState { ALIVE, DYING, DEAD };

class Enemy {
public:
    // Constants (khai báo trước)
    const float ANIM_SPEED = 0.15f;
    const int NUM_FRAMES_WALK = 6;
    const float DYING_DURATION = 0.6f;
    const float BLINK_INTERVAL = 0.1f;
    const float MOVE_SPEED = 50.0f;
    const float GRAVITY = 980.0f;
    const float MAX_FALL_SPEED = 600.0f;

    Enemy(vector2d p_pos, SDL_Texture* p_tex);

    void update(float dt, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight);
    void render(RenderWindow& window, float cameraX, float cameraY);
    SDL_Rect getWorldHitbox() const;
    void takeHit();
    bool isAlive() const;
    bool isDead() const;
    EnemyState getState() const;

private:
    // Khai báo thành viên theo thứ tự sẽ khởi tạo
    vector2d pos;
    SDL_Texture* tex;
    SDL_Rect currentFrame;
    int frameWidth;
    int frameHeight;
    int sheetColumns;
    int currentAnimFrameIndex;
    float animTimer;

    EnemyState currentState;
    SDL_Rect hitbox;

    float velocityY;
    bool isOnGround;

    float dyingTimer;
    bool isVisible;

    bool movingRight;

    // Hàm tiện ích
    int getTileAt(float worldX, float worldY, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight) const;
};
