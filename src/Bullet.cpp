// Trong Bullet.cpp

#include "Bullet.hpp"
#include "RenderWindow.hpp"
#include <cmath>
#include <algorithm>
#include <iostream> // <<< THÊM INCLUDE CHO COUT

using namespace std;

// --- Định nghĩa các tile rắn ---
const int TILE_EMPTY = 0;
const int TILE_GRASS = 1;
const int TILE_UNKNOWN_SOLID = 2;
const int TILE_WATER_SURFACE = 3;

Bullet::Bullet(vector2d p_pos, vector2d p_vel, SDL_Texture* p_tex)
    : pos(p_pos), velocity(p_vel), tex(p_tex), active(true), lifeTime(0.0)
{
    // Log vận tốc ban đầu khi tạo đạn
    cout << "[DEBUG] Bullet Created. Initial Velocity: (" << velocity.x << ", " << velocity.y << ")" << endl; // <<< DEBUG LOG

    if (tex) {
        SDL_QueryTexture(tex, NULL, NULL, &currentFrame.w, &currentFrame.h);
    } else {
        currentFrame.w = 2; currentFrame.h = 2;
         cerr << "Warning: Bullet created with NULL texture!" << endl;
    }
    currentFrame.x = 0; currentFrame.y = 0;
}

void Bullet::update(double dt) {
    if (!active) return;

    // Log trước khi cập nhật vị trí
    // cout << "[DEBUG] Bullet Update Start. Pos: (" << pos.x << ", " << pos.y << "), Vel: (" << velocity.x << ", " << velocity.y << "), dt: " << dt << endl; // <<< DEBUG LOG (Có thể bật nếu cần)

    // --- Cập nhật vị trí ---
    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
    // ---------------------

    // Log sau khi cập nhật vị trí
     cout << "[DEBUG] Bullet Update End. New Pos: (" << pos.x << ", " << pos.y << ")" << endl; // <<< DEBUG LOG

    // Cập nhật thời gian tồn tại
    lifeTime += dt;
    if (lifeTime >= MAX_LIFETIME) {
        // cout << "[DEBUG] Bullet lifetime expired." << endl; // Debug log
        active = false;
    }
}

// Hàm kiểm tra va chạm đơn giản với map
void Bullet::checkMapCollision(const vector<vector<int>>& mapData, int tileWidth, int tileHeight) {
     if (!active) return;

     // Lấy tâm hoặc góc của viên đạn để kiểm tra
     // Dùng góc trên trái đơn giản hơn:
     double checkX = pos.x;
     double checkY = pos.y;
     // Hoặc dùng tâm:
     // double checkX = pos.x + static_cast<double>(currentFrame.w) / 2.0;
     // double checkY = pos.y + static_cast<double>(currentFrame.h) / 2.0;


     int tileType = getTileAt(checkX, checkY, mapData, tileWidth, tileHeight);

     // Nếu đạn va vào ô rắn (1 hoặc 2) -> hủy đạn
     if (tileType == TILE_GRASS || tileType == TILE_UNKNOWN_SOLID) {
         // cout << "[DEBUG] Bullet hit solid map tile: " << tileType << endl; // Debug log
         active = false;
     }
}


void Bullet::render(RenderWindow& window, double cameraX, double cameraY) {
    if (!active || !tex) return;

    // --- KÍCH THƯỚC MONG MUỐN KHI VẼ ---
    const int DESIRED_BULLET_WIDTH = 8;  // <<< Thay bằng chiều rộng bạn muốn
    const int DESIRED_BULLET_HEIGHT = 8; // <<< Thay bằng chiều cao bạn muốn
    // ------------------------------------

    SDL_Rect destRect;
    destRect.x = static_cast<int>(round(pos.x - cameraX));
    destRect.y = static_cast<int>(round(pos.y - cameraY));
    destRect.w = DESIRED_BULLET_WIDTH;   // <<< Sử dụng kích thước mong muốn
    destRect.h = DESIRED_BULLET_HEIGHT; // <<< Sử dụng kích thước mong muốn

    // ¤tFrame vẫn chỉ định phần nào của texture gốc sẽ được vẽ (vẫn là toàn bộ texture nếu x,y=0 và w,h=kích thước gốc)
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
    worldHB.w = currentFrame.w;
    worldHB.h = currentFrame.h;
    return worldHB;
}

// Hàm tiện ích lấy tile (giữ nguyên)
int Bullet::getTileAt(double worldX, double worldY, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) const {
    if (worldX < 0 || worldY < 0 || tileWidth <= 0 || tileHeight <= 0) return TILE_EMPTY;
    int col = static_cast<int>(floor(worldX / tileWidth));
    int row = static_cast<int>(floor(worldY / tileHeight));
    if (row >= 0 && row < mapData.size()) {
        const auto& rowData = mapData[row];
        if (col >= 0 && col < rowData.size()) { return rowData[col]; }
    }
    return TILE_EMPTY;
}