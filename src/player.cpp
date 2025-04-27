#include "Player.hpp"
#include "RenderWindow.hpp" // Đảm bảo bạn có file này và nó định nghĩa lớp RenderWindow
#include <SDL2/SDL.h>
#include <algorithm> // For max, min
#include <cmath>     // For abs, round, floor
#include <vector>    // For vector
#include <iostream>  // For cout, endl, cerr (debugging)

// Sử dụng namespace std để tránh gõ std:: nhiều lần
using namespace std;

// --- Tile Type Constants ---
const int TILE_EMPTY = 0;
const int TILE_GRASS = 1;          // Walkable, Droppable
const int TILE_UNKNOWN_SOLID = 2; // Solid Wall/Ground
const int TILE_WATER_SURFACE = 3; // Water Surface

// --- Constructor ---
Player::Player(vector2d p_pos,
               SDL_Texture* p_runTex, int p_runSheetCols,
               SDL_Texture* p_jumpTex, int p_jumpSheetCols,
               SDL_Texture* p_enterWaterTex, int p_enterWaterSheetCols,
               SDL_Texture* p_swimTex, int p_swimSheetCols,
               SDL_Texture* p_shootHorizTex, int p_shootHorizSheetCols, // Bắn ngang khi đứng yên / trên không
               SDL_Texture* p_shootUpTex, int p_shootUpSheetCols,
               SDL_Texture* p_runShootHorizTex, int p_runShootHorizSheetCols, // <<< Texture chạy bắn
               int p_frameW, int p_frameH)
    : entity(p_pos, p_runTex, p_frameW, p_frameH, p_runSheetCols), // Gọi constructor lớp cha
      runTexture(p_runTex),
      jumpTexture(p_jumpTex),
      enterWaterTexture(p_enterWaterTex),
      swimTexture(p_swimTex),
      shootHorizTexture(p_shootHorizTex),
      shootUpTexture(p_shootUpTex),
      runShootHorizTexture(p_runShootHorizTex), // <<< Gán texture chạy bắn
      runSheetColumns(p_runSheetCols),
      jumpSheetColumns(p_jumpSheetCols),
      enterWaterSheetColumns(p_enterWaterSheetCols),
      swimSheetColumns(p_swimSheetCols),
      shootHorizSheetColumns(p_shootHorizSheetCols),
      shootUpSheetColumns(p_shootUpSheetCols),
      runShootHorizSheetColumns(p_runShootHorizSheetCols), // <<< Gán số cột chạy bắn
      frameWidth(p_frameW),
      frameHeight(p_frameH),
      velocity({0.0, 0.0}),
      currentState(PlayerState::FALLING), // Bắt đầu ở trạng thái rơi
      facing(FacingDirection::RIGHT),
      isOnGround(false),
      isInWaterState(false),
      waterSurfaceY(0.0),
      shootRequested(false), // Cờ để báo hiệu tạo đạn
      shootUpHeld(false),
      isShootingHeld(false), // <<< Khởi tạo cờ giữ bắn là false
      shootCooldownTimer(0.0),
      animTimer(0.0),
      currentAnimFrameIndex(0),
      currentMapData(nullptr),
      currentTileWidth(0),
      currentTileHeight(0)
{
    hitbox.x = 10;
    hitbox.y = 4;
    hitbox.w = 13;
    hitbox.h = 78;

    currentSourceRect = {0, 0, frameWidth, frameHeight};
}

// --- Get World Hitbox ---
SDL_Rect Player::getWorldHitbox() {
    SDL_Rect worldHB;
    worldHB.x = static_cast<int>(round(getPos().x + hitbox.x));
    worldHB.y = static_cast<int>(round(getPos().y + hitbox.y));
    worldHB.w = hitbox.w;
    worldHB.h = hitbox.h;
    return worldHB;
}

