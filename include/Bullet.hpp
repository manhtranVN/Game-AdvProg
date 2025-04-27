#pragma once

#include <SDL2/SDL.h>
#include "math.hpp" // Giả định vector2d dùng double
#include <vector>   // Để dùng trong checkCollision

using namespace std;

class RenderWindow; // Forward declaration

class Bullet {
public:
    Bullet(vector2d p_pos, vector2d p_vel, SDL_Texture* p_tex);

    void update(double dt);
    void render(RenderWindow& window, double cameraX, double cameraY);
    bool isActive() const;
    void setActive(bool active);
    SDL_Rect getWorldHitbox() const; // Hitbox để kiểm tra va chạm

    // Hàm kiểm tra va chạm đơn giản với map
    void checkMapCollision(const vector<vector<int>>& mapData, int tileWidth, int tileHeight);

private:
    vector2d pos;
    vector2d velocity;
    SDL_Texture* tex;
    SDL_Rect currentFrame; // Giả sử đạn chỉ có 1 frame
    bool active;
    double lifeTime; // Thời gian tồn tại (tùy chọn)
    const double MAX_LIFETIME = 2.0; // Ví dụ: đạn biến mất sau 2 giây

    // Hàm tiện ích lấy tile (có thể copy từ Player hoặc để ở utils)
    int getTileAt(double worldX, double worldY, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) const;
};	