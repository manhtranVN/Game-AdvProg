#include "Enemy.hpp"
#include <cmath>    // Cho round, floor, min
#include <vector>   // Cho vector (dùng trong getTileAt và update)
#include <iostream> // Cho debug (cout, cerr)

using namespace std;

// Định nghĩa các loại tile (cần khớp với định nghĩa ở các file khác)
const int TILE_EMPTY = 0;
const int TILE_GRASS = 1;
const int TILE_UNKNOWN_SOLID = 2;
const int TILE_WATER_SURFACE = 3; // Có thể dùng để enemy quay đầu hoặc xử lý khác

// --- Constructor ---
Enemy::Enemy(vector2d p_pos, SDL_Texture* p_tex)
    : pos(p_pos),
      tex(p_tex),
      currentState(EnemyState::ALIVE),
      animTimer(0.0),
      currentAnimFrameIndex(0),
      velocityY(0.0),           // <<< Khởi tạo vận tốc Y
      isOnGround(false),        // <<< Khởi tạo chưa trên mặt đất
      dyingTimer(0.0),
      isVisible(true),
      movingRight(false)         // <<< KHỞI TẠO: Bắt đầu đi sang phải
{
    // Lấy kích thước từ texture và tính toán frame
    if (tex) {
        int totalWidth, totalHeight;
        SDL_QueryTexture(tex, NULL, NULL, &totalWidth, &totalHeight);
        sheetColumns = NUM_FRAMES; // Giả định số cột bằng số frame
        if (sheetColumns > 0) {
            frameWidth = totalWidth / sheetColumns;
            frameHeight = totalHeight;
        } else { // Xử lý lỗi nếu sheetColumns = 0
            frameWidth = 40; frameHeight = 72; sheetColumns = 1;
            cerr << "Warning: Enemy texture has 0 sheet columns? Using default values." << endl;
        }
    } else { // Kích thước mặc định nếu không có texture
        frameWidth = 40; frameHeight = 72; sheetColumns = NUM_FRAMES;
        cerr << "Error: Enemy created with NULL texture! Using default values." << endl;
    }

    // Đặt frame ban đầu
    currentFrame.x = 0; currentFrame.y = 0;
    currentFrame.w = frameWidth; currentFrame.h = frameHeight;

    // Thiết lập hitbox (điều chỉnh dựa trên frameWidth/frameHeight)
    hitbox.w = frameWidth - 10; hitbox.h = frameHeight - 5;
    hitbox.x = (frameWidth - hitbox.w) / 2; // Căn giữa hitbox theo chiều ngang
    hitbox.y = (frameHeight - hitbox.h);    // Đặt hitbox ở phần dưới frame
}

// --- Hàm lấy Tile tại tọa độ thế giới ---
int Enemy::getTileAt(double worldX, double worldY, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) const {
    if (worldX < 0 || worldY < 0 || tileWidth <= 0 || tileHeight <= 0 || mapData.empty()) return TILE_EMPTY;
    int col = static_cast<int>(floor(worldX / tileWidth));
    int row = static_cast<int>(floor(worldY / tileHeight));
    if (row >= 0 && row < mapData.size()) {
        const auto& rowData = mapData[row];
        if (col >= 0 && col < rowData.size()) { return rowData[col]; }
    }
    return TILE_EMPTY;
}