// --- Handle Input (Keys Held Down) ---
void Player::handleInput(const Uint8* keyStates) {
    shootUpHeld = keyStates[SDL_SCANCODE_UP];
    isShootingHeld = keyStates[SDL_SCANCODE_F]; // <<< CẬP NHẬT TRẠNG THÁI GIỮ PHÍM BẮN 'F'

    // --- Xử lý bắn khi giữ phím ---
    // Chỉ cho phép bắn khi không ở trong nước và cooldown đã sẵn sàng
    if (isShootingHeld && !isInWaterState && shootCooldownTimer <= 0.0) {
        // cout << "[DEBUG] handleInput: F held & Cooldown OK. Requesting shot." << endl; // Debug log
        shootRequested = true; // Đặt cờ để hàm wantsToShoot() xử lý
        shootCooldownTimer = SHOOT_COOLDOWN; // Reset cooldown để có tốc độ bắn
        // Không cần reset animation frame ở đây nữa
    }
    // --------------------------------

    // --- Xử lý di chuyển ---
    // Logic di chuyển này sẽ xác định velocity.x, nhưng trạng thái animation cuối cùng
    // (RUNNING vs RUN_SHOOTING_HORIZ) sẽ được quyết định trong updateState()
    if (isInWaterState) {
        // Di chuyển trong nước
        if (keyStates[SDL_SCANCODE_LEFT]) {
            velocity.x = -MOVE_SPEED * 0.7; facing = FacingDirection::LEFT;
        } else if (keyStates[SDL_SCANCODE_RIGHT]) {
            velocity.x = MOVE_SPEED * 0.7; facing = FacingDirection::RIGHT;
        } else {
            velocity.x *= WATER_DRAG_X; if (abs(velocity.x) < 1.0) velocity.x = 0.0;
        }
    } else {
        // Di chuyển trên cạn
        if (keyStates[SDL_SCANCODE_LEFT]) {
            velocity.x = -MOVE_SPEED; facing = FacingDirection::LEFT;
        } else if (keyStates[SDL_SCANCODE_RIGHT]) {
            velocity.x = MOVE_SPEED; facing = FacingDirection::RIGHT;
        } else {
            velocity.x = 0.0; // Đứng yên nếu không nhấn trái/phải
        }
    }
    //-------------------------
}

// --- Handle Key Down Event ---
// Chỉ xử lý các hành động nhấn 1 lần: Nhảy, Rơi qua platform
void Player::handleKeyDown(SDL_Keycode key) {
    if (isInWaterState) {
        if (key == SDLK_SPACE) {
            velocity.y = -WATER_JUMP_STRENGTH;
            // Có thể thêm state WATER_JUMP nếu cần animation riêng
        }
    } else { // Trên cạn
        // Nhảy
        if (key == SDLK_SPACE && isOnGround) {
            velocity.y = -JUMP_STRENGTH;
            isOnGround = false;
            currentState = PlayerState::JUMPING; // Chuyển state ngay lập tức
            currentAnimFrameIndex = 0; // Reset animation nhảy
        }
        // Rơi qua platform
        else if (key == SDLK_DOWN && isOnGround) {
            SDL_Rect hb = getWorldHitbox();
            double checkX = hb.x + hb.w / 2.0;
            double checkY = hb.y + hb.h + 1.0; // Kiểm tra ngay dưới chân

            if (currentMapData) {
                int groundTile = getTileAt(checkX, checkY);
                if (groundTile == TILE_GRASS) { // Chỉ rơi qua TILE_GRASS
                    isOnGround = false;
                    currentState = PlayerState::DROPPING; // Chuyển state
                    getPos().y += 2.0; // Di chuyển nhẹ xuống để chắc chắn ra khỏi platform
                    currentAnimFrameIndex = 0; // Reset animation rơi/nhảy
                    // cout << "--- Dropping through platform ---" << endl;
                }
            }
        }
        // <<< Không còn xử lý phím 'F' ở đây >>>
    }
}


// --- Hàm kiểm tra và trả thông tin tạo đạn ---
bool Player::wantsToShoot(vector2d& out_bulletStartPos, vector2d& out_bulletVelocity) {
    // Hàm này chỉ kiểm tra cờ shootRequested và reset nó
    // Cờ này được đặt trong handleInput khi giữ phím F và cooldown OK
    // cout << "[DEBUG] wantsToShoot called. Current shootRequested = " << boolalpha << shootRequested << endl; // Debug log

    if (!shootRequested) {
        return false; // Không có yêu cầu bắn nào được đặt
    }

    shootRequested = false;
    SDL_Rect hb = getWorldHitbox();
    double startX, startY, bulletVelX, bulletVelY;

    if (shootUpHeld) {
        startX = hb.x + hb.w / 2.0 - 5.0; startY = hb.y - 10.0;
        bulletVelX = 0; // Vận tốc X = 0 khi bắn lên
        bulletVelY = -BULLET_SPEED; // Vận tốc Y âm (đi lên)
    } else {
        startX = (facing == FacingDirection::RIGHT) ? (hb.x + hb.w) : (hb.x - 10.0);
        startY = hb.y + hb.h * 0.4;
        bulletVelX = (facing == FacingDirection::RIGHT) ? BULLET_SPEED : -BULLET_SPEED; // Vận tốc X dương hoặc âm
        bulletVelY = 0; // Vận tốc Y = 0 khi bắn ngang
    }
    out_bulletStartPos = {startX, startY};
    out_bulletVelocity = {bulletVelX, bulletVelY}; // <<< Gán vận tốc đã tính

    // Log vận tốc ngay trước khi trả về
    cout << "[DEBUG] Player::wantsToShoot Calculated Velocity: (" << out_bulletVelocity.x << ", " << out_bulletVelocity.y << ")" << endl; // <<< DEBUG LOG

    return true;
}


