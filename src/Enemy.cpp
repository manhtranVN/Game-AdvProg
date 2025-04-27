#include "Enemy.hpp"
#include <cmath>    // Cho round, floor, min
#include <vector>   // Cho vector (dùng trong getTileAt và update)
#include <iostream> // Cho debug (cout, cerr)
#include <SDL2/SDL_mixer.h> // <<< THÊM INCLUDE SDL_mixer

using namespace std;

// Định nghĩa các loại tile (cần khớp với định nghĩa ở các file khác)
const int TILE_EMPTY = 0;
const int TILE_GRASS = 1;
const int TILE_UNKNOWN_SOLID = 2;
const int TILE_WATER_SURFACE = 3;

// <<< KHAI BÁO BIẾN TOÀN CỤC CHO ÂM THANH (Cần định nghĩa ở main.cpp) >>>
// Sử dụng 'extern' để báo rằng biến này được định nghĩa ở nơi khác
extern Mix_Chunk* gEnemyDeathSound;

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
      movingRight(false)         // <<< Khởi tạo hướng đi sang phải
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
    // Kiểm tra biên an toàn cho mapData
    if (row >= 0 && row < mapData.size()) {
        const auto& rowData = mapData[row];
        if (col >= 0 && col < rowData.size()) {
            return rowData[col]; // Trả về tile type
        }
    }
    return TILE_EMPTY; // Ngoài map hoặc lỗi -> coi như trống
}


// --- Hàm Update (Kết hợp AI di chuyển và trọng lực) ---
void Enemy::update(double dt, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) {

    // <<< THÊM LOG Ở ĐẦU UPDATE >>>
    // In vị trí và trạng thái isOnGround TRƯỚC khi tính toán vật lý của frame này
    // Chỉ in khi còn sống để tránh spam console
    // if(currentState == EnemyState::ALIVE) {
    //      cout << "[DEBUG] Enemy Update Start. PosY=" << pos.y << ", VelY=" << velocityY << ", IsOnGround=" << boolalpha << isOnGround << endl;
    // }
    // <<< KẾT THÚC LOG Ở ĐẦU UPDATE >>>


    switch (currentState) {
        case EnemyState::ALIVE: { // Scope cho case ALIVE

            // 1. ÁP DỤNG TRỌNG LỰC
            if (!isOnGround) {
                velocityY += GRAVITY * dt;
                velocityY = min(velocityY, MAX_FALL_SPEED); // Giới hạn tốc độ rơi
            }

            // 2. CẬP NHẬT VỊ TRÍ Y
            // double oldY = pos.y; // Lưu vị trí cũ để debug nếu cần
            pos.y += velocityY * dt;

            // 3. KIỂM TRA VA CHẠM ĐẤT VÀ ĐIỀU CHỈNH VỊ TRÍ Y
            bool landedThisFrame = false; // Cờ kiểm tra có chạm đất trong frame này không
            isOnGround = false; // Reset trước khi kiểm tra
            if (velocityY >= 0) { // Chỉ kiểm tra khi đang rơi hoặc đứng yên
                // Kiểm tra nhiều điểm dưới chân để ổn định hơn
                double feetX_left = pos.x + hitbox.x + 1; // Bên trong hitbox trái
                double feetX_mid = pos.x + frameWidth / 2.0; // Giữa frame
                double feetX_right = pos.x + hitbox.x + hitbox.w - 1; // Bên trong hitbox phải
                double feetY = pos.y + frameHeight; // Đáy của frame hình ảnh

                // Lấy tile dưới các điểm kiểm tra
                int tileBelowLeft = getTileAt(feetX_left, feetY + 1.0, mapData, tileWidth, tileHeight);
                int tileBelowMid = getTileAt(feetX_mid, feetY + 1.0, mapData, tileWidth, tileHeight);
                int tileBelowRight = getTileAt(feetX_right, feetY + 1.0, mapData, tileWidth, tileHeight);

                // Xác định tile thực sự đang đứng trên (ưu tiên giữa)
                int standingOnTile = TILE_EMPTY;
                double checkFeetYForSnap = feetY; // Y để tính toán snap
                if (tileBelowMid == TILE_GRASS || tileBelowMid == TILE_UNKNOWN_SOLID) {
                    standingOnTile = tileBelowMid;
                    checkFeetYForSnap = feetY; // Dùng Y của chân giữa
                } else if (tileBelowLeft == TILE_GRASS || tileBelowLeft == TILE_UNKNOWN_SOLID) {
                    standingOnTile = tileBelowLeft;
                     checkFeetYForSnap = feetY; // Vẫn dùng Y chung
                } else if (tileBelowRight == TILE_GRASS || tileBelowRight == TILE_UNKNOWN_SOLID) {
                    standingOnTile = tileBelowRight;
                    checkFeetYForSnap = feetY; // Vẫn dùng Y chung
                }


                if (standingOnTile != TILE_EMPTY) { // Nếu có chạm đất ở ít nhất 1 điểm
                    int tileBelowRow = static_cast<int>(floor((checkFeetYForSnap + 1.0) / tileHeight));
                    double tileTopY = static_cast<double>(tileBelowRow * tileHeight);

                    // Chỉ xử lý nếu chân thực sự chạm hoặc lún vào tile
                    // Dùng pos.y + frameHeight (đáy frame) để kiểm tra
                    if (pos.y + frameHeight >= tileTopY - 1.0) { // Cho phép sai số nhỏ (-1.0)
                        isOnGround = true;    // Đặt là đang trên mặt đất
                        velocityY = 0.0;      // Dừng rơi
                        // Điều chỉnh vị trí Y để đặt đáy frame ngay trên mặt đất
                        pos.y = tileTopY - frameHeight;
                        landedThisFrame = true;
                    }
                }
            } // Kết thúc kiểm tra va chạm đất

            // 4. AI DI CHUYỂN NGANG (Chỉ khi đang trên mặt đất)
            if (isOnGround) {
                double checkX_ahead; // Tọa độ X để kiểm tra phía trước
                double checkY_wall = pos.y + frameHeight / 2.0; // Ngang hông check tường
                double checkY_ground_ahead = pos.y + frameHeight + 1; // Dưới chân phía trước check vực

                if (movingRight) {
                    checkX_ahead = pos.x + frameWidth + 1; // Phía trước bên phải
                } else {
                    checkX_ahead = pos.x - 1; // Phía trước bên trái
                }

                int tileInFrontWall = getTileAt(checkX_ahead, checkY_wall, mapData, tileWidth, tileHeight);
                int tileBelowFront = getTileAt(checkX_ahead, checkY_ground_ahead, mapData, tileWidth, tileHeight);

                // Điều kiện đổi hướng: gặp tường rắn HOẶC không có đất cứng phía trước
                bool shouldTurn = false;
                if (tileInFrontWall == TILE_UNKNOWN_SOLID) {
                    shouldTurn = true;
                } else if (tileBelowFront != TILE_GRASS && tileBelowFront != TILE_UNKNOWN_SOLID) {
                    shouldTurn = true;
                }

                if (shouldTurn) {
                    movingRight = !movingRight; // Đổi hướng
                }

                // Cập nhật vị trí X
                double moveAmount = MOVE_SPEED * dt;
                if (movingRight) {
                    pos.x += moveAmount;
                } else {
                    pos.x -= moveAmount;
                }
            } // Kết thúc AI di chuyển ngang (chỉ khi onGround)

            // 5. CẬP NHẬT ANIMATION
            animTimer += dt;
            if (animTimer >= ANIM_SPEED) {
                animTimer -= ANIM_SPEED;
                currentAnimFrameIndex = (currentAnimFrameIndex + 1) % NUM_FRAMES;
            }

             // <<< THÊM LOG Ở CUỐI UPDATE (Nếu chạm đất) >>>
            // if(landedThisFrame){
            //      cout << "[DEBUG] Enemy Landed. Final PosY=" << pos.y << endl;
            // }
            // <<< KẾT THÚC LOG Ở CUỐI UPDATE >>>


            break; // Kết thúc case ALIVE
        } // Đóng scope case ALIVE

        case EnemyState::DYING:
            velocityY = 0.0; // Ngừng rơi
            // Logic nhấp nháy
            dyingTimer += dt;
            if (dyingTimer >= DYING_DURATION) {
                currentState = EnemyState::DEAD;
                isVisible = false;
            } else {
                isVisible = (static_cast<int>(floor(dyingTimer / BLINK_INTERVAL)) % 2 == 0);
            }
            break; // Kết thúc case DYING

        case EnemyState::DEAD:
            // Không làm gì
            break; // Kết thúc case DEAD
    }
}

