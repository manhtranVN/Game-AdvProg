#pragma once

#include <SDL2/SDL.h>
#include "math.hpp"
#include "RenderWindow.hpp"
#include <vector>

using namespace std;

enum class EnemyState { ALIVE, DYING, DEAD };

class Enemy {
public:
    Enemy(vector2d p_pos, SDL_Texture* p_tex);

    void update(double dt, const vector<vector<int>>& mapData, int tileWidth, int tileHeight);
    void render(RenderWindow& window, double cameraX, double cameraY);
    SDL_Rect getWorldHitbox() const;
    void takeHit();
    bool isAlive() const;
    bool isDead() const;
    EnemyState getState() const;

    // Hàm này có thể không cần thiết nữa nếu logic va chạm tích hợp vào update
    // void checkMapCollision(const vector<vector<int>>& mapData, int tileWidth, int tileHeight);


private:
    vector2d pos;
    SDL_Texture* tex;
    SDL_Rect currentFrame;
    int frameWidth;
    int frameHeight;
    int sheetColumns;
    int currentAnimFrameIndex;
    double animTimer;

    EnemyState currentState;
    SDL_Rect hitbox;

    // --- THÊM BIẾN VẬT LÝ ---
    double velocityY; // Vận tốc dọc
    bool isOnGround; // Cờ kiểm tra đang trên mặt đất

    // Cho hiệu ứng chết
    double dyingTimer;
    bool isVisible;

    // Cho AI di chuyển
    bool movingRight;

    // Hằng số
    const double ANIM_SPEED = 0.15;
    const int NUM_FRAMES = 6;
    const double DYING_DURATION = 0.6;
    const double BLINK_INTERVAL = 0.1;
    const double MOVE_SPEED = 50.0; // Tốc độ di chuyển ngang
    // <<< THÊM HẰNG SỐ VẬT LÝ (có thể dùng chung với Player) >>>
    const double GRAVITY = 980.0;
    const double MAX_FALL_SPEED = 600.0;


    // Hàm tiện ích lấy tile
    int getTileAt(double worldX, double worldY, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) const;
};