// --- Debug Print Tile Column Info ---
// Giữ nguyên hàm này nếu bạn cần dùng để debug va chạm
void Player::printTileColumnInfo() {
    if (!currentMapData || currentTileWidth <= 0 || currentTileHeight <= 0) return;
    SDL_Rect currentWorldHB = getWorldHitbox();
    double checkWorldX = static_cast<double>(currentWorldHB.x) + static_cast<double>(currentWorldHB.w) / 2.0;
    double startWorldY = static_cast<double>(currentWorldHB.y);
    int currentCol = static_cast<int>(floor(checkWorldX / currentTileWidth));
    int startRow = static_cast<int>(floor(startWorldY / currentTileHeight));
    cout << "\n--- Player Tile Column Info (Col: " << currentCol << ") ---" << endl;
    cout << "  Player World Pos: X=" << getPos().x << ", Y=" << getPos().y << endl;
    cout << "  Hitbox World Pos: X=" << currentWorldHB.x << ", Y=" << currentWorldHB.y
         << ", W=" << currentWorldHB.w << ", H=" << currentWorldHB.h << endl;
    cout << "  Player Top approx Row: " << startRow << endl;
    int mapTotalRows = currentMapData->size();
    for (int r = max(0, startRow - 2); r < mapTotalRows && r < startRow + 7; ++r) {
        int tileType = TILE_EMPTY;
        if (r >= 0 && r < currentMapData->size()) {
            const auto& rowData = (*currentMapData)[r];
            if (currentCol >= 0 && currentCol < rowData.size()) { tileType = rowData[currentCol]; }
        }
        cout << "  Row " << r << ": Type=" << tileType;
        double hbTopY = static_cast<double>(currentWorldHB.y); double hbBottomY = static_cast<double>(currentWorldHB.y) + currentWorldHB.h;
        double rowTopY = static_cast<double>(r * currentTileHeight); double rowBottomY = static_cast<double>((r + 1) * currentTileHeight);
        bool overlaps = (hbTopY < rowBottomY && hbBottomY > rowTopY);
        bool feetNear = (abs(hbBottomY - rowTopY) < 5.0 && tileType != TILE_EMPTY);
        if (overlaps) { cout << " <-- Hitbox Overlaps"; if (feetNear && velocity.y >= 0) cout << " (Feet near top)"; }
        cout << endl;
    }
    if (startRow + 7 < mapTotalRows) { cout << "  ..." << endl; }
    cout << "-----------------------------------------" << endl;
}

// --- Update Function ---
// Giữ nguyên logic tổng thể: cập nhật cooldown, vật lý, va chạm, state, animation
void Player::update(double dt, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) {
    currentMapData = &mapData;
    currentTileWidth = tileWidth;
    currentTileHeight = tileHeight;

    // Cập nhật cooldown bắn
    if (shootCooldownTimer > 0.0) {
        shootCooldownTimer -= dt;
    }

    applyGravity(dt);   // Áp dụng trọng lực
    move(dt);           // Di chuyển dựa trên vận tốc (đã được set trong handleInput)
    checkMapCollision(); // Kiểm tra và xử lý va chạm map, cập nhật isOnGround
    updateState();      // <<< Cập nhật trạng thái dựa trên isShootingHeld, isOnGround, velocity, etc.
    updateAnimation(dt); // Cập nhật frame animation dựa trên currentState

    // Giới hạn vị trí trái
    vector2d& posRef = getPos();
    posRef.x = max(0.0, posRef.x);

    // Optional: In thông tin debug
    // printTileColumnInfo();
}

