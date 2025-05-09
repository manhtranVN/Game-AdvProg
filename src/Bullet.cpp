#include "Bullet.hpp"
#include "RenderWindow.hpp"
#include <cmath>
#include <algorithm>
#include <iostream> 

// using namespace std; // Bỏ nếu đã bỏ trong .hpp

// --- Định nghĩa các tile rắn (giờ không còn cần thiết cho va chạm đạn với map) ---
// const int TILE_EMPTY = 0;
// const int TILE_GRASS = 1;
// const int TILE_UNKNOWN_SOLID = 2;
// const int TILE_WATER_SURFACE = 3;

Bullet::Bullet(vector2d p_pos, vector2d p_vel, SDL_Texture* p_tex, int p_renderW, int p_renderH)
    : pos(p_pos), velocity(p_vel), tex(p_tex), active(true), lifeTime(0.0),
      renderWidth(p_renderW), renderHeight(p_renderH) 
{
    std::cout << "[DEBUG] Bullet Created. Pos: (" << pos.x << ", " << pos.y 
              << "), Vel: (" << velocity.x << ", " << velocity.y 
              << "), RenderSize: (" << renderWidth << ", " << renderHeight << ")" << std::endl;

    if (tex) {
        SDL_QueryTexture(tex, NULL, NULL, &currentFrame.w, &currentFrame.h);
    } else {
        currentFrame.w = 2; 
        currentFrame.h = 2;
        std::cerr << "Warning: Bullet created with NULL texture!" << std::endl;
    }
    currentFrame.x = 0; 
    currentFrame.y = 0;
}

void Bullet::update(double dt) {
    if (!active) return;

    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
    
    lifeTime += dt;
    if (lifeTime >= MAX_LIFETIME) {
        active = false;
    }
}

void Bullet::render(RenderWindow& window, double cameraX, double cameraY) {
    if (!active || !tex) return;

    SDL_Rect destRect;
    destRect.x = static_cast<int>(round(pos.x - cameraX));
    destRect.y = static_cast<int>(round(pos.y - cameraY));
    destRect.w = this->renderWidth;   
    destRect.h = this->renderHeight; 

    SDL_RenderCopy(window.getRenderer(), tex, &currentFrame, &destRect);
}

bool Bullet::isActive() const {
    return active;
}

void Bullet::setActive(bool p_active) {
    active = p_active;
}

SDL_Rect Bullet::getWorldHitbox() const {
    SDL_Rect worldHB;
    worldHB.x = static_cast<int>(round(pos.x));
    worldHB.y = static_cast<int>(round(pos.y));
    worldHB.w = this->renderWidth;
    worldHB.h = this->renderHeight;
    return worldHB;
}

// Hàm getTileAt vẫn có thể hữu ích cho các mục đích khác, không nhất thiết phải xóa
// Nhưng nó không còn được gọi bởi checkMapCollision của Bullet nữa
int Bullet::getTileAt(double worldX, double worldY, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight) const {
    // Định nghĩa các hằng số tile ở đây hoặc đảm bảo chúng được include/global
    const int TILE_EMPTY_FOR_GETTILE = 0; 

    if (worldX < 0 || worldY < 0 || tileWidth <= 0 || tileHeight <= 0) return TILE_EMPTY_FOR_GETTILE;
    int col = static_cast<int>(floor(worldX / tileWidth));
    int row = static_cast<int>(floor(worldY / tileHeight));
    if (row >= 0 && static_cast<size_t>(row) < mapData.size()) {
        const auto& rowData = mapData[row];
        if (col >= 0 && static_cast<size_t>(col) < rowData.size()) { return rowData[col]; }
    }
    return TILE_EMPTY_FOR_GETTILE;
}
