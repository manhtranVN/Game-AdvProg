#pragma once

#include <SDL2/SDL.h>
#include "math.hpp" // Giả định vector2d dùng double
#include <vector>   // Để dùng trong checkCollision

// using namespace std; // Nên tránh using namespace std trong file header

class RenderWindow; // Forward declaration

class Bullet {
public:
    // SỬA CONSTRUCTOR: Thêm p_renderW, p_renderH
    Bullet(vector2d p_pos, vector2d p_vel, SDL_Texture* p_tex, int p_renderW, int p_renderH);

    void update(double dt);
    void render(RenderWindow& window, double cameraX, double cameraY);
    bool isActive() const;
    void setActive(bool active);
    SDL_Rect getWorldHitbox() const; 

    void checkMapCollision(const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight);

private:
    vector2d pos;
    vector2d velocity;
    SDL_Texture* tex;
    SDL_Rect currentFrame;      // Source rect từ texture (kích thước gốc của sprite đạn)
    bool active;
    double lifeTime; 
    const double MAX_LIFETIME = 2.0; 

    // THÊM THÀNH VIÊN: Kích thước render mong muốn
    int renderWidth;
    int renderHeight;

    int getTileAt(double worldX, double worldY, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight) const;
};