// --- Apply Gravity ---
// Giữ nguyên logic
void Player::applyGravity(double dt) {
    if (isInWaterState) {
        velocity.y += GRAVITY * WATER_GRAVITY_MULTIPLIER * dt;
        velocity.y = max(-MAX_FALL_SPEED * WATER_MAX_SPEED_MULTIPLIER,
                        min(MAX_FALL_SPEED * WATER_MAX_SPEED_MULTIPLIER, velocity.y));
    } else if (!isOnGround) {
        velocity.y += GRAVITY * dt;
        velocity.y = min(velocity.y, MAX_FALL_SPEED);
    }
     // Nếu đang trên mặt đất (và không phải DROPPING), velocity.y đã được set = 0 trong checkMapCollision
}


// --- Move Function ---
// Giữ nguyên logic
void Player::move(double dt) {
    vector2d& posRef = getPos();
    posRef.x += velocity.x * dt;
    posRef.y += velocity.y * dt;
}

// --- Check Map Collision ---
// Giữ nguyên logic va chạm dọc và ngang
void Player::checkMapCollision() {
    if (!currentMapData || currentTileWidth <= 0 || currentTileHeight <=0) return;

    vector2d& posRef = getPos();
    SDL_Rect playerHB = getWorldHitbox();
    // bool onGroundBeforeCheck = isOnGround; // Không cần thiết nữa

    isOnGround = false; // Reset mỗi lần kiểm tra

    double feetWorldY = static_cast<double>(playerHB.y) + playerHB.h;
    double headWorldY = static_cast<double>(playerHB.y);
    double midWorldX = static_cast<double>(playerHB.x) + static_cast<double>(playerHB.w) / 2.0;
    double leftWorldX = static_cast<double>(playerHB.x);
    double rightWorldX = static_cast<double>(playerHB.x) + playerHB.w;
    double midWorldY = static_cast<double>(playerHB.y) + static_cast<double>(playerHB.h) / 2.0;

    // --- VERTICAL COLLISION ---
    if (velocity.y >= 0) { // Kiểm tra va chạm dưới chân khi rơi hoặc đứng yên
        int tileBelowRow = static_cast<int>(floor(feetWorldY / currentTileHeight));
        int tileBelowType = getTileAt(midWorldX, feetWorldY + 1.0); // Kiểm tra ngay dưới chân
        bool ignoreCollision = (currentState == PlayerState::DROPPING && tileBelowType == TILE_GRASS);

        if (!ignoreCollision && (tileBelowType == TILE_GRASS || tileBelowType == TILE_UNKNOWN_SOLID)) {
            double tileTopY = static_cast<double>(tileBelowRow * currentTileHeight);
            if (feetWorldY >= tileTopY) { // Chỉ xử lý nếu chân đã đi vào hoặc chạm tile
                isOnGround = true; // Đặt là đang trên đất
                if (isInWaterState) { isInWaterState = false; } // Thoát nước nếu chạm đất
                velocity.y = 0.0; // Dừng rơi
                // Điều chỉnh vị trí Y
                posRef.y = tileTopY - static_cast<double>(hitbox.h) - static_cast<double>(hitbox.y);
                // Nếu đang DROPPING mà chạm đất, updateState sẽ xử lý chuyển sang IDLE/RUNNING
            }
        } else if (tileBelowType == TILE_WATER_SURFACE && !isInWaterState) { // Va chạm mặt nước
            double waterTileTopY = static_cast<double>(tileBelowRow * currentTileHeight);
            if (midWorldY > waterTileTopY) { // Vào nước khi phần lớn cơ thể chìm
                isOnGround = false; isInWaterState = true; currentState = PlayerState::ENTERING_WATER;
                velocity.y *= 0.3; waterSurfaceY = waterTileTopY;
                posRef.y = waterSurfaceY - static_cast<double>(hitbox.h) * 0.8; // Chìm một phần
                currentAnimFrameIndex = 0;
            }
        }
        // Nếu không va chạm gì bên dưới, isOnGround vẫn là false
    } else { // Đang bay lên (velocity.y < 0)
        isOnGround = false; // Chắc chắn không trên mặt đất
    }

    // Ceiling check (va chạm trần)
    if (velocity.y < 0 && !isInWaterState) {
        int tileAboveType = getTileAt(midWorldX, headWorldY);
        if (tileAboveType == TILE_GRASS || tileAboveType == TILE_UNKNOWN_SOLID) {
            velocity.y = 0.0; // Dừng bay lên
            int tileRow = static_cast<int>(floor(headWorldY / currentTileHeight));
            posRef.y = static_cast<double>((tileRow + 1) * currentTileHeight) - static_cast<double>(hitbox.y); // Đặt ngay dưới trần
        }
    }

    // --- HORIZONTAL COLLISION --- (va chạm tường)
    if (!isInWaterState) {
        if (velocity.x > 0) { // Đi sang phải
            int tileRightType = getTileAt(rightWorldX, midWorldY);
            if (tileRightType == TILE_GRASS || tileRightType == TILE_UNKNOWN_SOLID) {
                velocity.x = 0.0; // Dừng lại
                int tileCol = static_cast<int>(floor(rightWorldX / currentTileWidth));
                posRef.x = static_cast<double>(tileCol * currentTileWidth) - static_cast<double>(hitbox.w) - static_cast<double>(hitbox.x) - 0.1; // Đặt sát tường trái
            }
        } else if (velocity.x < 0) { // Đi sang trái
            int tileLeftType = getTileAt(leftWorldX, midWorldY);
            if (tileLeftType == TILE_GRASS || tileLeftType == TILE_UNKNOWN_SOLID) {
                velocity.x = 0.0; // Dừng lại
                int tileCol = static_cast<int>(floor(leftWorldX / currentTileWidth));
                posRef.x = static_cast<double>((tileCol + 1) * currentTileWidth) - static_cast<double>(hitbox.x) + 0.1; // Đặt sát tường phải
            }
        }
    }

    // --- CHECK EXIT WATER --- (Kiểm tra thoát khỏi nước)
    if (isInWaterState) {
        int tileAtHead = getTileAt(midWorldX, headWorldY - 1.0); // Tile ngay trên đầu
        // Thoát nước nếu đầu vượt lên trên mặt nước và ô trên đầu là không khí
        if (tileAtHead == TILE_EMPTY && headWorldY < waterSurfaceY ) {
            isInWaterState = false;
            // Trọng lực sẽ xử lý việc rơi xuống nếu không nhảy tiếp
        }
        // Thoát nước khi chạm đất đã được xử lý ở va chạm dọc
    }
}