// Hàm checkMapCollision có thể không cần thiết nữa
/*
void Enemy::checkMapCollision(const vector<vector<int>>& mapData, int tileWidth, int tileHeight) { }
*/

// --- Hàm Render (Đã sửa để lật hình) ---
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

    // Xác định hướng lật hình
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (!movingRight) { // Nếu đi sang trái
        flip = SDL_FLIP_HORIZONTAL;
    }

    // Sử dụng SDL_RenderCopyEx để vẽ và lật hình nếu cần
    SDL_RenderCopyEx(window.getRenderer(), tex, &currentFrame, &destRect, 0.0, NULL, flip);

    /* // Debug hitbox
    if(currentState == EnemyState::ALIVE){
        SDL_SetRenderDrawColor(window.getRenderer(), 0, 0, 255, 150); // Màu xanh
        SDL_Rect debugHitbox = getWorldHitbox();
        debugHitbox.x = static_cast<int>(round(debugHitbox.x - cameraX));
        debugHitbox.y = static_cast<int>(round(debugHitbox.y - cameraY));
        SDL_RenderDrawRect(window.getRenderer(), &debugHitbox);
    }
    */
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

// --- Hàm nhận sát thương (Phát âm thanh) ---
void Enemy::takeHit() {
    if (currentState == EnemyState::ALIVE) {
        currentState = EnemyState::DYING;
        dyingTimer = 0.0;
        isVisible = true;

        // <<< PHÁT ÂM THANH CHẾT >>>
        if (gEnemyDeathSound != nullptr) {
            Mix_PlayChannel(-1, gEnemyDeathSound, 0); // Phát trên kênh tự động
        } else {
            cerr << "Warning: gEnemyDeathSound is NULL in Enemy::takeHit!" << endl;
        }
        // velocityY = -150.0; // Optional: Nảy nhẹ lên khi trúng đạn
        // isOnGround = false;
    }
}

// --- Các hàm kiểm tra trạng thái ---
bool Enemy::isAlive() const { return currentState == EnemyState::ALIVE; }
bool Enemy::isDead() const { return currentState == EnemyState::DEAD; }
EnemyState Enemy::getState() const { return currentState; }