// --- Hàm Update (Kết hợp AI di chuyển và trọng lực) ---
void Enemy::update(double dt, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) {
    switch (currentState) {
        case EnemyState::ALIVE: { // Scope cho case ALIVE

            // 1. ÁP DỤNG TRỌNG LỰC
            if (!isOnGround) {
                velocityY += GRAVITY * dt;
                velocityY = min(velocityY, MAX_FALL_SPEED);
            }

            // 2. CẬP NHẬT VỊ TRÍ Y
            pos.y += velocityY * dt;

            // 3. KIỂM TRA VA CHẠM ĐẤT VÀ ĐIỀU CHỈNH VỊ TRÍ Y
            isOnGround = false; // Reset trước khi kiểm tra
            if (velocityY >= 0) {
                double feetX_left = pos.x + hitbox.x + 1;
                double feetX_right = pos.x + hitbox.x + hitbox.w - 1;
                double feetY = pos.y + frameHeight;

                int tileBelowLeft = getTileAt(feetX_left, feetY + 1.0, mapData, tileWidth, tileHeight);
                int tileBelowRight = getTileAt(feetX_right, feetY + 1.0, mapData, tileWidth, tileHeight);

                int tileBelow = TILE_EMPTY;
                if (tileBelowLeft == TILE_GRASS || tileBelowLeft == TILE_UNKNOWN_SOLID) tileBelow = tileBelowLeft;
                else if (tileBelowRight == TILE_GRASS || tileBelowRight == TILE_UNKNOWN_SOLID) tileBelow = tileBelowRight;

                if (tileBelow != TILE_EMPTY) {
                    int tileBelowRow = static_cast<int>(floor((feetY + 1.0) / tileHeight));
                    double tileTopY = static_cast<double>(tileBelowRow * tileHeight);
                    if (feetY >= tileTopY - 1.0) {
                        isOnGround = true;
                        velocityY = 0.0;
                        pos.y = tileTopY - frameHeight;
                    }
                }
            } // Kết thúc kiểm tra va chạm đất

            // 4. AI DI CHUYỂN NGANG (Chỉ khi đang trên mặt đất)
            if (isOnGround) {
                double checkX_ahead;
                double checkY_wall = pos.y + frameHeight / 2.0;
                double checkY_ground_ahead = pos.y + frameHeight + 1;

                if (movingRight) checkX_ahead = pos.x + frameWidth + 1;
                else checkX_ahead = pos.x - 1;

                int tileInFrontWall = getTileAt(checkX_ahead, checkY_wall, mapData, tileWidth, tileHeight);
                int tileBelowFront = getTileAt(checkX_ahead, checkY_ground_ahead, mapData, tileWidth, tileHeight);

                bool shouldTurn = false;
                if (tileInFrontWall == TILE_UNKNOWN_SOLID) shouldTurn = true;
                else if (tileBelowFront != TILE_GRASS && tileBelowFront != TILE_UNKNOWN_SOLID) shouldTurn = true;

                if (shouldTurn) {
                    movingRight = !movingRight; // Đổi hướng
                }

                // Cập nhật vị trí X
                double moveAmount = MOVE_SPEED * dt;
                if (movingRight) pos.x += moveAmount;
                else pos.x -= moveAmount;
            } // Kết thúc AI di chuyển ngang

            // 5. CẬP NHẬT ANIMATION
            animTimer += dt;
            if (animTimer >= ANIM_SPEED) {
                animTimer -= ANIM_SPEED;
                currentAnimFrameIndex = (currentAnimFrameIndex + 1) % NUM_FRAMES;
            }

            break; // Kết thúc case ALIVE
        } // Đóng scope case ALIVE

        case EnemyState::DYING:
            velocityY = 0.0; // Ngừng rơi
            dyingTimer += dt;
            if (dyingTimer >= DYING_DURATION) { currentState = EnemyState::DEAD; isVisible = false; }
            else { isVisible = (static_cast<int>(floor(dyingTimer / BLINK_INTERVAL)) % 2 == 0); }
            break;

        case EnemyState::DEAD:
            break;
    }
}

// Hàm checkMapCollision có thể không cần thiết nữa
/*
void Enemy::checkMapCollision(const vector<vector<int>>& mapData, int tileWidth, int tileHeight) { }
*/

// --- Hàm Render (<<< SỬA ĐỂ LẬT HÌNH >>>) ---
void Enemy::render(RenderWindow& window, double cameraX, double cameraY) {
    if (currentState == EnemyState::DEAD || (currentState == EnemyState::DYING && !isVisible)) return;
    if (!tex) return;

    currentFrame.x = currentAnimFrameIndex * frameWidth;
    currentFrame.y = 0;

    SDL_Rect destRect;
    destRect.x = static_cast<int>(round(pos.x - cameraX));
    destRect.y = static_cast<int>(round(pos.y - cameraY));
    destRect.w = frameWidth;
    destRect.h = frameHeight;

    // --- XÁC ĐỊNH HƯỚNG LẬT HÌNH ---
    SDL_RendererFlip flip = SDL_FLIP_NONE; // Mặc định không lật
    if (!movingRight) { // Nếu đang đi sang trái
        flip = SDL_FLIP_HORIZONTAL; // Lật ngang
    }
    // ---------------------------------

    // --- SỬ DỤNG SDL_RenderCopyEx ---
    SDL_RenderCopyEx(window.getRenderer(), tex, &currentFrame, &destRect, 0.0, NULL, flip);
    // SDL_RenderCopy(window.getRenderer(), tex, ¤tFrame, &destRect); // <<< Thay thế dòng này

    /* // Debug hitbox ... */
}

// --- Hàm lấy Hitbox ---
SDL_Rect Enemy::getWorldHitbox() const {
    SDL_Rect worldHB;
    worldHB.x = static_cast<int>(round(pos.x + hitbox.x));
    worldHB.y = static_cast<int>(round(pos.y + hitbox.y));
    worldHB.w = hitbox.w;
    worldHB.h = hitbox.h;
    return worldHB;
}

// --- Hàm nhận sát thương ---
void Enemy::takeHit() {
    if (currentState == EnemyState::ALIVE) {
        currentState = EnemyState::DYING;
        dyingTimer = 0.0;
        isVisible = true;
        // velocityY = -150.0; // Optional nảy lên
        // isOnGround = false;
    }
}

// --- Các hàm kiểm tra trạng thái ---
bool Enemy::isAlive() const { return currentState == EnemyState::ALIVE; }
bool Enemy::isDead() const { return currentState == EnemyState::DEAD; }
EnemyState Enemy::getState() const { return currentState; }