// --- Update State ---
// <<< LOGIC CẬP NHẬT TRẠNG THÁI ĐÃ SỬA ĐỂ ƯU TIÊN BẮN VÀ THÊM RUN_SHOOTING_HORIZ >>>
void Player::updateState() {
    PlayerState previousState = currentState;

    // Xử lý trạng thái dựa trên các yếu tố: trong nước, trên không, trên đất, có đang giữ bắn không

    if (isInWaterState) {
        // Ưu tiên trạng thái vào nước nếu animation chưa xong
        if (currentState == PlayerState::ENTERING_WATER) {
             if (currentAnimFrameIndex >= ENTER_WATER_FRAMES - 1) {
                 currentState = PlayerState::SWIMMING; // Chuyển sang bơi khi anim xong
             }
             // else: Giữ nguyên ENTERING_WATER
        } else {
             // Có thể phân biệt WATER_JUMP dựa vào velocity.y nếu cần
             currentState = PlayerState::SWIMMING; // Mặc định là bơi
        }
    }
    else if (!isOnGround) { // --- Đang ở trên không ---
        // Ưu tiên trạng thái bắn nếu đang giữ phím bắn
        if (isShootingHeld) {
            // Bắn lên có ưu tiên cao nhất khi ở trên không
            if (shootUpHeld) {
                currentState = PlayerState::SHOOTING_UP;
            } else {
                // Nếu không bắn lên, dùng animation bắn ngang (như đứng yên) khi ở trên không
                currentState = PlayerState::SHOOTING_HORIZ;
            }
        }
        // Nếu không giữ bắn, xác định là nhảy hay rơi
        else {
            // Kiểm tra nếu đang rơi qua platform
            if (currentState == PlayerState::DROPPING) {
                 // Giữ nguyên DROPPING cho đến khi chạm đất (checkMapCollision sẽ xử lý)
                 // Hoặc nếu vận tốc y < 0 (ví dụ nhảy lên ngay sau khi rơi) thì chuyển sang JUMPING
                 if(velocity.y < 0) currentState = PlayerState::JUMPING;
            } else {
                 // Nếu không phải DROPPING, xác định dựa trên vận tốc dọc
                 currentState = (velocity.y < 0) ? PlayerState::JUMPING : PlayerState::FALLING;
            }
        }
    }
    else { // --- Đang ở trên mặt đất ---
        // Ưu tiên trạng thái bắn nếu đang giữ phím bắn
        if (isShootingHeld) {
            // Bắn lên có ưu tiên cao nhất
            if (shootUpHeld) {
                currentState = PlayerState::SHOOTING_UP;
            }
            // Nếu không bắn lên, kiểm tra có đang chạy không
            else if (velocity.x != 0) {
                currentState = PlayerState::RUN_SHOOTING_HORIZ; // <<< Trạng thái chạy và bắn
            }
            // Nếu không bắn lên và không chạy -> đứng yên bắn
            else {
                currentState = PlayerState::SHOOTING_HORIZ;
            }
        }
        // Nếu không giữ bắn, xác định là chạy hay đứng yên
        else {
            if (velocity.x != 0) {
                currentState = PlayerState::RUNNING;
            } else {
                currentState = PlayerState::IDLE;
            }
        }
    }

    // Reset animation nếu state thay đổi
    if (currentState != previousState) {
         // cout << "[DEBUG] updateState: State Changed from " << static_cast<int>(previousState) << " to " << static_cast<int>(currentState) << ". Resetting animation." << endl; // Debug log
        animTimer = 0.0;
        currentAnimFrameIndex = 0; // Luôn reset frame index khi thay đổi state
    }
}

// --- Get Tile At ---
// Giữ nguyên logic
int Player::getTileAt(double worldX, double worldY) const {
    if (!currentMapData || worldX < 0 || worldY < 0 || currentTileWidth <= 0 || currentTileHeight <= 0) return TILE_EMPTY;
    int col = static_cast<int>(floor(worldX / currentTileWidth));
    int row = static_cast<int>(floor(worldY / currentTileHeight));
    if (row >= 0 && row < currentMapData->size()) {
        const auto& rowData = (*currentMapData)[row];
        if (col >= 0 && col < rowData.size()) { return rowData[col]; }
    }
    return TILE_EMPTY;
}

// --- Update Animation ---
// <<< THÊM CASE CHO RUN_SHOOTING_HORIZ >>>
void Player::updateAnimation(double dt) {
    animTimer += dt; // Tăng bộ đếm thời gian

    // Nếu đủ thời gian để chuyển frame
    if (animTimer >= ANIM_SPEED) {
        animTimer -= ANIM_SPEED; // Reset timer

        int sheetCols = runSheetColumns; // Mặc định
        int numFrames = 1;               // Mặc định
        bool loopAnim = true;            // Mặc định

        // Xác định thông số animation dựa trên trạng thái hiện tại
        switch(currentState) {
            case PlayerState::IDLE:
                sheetCols = runSheetColumns; numFrames = 1; currentAnimFrameIndex = 0; loopAnim = true; // IDLE chỉ có 1 frame
                break;
            case PlayerState::RUNNING:
                sheetCols = runSheetColumns; numFrames = RUN_FRAMES; loopAnim = true;
                break;
            case PlayerState::JUMPING: case PlayerState::FALLING: case PlayerState::DROPPING:
                sheetCols = jumpSheetColumns; numFrames = JUMP_FRAMES; loopAnim = false; // Nhảy/rơi không lặp
                break;
            case PlayerState::ENTERING_WATER:
                sheetCols = enterWaterSheetColumns; numFrames = ENTER_WATER_FRAMES; loopAnim = false; // Vào nước không lặp
                break;
            case PlayerState::SWIMMING: case PlayerState::WATER_JUMP:
                sheetCols = swimSheetColumns; numFrames = SWIM_FRAMES; loopAnim = true; // Bơi lặp lại
                break;
            case PlayerState::SHOOTING_HORIZ: // Bắn ngang khi đứng yên / trên không
                sheetCols = shootHorizSheetColumns; numFrames = SHOOT_HORIZ_FRAMES; loopAnim = false; // Bắn thường không lặp
                break;
            case PlayerState::SHOOTING_UP:
                sheetCols = shootUpSheetColumns; numFrames = SHOOT_UP_FRAMES; loopAnim = false; // Bắn lên không lặp
                break;
            case PlayerState::RUN_SHOOTING_HORIZ: // <<< CASE MỚI CHO CHẠY BẮN
                sheetCols = runShootHorizSheetColumns; numFrames = RUN_SHOOT_HORIZ_FRAMES; loopAnim = true; // Chạy bắn lặp lại
                break;
        }

        // Cập nhật frame index (chỉ khi state không phải IDLE)
        if (currentState != PlayerState::IDLE) {
            if (loopAnim) {
                // Animation lặp lại: tăng frame và quay về 0 nếu vượt quá
                currentAnimFrameIndex = (currentAnimFrameIndex + 1) % numFrames;
            } else {
                // Animation không lặp: chỉ tăng nếu chưa phải frame cuối
                if (currentAnimFrameIndex < numFrames - 1) {
                    currentAnimFrameIndex++;
                }
                // Nếu đã là frame cuối thì giữ nguyên frame đó
            }
        }
        // else: IDLE luôn giữ frame 0 đã được set trong switch-case

        // Tính toán tọa độ sourceRect dựa trên frame index mới
        currentSourceRect.x = (currentAnimFrameIndex % sheetCols) * frameWidth;
        currentSourceRect.y = (currentAnimFrameIndex / sheetCols) * frameHeight;
        currentSourceRect.w = frameWidth; // Giữ nguyên kích thước frame
        currentSourceRect.h = frameHeight;
    }
     // Nếu không đủ thời gian chuyển frame, không làm gì cả, giữ nguyên frame cũ và sourceRect
}


// --- Render ---
// <<< THÊM CASE ĐỂ CHỌN TEXTURE RUN_SHOOTING_HORIZ >>>
void Player::render(RenderWindow& window, double cameraX, double cameraY) {
    SDL_Texture* textureToRender = runTexture; // Texture mặc định

    // Chọn texture phù hợp với trạng thái hiện tại
    switch(currentState) {
        // Các state cũ giữ nguyên
        case PlayerState::JUMPING: case PlayerState::FALLING: case PlayerState::DROPPING: textureToRender = jumpTexture; break;
        case PlayerState::ENTERING_WATER: textureToRender = enterWaterTexture; break;
        case PlayerState::SWIMMING: case PlayerState::WATER_JUMP: textureToRender = swimTexture; break;
        case PlayerState::SHOOTING_HORIZ: textureToRender = shootHorizTexture; break; // Bắn đứng yên / trên không
        case PlayerState::SHOOTING_UP: textureToRender = shootUpTexture; break;

        // State mới
        case PlayerState::RUN_SHOOTING_HORIZ: textureToRender = runShootHorizTexture; break; // <<< Chọn texture chạy bắn

        // State mặc định
        case PlayerState::IDLE: case PlayerState::RUNNING: default: textureToRender = runTexture; break;
    }

    // Kiểm tra texture null
    if (!textureToRender) {
        cerr << "Error: Texture is NULL for state " << static_cast<int>(currentState) << endl;
        // Có thể vẽ hình báo lỗi thay vì return
        return;
    }

    // Tính toán destination rectangle (vị trí vẽ trên màn hình)
    SDL_Rect destRect = {
        static_cast<int>(round(getPos().x - cameraX)),
        static_cast<int>(round(getPos().y - cameraY)),
        currentSourceRect.w, // Kích thước vẽ lấy từ source rect
        currentSourceRect.h
    };

    // Xác định hướng flip (lật hình)
    // Không lật hình khi bắn lên
    SDL_RendererFlip flip = (facing == FacingDirection::LEFT && currentState != PlayerState::SHOOTING_UP)
                              ? SDL_FLIP_HORIZONTAL
                              : SDL_FLIP_NONE;

    // Vẽ sprite lên màn hình
    SDL_RenderCopyEx(window.getRenderer(), textureToRender, &currentSourceRect, &destRect, 0.0, NULL, flip);

    /* // Optional: Vẽ hitbox để debug
    SDL_SetRenderDrawColor(window.getRenderer(), 255, 0, 0, 150); // Màu đỏ
    SDL_Rect debugHitbox = getWorldHitbox();
    debugHitbox.x = static_cast<int>(round(debugHitbox.x - cameraX));
    debugHitbox.y = static_cast<int>(round(debugHitbox.y - cameraY));
    SDL_RenderDrawRect(window.getRenderer(), &debugHitbox);
    */
}