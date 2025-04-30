#include "Player.hpp"
#include "RenderWindow.hpp" // Cần include RenderWindow.hpp để vẽ
#include <SDL2/SDL.h>
#include <algorithm> // For max, min
#include <cmath>     // For abs, round, floor
#include <vector>    // For vector
#include <iostream>  // For cout, endl, cerr (debugging)
#include <set>       // Cần include set ở đây nữa nếu chưa có trong hpp
#include <utility>   // Cần include utility ở đây nữa nếu chưa có trong hpp

// Sử dụng namespace std để tránh gõ std:: nhiều lần
using namespace std;

// --- Tile Type Constants (Định nghĩa lại ở đây để dễ tham khảo) ---
const int TILE_EMPTY = 0;
const int TILE_GRASS = 1;          // Đi được, Lộn xuống được, Nhảy xuyên qua từ dưới
const int TILE_UNKNOWN_SOLID = 2; // Tường/Đất rắn, không thể đi/nhảy xuyên
const int TILE_WATER_SURFACE = 3; // Mặt nước

// --- Constructor ---
Player::Player(vector2d p_pos,
               SDL_Texture* p_runTex, int p_runSheetCols,
               SDL_Texture* p_jumpTex, int p_jumpSheetCols,
               SDL_Texture* p_enterWaterTex, int p_enterWaterSheetCols,
               SDL_Texture* p_swimTex, int p_swimSheetCols,
               SDL_Texture* p_shootHorizTex, int p_shootHorizSheetCols,
               SDL_Texture* p_shootUpTex, int p_shootUpSheetCols,
               SDL_Texture* p_runShootHorizTex, int p_runShootHorizSheetCols,
               int p_frameW, int p_frameH)
    : entity(p_pos, p_runTex, p_frameW, p_frameH, p_runSheetCols), // Gọi constructor lớp cha
      runTexture(p_runTex),
      jumpTexture(p_jumpTex),
      enterWaterTexture(p_enterWaterTex),
      swimTexture(p_swimTex),
      shootHorizTexture(p_shootHorizTex),
      shootUpTexture(p_shootUpTex),
      runShootHorizTexture(p_runShootHorizTex),
      runSheetColumns(p_runSheetCols),
      jumpSheetColumns(p_jumpSheetCols),
      enterWaterSheetColumns(p_enterWaterSheetCols),
      swimSheetColumns(p_swimSheetCols),
      shootHorizSheetColumns(p_shootHorizSheetCols),
      shootUpSheetColumns(p_shootUpSheetCols),
      runShootHorizSheetColumns(p_runShootHorizSheetCols),
      frameWidth(p_frameW),
      frameHeight(p_frameH),
      velocity({0.0, 0.0}),
      currentState(PlayerState::FALLING), // Bắt đầu ở trạng thái rơi
      facing(FacingDirection::RIGHT),
      isOnGround(false),
      isInWaterState(false),
      waterSurfaceY(0.0),
      shootRequested(false),
      shootUpHeld(false),
      isShootingHeld(false),
      shootCooldownTimer(0.0),
      animTimer(0.0),
      currentAnimFrameIndex(0),
      currentMapData(nullptr),
      currentMapRows(0),
      currentMapCols(0),
      currentTileWidth(0),
      currentTileHeight(0)
      // temporarilyDisabledTiles được khởi tạo rỗng mặc định
{
    // Hitbox gốc của bạn
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
    // Kiểm tra giữ phím bắn và hướng lên
    shootUpHeld = keyStates[SDL_SCANCODE_UP];
    isShootingHeld = keyStates[SDL_SCANCODE_F];

    // Yêu cầu bắn nếu giữ phím F, không ở trong nước và cooldown đã hết
    if (isShootingHeld && !isInWaterState && shootCooldownTimer <= 0.0) {
        shootRequested = true; // Đặt cờ yêu cầu bắn
        shootCooldownTimer = SHOOT_COOLDOWN; // Reset cooldown
    }

    // Xử lý di chuyển ngang
    if (isInWaterState) { // Di chuyển trong nước chậm hơn và có lực cản
        if (keyStates[SDL_SCANCODE_LEFT]) { velocity.x = -MOVE_SPEED * 0.7; facing = FacingDirection::LEFT; }
        else if (keyStates[SDL_SCANCODE_RIGHT]) { velocity.x = MOVE_SPEED * 0.7; facing = FacingDirection::RIGHT; }
        else { velocity.x *= WATER_DRAG_X; if (abs(velocity.x) < 1.0) velocity.x = 0.0; } // Lực cản
    } else { // Di chuyển trên cạn
        if (keyStates[SDL_SCANCODE_LEFT]) { velocity.x = -MOVE_SPEED; facing = FacingDirection::LEFT; }
        else if (keyStates[SDL_SCANCODE_RIGHT]) { velocity.x = MOVE_SPEED; facing = FacingDirection::RIGHT; }
        else { velocity.x = 0.0; } // Đứng yên nếu không nhấn trái/phải
    }
}

// --- Handle Key Down Event (Nhấn phím một lần) ---
void Player::handleKeyDown(SDL_Keycode key) {
    // Xử lý trong nước
    if (isInWaterState) {
        if (key == SDLK_SPACE) { // Nhảy trong nước
            velocity.y = -WATER_JUMP_STRENGTH;
            currentState = PlayerState::WATER_JUMP;
            currentAnimFrameIndex = 0;
        }
        // Không cho lộn xuống khi đang bơi
    }
    // Xử lý trên cạn/không khí
    else {
        // Nhảy (chỉ khi đang trên mặt đất)
        if (key == SDLK_SPACE && isOnGround) {
            velocity.y = -JUMP_STRENGTH;
            isOnGround = false;
            currentState = PlayerState::JUMPING;
            currentAnimFrameIndex = 0;
        }
        // Lộn xuống (nhấn XUỐNG khi đang trên mặt đất)
        else if (key == SDLK_DOWN && isOnGround) {
            SDL_Rect hb = getWorldHitbox();
            double checkX = hb.x + hb.w / 2.0;
            // Xác định Row, Col của tile ngay dưới chân một chút
            double checkY_below = hb.y + hb.h + 1.0;
            int groundRow = static_cast<int>(floor(checkY_below / currentTileHeight));
            int groundCol = static_cast<int>(floor(checkX / currentTileWidth));

            // Kiểm tra tile đó có hợp lệ và là GRASS (1) không
            if (groundRow >= 0 && groundRow < currentMapRows && groundCol >= 0 && groundCol < currentMapCols) {
                // Đảm bảo có dòng dữ liệu mapData[groundRow] tồn tại
                if (groundCol < (*currentMapData)[groundRow].size()) {
                    int groundTileType = (*currentMapData)[groundRow][groundCol];
                    if (groundTileType == TILE_GRASS) {
                        // <<< Vô hiệu hóa tile cỏ đang đứng >>>
                        temporarilyDisabledTiles.insert({groundRow, groundCol});
                        // cout << "[DEBUG] Disabled tile (" << groundRow << ", " << groundCol << ")" << endl;

                        isOnGround = false; // Không còn đứng trên đất nữa
                        currentState = PlayerState::DROPPING; // Chuyển sang trạng thái rơi/lộn xuống
                        getPos().y += 3.0; // Di chuyển nhẹ xuống để chắc chắn rời khỏi platform
                        currentAnimFrameIndex = 0; // Reset animation (sẽ dùng anim nhảy/rơi)
                    }
                }
            }
        }
        // Không xử lý bắn 'F' ở đây nữa
    }
}

// --- Wants To Shoot (Kiểm tra cờ yêu cầu bắn và trả thông tin) ---
bool Player::wantsToShoot(vector2d& out_bulletStartPos, vector2d& out_bulletVelocity) {
    if (!shootRequested) {
        return false; // Không có yêu cầu bắn
    }

    shootRequested = false; // Reset cờ
    SDL_Rect hb = getWorldHitbox();
    double startX, startY, bulletVelX, bulletVelY;

    if (shootUpHeld) { // Bắn lên
        startX = hb.x + hb.w / 2.0 - 5.0; // Điều chỉnh vị trí đạn bắn lên
        startY = hb.y - 10.0;
        bulletVelX = 0;
        bulletVelY = -BULLET_SPEED;
    } else { // Bắn ngang
        startX = (facing == FacingDirection::RIGHT) ? (hb.x + hb.w) : (hb.x - 10.0); // Điều chỉnh vị trí đạn bắn ngang
        startY = hb.y + hb.h * 0.4; // Vị trí Y của nòng súng (ví dụ)
        bulletVelX = (facing == FacingDirection::RIGHT) ? BULLET_SPEED : -BULLET_SPEED;
        bulletVelY = 0;
    }
    out_bulletStartPos = {startX, startY};
    out_bulletVelocity = {bulletVelX, bulletVelY};
    return true;
}

// --- Debug Print Tile Column Info (Tùy chọn giữ lại) ---
void Player::printTileColumnInfo() {
   // In thông tin debug về tile và vị trí người chơi
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
       int tileType = TILE_EMPTY; // Mặc định nếu ngoài biên
       if (r >= 0 && r < currentMapData->size()) {
           // Kiểm tra biên cột trước khi truy cập
           if (currentCol >= 0 && currentCol < (*currentMapData)[r].size()) {
               tileType = (*currentMapData)[r][currentCol];
            }
       }
       cout << "  Row " << r << ": Type=" << tileType;
       // Kiểm tra xem tile này có đang bị vô hiệu hóa không
       if (temporarilyDisabledTiles.count({r, currentCol}) > 0) {
            cout << " (DISABLED)";
       }
       double hbTopY = static_cast<double>(currentWorldHB.y);
       double hbBottomY = static_cast<double>(currentWorldHB.y) + currentWorldHB.h;
       double rowTopY = static_cast<double>(r * currentTileHeight);
       double rowBottomY = static_cast<double>((r + 1) * currentTileHeight);
       bool overlaps = (hbTopY < rowBottomY && hbBottomY > rowTopY);
       bool feetNear = (abs(hbBottomY - rowTopY) < 5.0 && tileType != TILE_EMPTY && tileType != TILE_WATER_SURFACE); // Chỉ feet near với đất liền
       if (overlaps) { cout << " <-- HB Overlaps"; }
       if (feetNear && velocity.y >= 0) { cout << " (Feet near ground)"; }
       cout << endl;
   }
   if (startRow + 7 < mapTotalRows) { cout << "  ..." << endl; }
   cout << "  State: " << static_cast<int>(currentState) << ", isOnGround: " << boolalpha << isOnGround << ", isInWater: " << isInWaterState << endl;
   cout << "  Velocity: (" << velocity.x << ", " << velocity.y << ")" << endl;
   cout << "-----------------------------------------" << endl;
}


// --- Update Function (Thứ tự gọi hàm giữ nguyên như lần sửa trước) ---
void Player::update(double dt, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) {
    // Lưu thông tin map
    currentMapData = &mapData;
    currentTileWidth = tileWidth;
    currentTileHeight = tileHeight;
    currentMapRows = mapData.size();
    if (currentMapRows > 0) currentMapCols = mapData[0].size(); else currentMapCols = 0;

    // Cập nhật cooldown
    if (shootCooldownTimer > 0.0) { shootCooldownTimer -= dt; }

    // Cập nhật vật lý
    applyGravity(dt);
    move(dt);

    // Kiểm tra va chạm VÀ cập nhật trạng thái vật lý (isOnGround, isInWater)
    checkMapCollision();

    // Cập nhật trạng thái logic (IDLE, RUNNING, JUMPING, DROPPING...) dựa trên trạng thái vật lý
    updateState(); // <<< Gọi trước restore

    // Cập nhật frame animation dựa trên trạng thái logic hiện tại
    updateAnimation(dt);

    // Khôi phục lại các tile cỏ đã đi qua (sau khi va chạm và state đã được xử lý)
    restoreDisabledTiles(); // <<< Gọi sau cùng

    // Optional: In debug info
    // printTileColumnInfo();
}


// --- Apply Gravity ---
void Player::applyGravity(double dt) {
    if (isInWaterState) { // Trong nước, trọng lực yếu hơn
        velocity.y += GRAVITY * WATER_GRAVITY_MULTIPLIER * dt;
        velocity.y = max(-MAX_FALL_SPEED * WATER_MAX_SPEED_MULTIPLIER, // Giới hạn tốc độ lên/xuống trong nước
                        min(MAX_FALL_SPEED * WATER_MAX_SPEED_MULTIPLIER, velocity.y));
    } else if (!isOnGround) { // Trên không, trọng lực bình thường
        velocity.y += GRAVITY * dt;
        velocity.y = min(velocity.y, MAX_FALL_SPEED); // Giới hạn tốc độ rơi tối đa
    }
    // Nếu isOnGround = true, velocity.y sẽ được đặt lại = 0 trong checkMapCollision
}

// --- Move Function ---
void Player::move(double dt) {
    vector2d& posRef = getPos();
    posRef.x += velocity.x * dt;
    posRef.y += velocity.y * dt;
}


// --- Check Map Collision (VIẾT LẠI VỚI ƯU TIÊN XỬ LÝ ĐÁY) ---
void Player::checkMapCollision() {
    if (!currentMapData || currentTileWidth <= 0 || currentTileHeight <= 0 || currentMapRows <= 0) return;

    vector2d& posRef = getPos();
    SDL_Rect playerHB = getWorldHitbox();

    double feetWorldY = static_cast<double>(playerHB.y) + playerHB.h;
    double headWorldY = static_cast<double>(playerHB.y);
    double midWorldX = static_cast<double>(playerHB.x) + static_cast<double>(playerHB.w) / 2.0;
    double leftWorldX = static_cast<double>(playerHB.x);
    double rightWorldX = static_cast<double>(playerHB.x) + playerHB.w;
    double midWorldY = static_cast<double>(playerHB.y) + static_cast<double>(playerHB.h) / 2.0;

    // --- 1. KIỂM TRA VÀ XỬ LÝ CHẠM ĐÁY (ROW 6) TRƯỚC TIÊN ---
    double bottomBoundaryY = static_cast<double>((BOTTOM_ROW_INDEX + 1) * currentTileHeight);
    if (feetWorldY >= bottomBoundaryY) {
        // Lấy loại tile ở đáy
        int bottomTileCol = static_cast<int>(floor(midWorldX / currentTileWidth));
        int bottomTileType = TILE_EMPTY;
        if (BOTTOM_ROW_INDEX >= 0 && BOTTOM_ROW_INDEX < currentMapRows && bottomTileCol >= 0 && bottomTileCol < (*currentMapData)[BOTTOM_ROW_INDEX].size()) {
            bottomTileType = (*currentMapData)[BOTTOM_ROW_INDEX][bottomTileCol];
        }

        // Kẹp vị trí Y
        posRef.y = bottomBoundaryY - static_cast<double>(hitbox.h) - static_cast<double>(hitbox.y);
        feetWorldY = bottomBoundaryY; // Cập nhật Y chân
        velocity.y = 0; // Dừng rơi

        // Xử lý trạng thái tại đáy
        if (bottomTileType == TILE_WATER_SURFACE) {
             if (!isInWaterState) { // Nếu chưa ở trong nước -> vào nước
                 isOnGround = false;
                 isInWaterState = true;
                 // Đặt state ENTERING_WATER ở đây để anim vào nước chạy
                 currentState = PlayerState::ENTERING_WATER;
                 waterSurfaceY = static_cast<double>(BOTTOM_ROW_INDEX * currentTileHeight);
                 currentAnimFrameIndex = 0;
                 // cout << "[DEBUG] Entered Water AT BOTTOM." << endl;
             } else { // Đã ở trong nước rồi, chạm đáy nước -> vẫn bơi
                 isOnGround = false; // Không đứng trên đất
                 // cout << "[DEBUG] Hit water bottom while already swimming." << endl;
             }
        } else { // Đáy là rắn hoặc rỗng
            isOnGround = true; // Đứng trên đáy
            if (isInWaterState) { // Nếu đang bơi mà chạm đáy rắn -> thoát nước
                isInWaterState = false;
                // cout << "[DEBUG] Exited water hitting solid bottom." << endl;
            } else {
                 // cout << "[DEBUG] Hit solid/empty bottom." << endl;
            }
            // Đã xử lý va chạm dọc với đáy rắn -> không cần kiểm tra sàn/nước thông thường nữa
            goto HorizontalCollisionCheck; // Nhảy tới kiểm tra ngang
        }
        // Nếu đã xử lý vào nước ở đáy, không cần goto, để kiểm tra ngang chạy
        // Nếu chạm đáy nước khi đã ở trong nước, cũng không cần goto
    }
     // --- Nếu không chạm đáy ---
     else {
         // Reset trạng thái nền tảng trước khi kiểm tra va chạm thông thường
         isOnGround = false;
         // isInWaterState giữ nguyên giá trị từ frame trước

         // 2. KIỂM TRA VA CHẠM DỌC THÔNG THƯỜNG (TRẦN, SÀN/NƯỚC KHÁC)
         // 2.1 Va chạm trần (Khi bay lên và không trong nước)
         if (velocity.y < 0 && !isInWaterState) {
             int tileAboveRow = static_cast<int>(floor(headWorldY / currentTileHeight));
             int tileAboveCol = static_cast<int>(floor(midWorldX / currentTileWidth));
             if (tileAboveRow >= 0 && tileAboveRow < currentMapRows && tileAboveCol >= 0 && tileAboveCol < currentMapCols) {
                if (tileAboveCol < (*currentMapData)[tileAboveRow].size()) { // Kiểm tra biên cột
                    int tileAboveType = (*currentMapData)[tileAboveRow][tileAboveCol];
                    if (tileAboveType == TILE_UNKNOWN_SOLID) { // Chỉ chặn bởi SOLID(2)
                        double ceilingY = static_cast<double>((tileAboveRow + 1) * currentTileHeight);
                        if (headWorldY <= ceilingY) {
                            velocity.y = 50.0; // Đẩy nhẹ xuống
                            posRef.y = ceilingY - static_cast<double>(hitbox.y);
                        }
                    }
                }
             }
         }
         // 2.2 Va chạm sàn/nước thông thường (Khi rơi/đứng yên và chưa vào nước)
         else if (velocity.y >= 0 && !isInWaterState) {
             int playerFeetRow = static_cast<int>(floor(feetWorldY / currentTileHeight));
             // Row đã được đảm bảo < BOTTOM_ROW_INDEX do kiểm tra chạm đáy ở trên

             int playerMidCol = static_cast<int>(floor(midWorldX / currentTileWidth));
             int targetRow = playerFeetRow;
             int landingTileType = TILE_EMPTY;
             double targetTileTopY = 0.0;
             bool foundLandingSpot = false;
             bool tileIsDisabled = false;

             // Kiểm tra ô đích
             if (targetRow >= 0 && targetRow < currentMapRows && playerMidCol >= 0 && playerMidCol < currentMapCols) {
                 if (playerMidCol < (*currentMapData)[targetRow].size()) { // Kiểm tra biên cột
                    landingTileType = (*currentMapData)[targetRow][playerMidCol];
                    targetTileTopY = static_cast<double>(targetRow * currentTileHeight);
                    tileIsDisabled = temporarilyDisabledTiles.count({targetRow, playerMidCol}) > 0;

                    if (!tileIsDisabled && (landingTileType == TILE_GRASS || landingTileType == TILE_UNKNOWN_SOLID || landingTileType == TILE_WATER_SURFACE)) {
                        foundLandingSpot = true;
                    }
                 }
             }

             // Xử lý chạm điểm đáp
             if (foundLandingSpot && feetWorldY >= targetTileTopY) {
                  // A. Hạ cánh trên đất
                  if (landingTileType == TILE_GRASS || landingTileType == TILE_UNKNOWN_SOLID) {
                     isOnGround = true; // Đặt trạng thái đứng trên đất
                     velocity.y = 0;
                     posRef.y = targetTileTopY - static_cast<double>(hitbox.h) - static_cast<double>(hitbox.y);
                      // cout << "[DEBUG] Landed on Ground (Normal). Row: " << targetRow << endl;
                  }
                  // B. Vào nước (không phải ở đáy)
                  else if (landingTileType == TILE_WATER_SURFACE) {
                      if (midWorldY > targetTileTopY) { // Đủ sâu mới vào
                          isOnGround = false;
                          isInWaterState = true; // Đặt trạng thái trong nước
                          currentState = PlayerState::ENTERING_WATER;
                          velocity.y *= 0.3; // Giảm tốc rơi
                          waterSurfaceY = targetTileTopY;
                          posRef.y = waterSurfaceY - static_cast<double>(hitbox.h) * 0.8; // Chìm 1 phần
                          currentAnimFrameIndex = 0;
                          // cout << "[DEBUG] Entered Water (Normal). Row: " << targetRow << endl;
                      }
                      // else: chạm nhẹ, không làm gì, isOnGround vẫn là false
                  }
             }
             // Nếu không foundLandingSpot hoặc chưa chạm, isOnGround vẫn là false
         }
          // Nếu đang ở trong nước và không chạm đáy, không cần kiểm tra va chạm sàn/nước khác
         else if (isInWaterState) {
              isOnGround = false; // Đảm bảo không đứng trên đất khi đang bơi (trừ khi chạm đáy rắn)
         }
     } // Kết thúc else (không chạm đáy)


// --- 3. KIỂM TRA VA CHẠM NGANG ---
HorizontalCollisionCheck: // Nhãn goto từ xử lý đáy rắn
    if (!isInWaterState) { // Chỉ kiểm tra va chạm tường khi không ở trong nước
        playerHB = getWorldHitbox(); // Lấy lại hitbox vì Y có thể đã thay đổi
        leftWorldX = static_cast<double>(playerHB.x);
        rightWorldX = static_cast<double>(playerHB.x) + playerHB.w;
        midWorldY = static_cast<double>(playerHB.y) + static_cast<double>(playerHB.h) / 2.0;

        if (velocity.x > 0) { // Đi sang phải
            int row = static_cast<int>(floor(midWorldY / currentTileHeight));
            int col = static_cast<int>(floor(rightWorldX / currentTileWidth));
            if (row >= 0 && row < currentMapRows && col >= 0 && col < currentMapCols) {
                if (col < (*currentMapData)[row].size()) { // Kiểm tra biên cột
                    if ((*currentMapData)[row][col] == TILE_UNKNOWN_SOLID) { // Chỉ chặn bởi SOLID (2)
                        double wallX = static_cast<double>(col * currentTileWidth);
                        if (rightWorldX >= wallX) { velocity.x = 0.0; posRef.x = wallX - hitbox.w - hitbox.x - 0.1; }
                    }
                }
            }
        } else if (velocity.x < 0) { // Đi sang trái
             int row = static_cast<int>(floor(midWorldY / currentTileHeight));
             int col = static_cast<int>(floor(leftWorldX / currentTileWidth));
             if (row >= 0 && row < currentMapRows && col >= 0 && col < currentMapCols) {
                 if (col < (*currentMapData)[row].size()) { // Kiểm tra biên cột
                    if ((*currentMapData)[row][col] == TILE_UNKNOWN_SOLID) { // Chỉ chặn bởi SOLID (2)
                        double wallX = static_cast<double>((col + 1) * currentTileWidth);
                        if (leftWorldX <= wallX) { velocity.x = 0.0; posRef.x = wallX - hitbox.x + 0.1; }
                    }
                 }
             }
        }
    } // Kết thúc kiểm tra va chạm ngang

// --- 4. KIỂM TRA THOÁT NƯỚC LÊN TRÊN ---
    if (isInWaterState) {
        headWorldY = static_cast<double>(getWorldHitbox().y);
        midWorldX = static_cast<double>(getWorldHitbox().x) + static_cast<double>(getWorldHitbox().w) / 2.0;

        int tileAboveRow = static_cast<int>(floor((headWorldY - 1.0) / currentTileHeight));
        int tileAboveCol = static_cast<int>(floor(midWorldX / currentTileWidth));

        if (tileAboveRow >= 0 && tileAboveRow < currentMapRows && tileAboveCol >= 0 && tileAboveCol < currentMapCols) {
             if (tileAboveCol < (*currentMapData)[tileAboveRow].size()) { // Kiểm tra biên cột
                 int tileAtHead = (*currentMapData)[tileAboveRow][tileAboveCol];
                 // Thoát nước nếu đầu trên mặt nước VÀ ô trên đầu là không khí
                 if (headWorldY < waterSurfaceY && tileAtHead == TILE_EMPTY) {
                     isInWaterState = false;
                     // cout << "[DEBUG] Exited water upwards." << endl;
                 }
             }
        }
        // Thoát nước khi chạm đáy rắn đã xử lý ở phần 1
    }
}


// --- Restore Disabled Tiles ---
void Player::restoreDisabledTiles() {
    if (temporarilyDisabledTiles.empty()) return;
    SDL_Rect playerHB = getWorldHitbox();
    double feetWorldY = static_cast<double>(playerHB.y + playerHB.h);
    for (auto it = temporarilyDisabledTiles.begin(); it != temporarilyDisabledTiles.end(); ) {
        int disabledRow = it->first;
        double tileBottomY = static_cast<double>((disabledRow + 1) * currentTileHeight);
        if (feetWorldY >= tileBottomY + 1.0) { // Khôi phục khi chân đã qua hẳn
            it = temporarilyDisabledTiles.erase(it);
        } else {
            ++it;
        }
    }
}


// --- Update State ---
void Player::updateState() {
    PlayerState previousState = currentState;

    if (isInWaterState) { // Đang trong nước
        // Nếu đang vào nước, chờ animation xong hoặc bị đẩy lên
        if (currentState == PlayerState::ENTERING_WATER) {
            if (velocity.y < -10.0) { currentState = PlayerState::WATER_JUMP; }
            // Nếu anim xong, updateAnimation sẽ chuyển sang SWIMMING
        } else { // Đã ở trong nước
            currentState = (velocity.y < -10.0) ? PlayerState::WATER_JUMP : PlayerState::SWIMMING;
        }
    } else { // Không ở trong nước
        if (!isOnGround) { // Đang trên không
            if (currentState == PlayerState::DROPPING) {
                if (velocity.y < 0) { currentState = PlayerState::JUMPING; }
                 // Khi chạm đất, isOnGround=true, code dưới xử lý
            } else if (isShootingHeld) {
                currentState = shootUpHeld ? PlayerState::SHOOTING_UP : PlayerState::SHOOTING_HORIZ;
            } else {
                currentState = (velocity.y < 0) ? PlayerState::JUMPING : PlayerState::FALLING;
            }
        } else { // Đang trên mặt đất (isOnGround = true)
             // Nếu vừa mới chạm đất từ trên không
            if (previousState == PlayerState::FALLING || previousState == PlayerState::JUMPING || previousState == PlayerState::DROPPING) {
                 currentState = (abs(velocity.x) > 0.1) ? PlayerState::RUNNING : PlayerState::IDLE;
            }
            // Nếu đã ở trên đất từ trước
            else if (isShootingHeld) {
                if (shootUpHeld) { currentState = PlayerState::SHOOTING_UP; }
                else if (abs(velocity.x) > 0.1) { currentState = PlayerState::RUN_SHOOTING_HORIZ; }
                else { currentState = PlayerState::SHOOTING_HORIZ; }
            } else { // Không bắn
                if (abs(velocity.x) > 0.1 && currentState != PlayerState::RUNNING) {
                    currentState = PlayerState::RUNNING;
                } else if (abs(velocity.x) <= 0.1 && currentState != PlayerState::IDLE) {
                    currentState = PlayerState::IDLE;
                }
            }
        }
    }

    // Reset animation nếu state thay đổi
    if (currentState != previousState) {
        bool shouldReset = true;
        // Ngoại lệ: Không reset ngay khi anim vào nước chưa xong
        if (previousState == PlayerState::ENTERING_WATER && currentState == PlayerState::SWIMMING) {
             shouldReset = false;
        }
         // Ngoại lệ: Không reset ngay khi vừa chạm đất
        if ((previousState == PlayerState::FALLING || previousState == PlayerState::JUMPING || previousState == PlayerState::DROPPING) && isOnGround) {
             shouldReset = false;
        }

        if(shouldReset){
             animTimer = 0.0;
             currentAnimFrameIndex = 0;
        }
    }
}


// --- Get Tile At ---
int Player::getTileAt(double worldX, double worldY) const {
    if (!currentMapData || worldX < 0 || worldY < 0 || currentTileWidth <= 0 || currentTileHeight <= 0) return TILE_EMPTY;
    int col = static_cast<int>(floor(worldX / currentTileWidth));
    int row = static_cast<int>(floor(worldY / currentTileHeight));
    if (row >= 0 && row < currentMapRows) {
        // Kiểm tra biên cột trước khi truy cập
        if (col >= 0 && col < (*currentMapData)[row].size()) {
            return (*currentMapData)[row][col];
        }
    }
    return TILE_EMPTY;
}

// --- Update Animation ---
void Player::updateAnimation(double dt) {
    animTimer += dt;
    if (animTimer >= ANIM_SPEED) {
        animTimer -= ANIM_SPEED;
        int sheetCols = 1, numFrames = 1;
        bool loopAnim = true;
        SDL_Texture* currentTexture = runTexture;

        // Xác định thông số dựa trên state hiện tại
        switch(currentState) {
            case PlayerState::IDLE: sheetCols = runSheetColumns; numFrames = 1; currentAnimFrameIndex = 0; loopAnim = true; currentTexture = runTexture; break;
            case PlayerState::RUNNING: sheetCols = runSheetColumns; numFrames = RUN_FRAMES; loopAnim = true; currentTexture = runTexture; break;
            case PlayerState::JUMPING: case PlayerState::FALLING: case PlayerState::DROPPING: sheetCols = jumpSheetColumns; numFrames = JUMP_FRAMES; loopAnim = false; currentTexture = jumpTexture; break;
            case PlayerState::ENTERING_WATER: sheetCols = enterWaterSheetColumns; numFrames = ENTER_WATER_FRAMES; loopAnim = false; currentTexture = enterWaterTexture; break;
            case PlayerState::SWIMMING: case PlayerState::WATER_JUMP: sheetCols = swimSheetColumns; numFrames = SWIM_FRAMES; loopAnim = true; currentTexture = swimTexture; break;
            case PlayerState::SHOOTING_HORIZ: sheetCols = shootHorizSheetColumns; numFrames = SHOOT_HORIZ_FRAMES; loopAnim = false; currentTexture = shootHorizTexture; break;
            case PlayerState::SHOOTING_UP: sheetCols = shootUpSheetColumns; numFrames = SHOOT_UP_FRAMES; loopAnim = false; currentTexture = shootUpTexture; break;
            case PlayerState::RUN_SHOOTING_HORIZ: sheetCols = runShootHorizSheetColumns; numFrames = RUN_SHOOT_HORIZ_FRAMES; loopAnim = true; currentTexture = runShootHorizTexture; break;
        }

        // Cập nhật frame index
        if (currentState != PlayerState::IDLE) {
            if (loopAnim) {
                currentAnimFrameIndex = (currentAnimFrameIndex + 1) % numFrames;
            } else { // Animation không lặp
                if (currentAnimFrameIndex < numFrames - 1) {
                    currentAnimFrameIndex++;
                }
                // Tự động chuyển sang SWIMMING khi ENTERING_WATER xong
                else if (currentState == PlayerState::ENTERING_WATER) {
                    // Chỉ chuyển nếu state vẫn là ENTERING_WATER (tránh ghi đè nếu đã nhảy ra)
                    if (currentState == PlayerState::ENTERING_WATER) {
                        currentState = PlayerState::SWIMMING;
                        // cout << "[DEBUG] Animation finished, switching to SWIMMING" << endl;
                        // Animation sẽ tự đổi sang SWIMMING ở lần update tiếp theo
                        // Không cần reset frame ở đây vì SWIMMING là anim lặp
                    }
                }
                // Giữ nguyên frame cuối cho các anim không lặp khác
            }
        }

        // Tính toán sourceRect
        if (currentTexture) {
            int texW = 0; SDL_QueryTexture(currentTexture, NULL, NULL, &texW, NULL);
            int actualSheetCols = (frameWidth > 0 && texW > 0) ? (texW / frameWidth) : sheetCols;
            if (actualSheetCols <= 0) actualSheetCols = 1;
            currentSourceRect.x = (currentAnimFrameIndex % actualSheetCols) * frameWidth;
            currentSourceRect.y = (numFrames > actualSheetCols && actualSheetCols > 0) ? ((currentAnimFrameIndex / actualSheetCols) * frameHeight) : 0;
            currentSourceRect.w = frameWidth; currentSourceRect.h = frameHeight;
        } else { currentSourceRect = {0, 0, frameWidth, frameHeight}; }
    }
}


// --- Render ---
void Player::render(RenderWindow& window, double cameraX, double cameraY) {
    SDL_Texture* textureToRender = runTexture;
    switch(currentState) {
        case PlayerState::JUMPING: case PlayerState::FALLING: case PlayerState::DROPPING: textureToRender = jumpTexture; break;
        case PlayerState::ENTERING_WATER: textureToRender = enterWaterTexture; break;
        case PlayerState::SWIMMING: case PlayerState::WATER_JUMP: textureToRender = swimTexture; break;
        case PlayerState::SHOOTING_HORIZ: textureToRender = shootHorizTexture; break;
        case PlayerState::SHOOTING_UP: textureToRender = shootUpTexture; break;
        case PlayerState::RUN_SHOOTING_HORIZ: textureToRender = runShootHorizTexture; break;
        default: textureToRender = runTexture; break;
    }

    if (!textureToRender) {
        cerr << "Error: Texture is NULL for state " << static_cast<int>(currentState) << endl;
        SDL_Rect errorRect = { static_cast<int>(round(getPos().x - cameraX)), static_cast<int>(round(getPos().y - cameraY)), frameWidth, frameHeight };
        SDL_SetRenderDrawColor(window.getRenderer(), 255, 0, 0, 255);
        SDL_RenderFillRect(window.getRenderer(), &errorRect);
        return;
    }

    SDL_Rect destRect = {
        static_cast<int>(round(getPos().x - cameraX)),
        static_cast<int>(round(getPos().y - cameraY)),
        currentSourceRect.w, currentSourceRect.h };

    SDL_RendererFlip flip = (facing == FacingDirection::LEFT && currentState != PlayerState::SHOOTING_UP) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(window.getRenderer(), textureToRender, &currentSourceRect, &destRect, 0.0, NULL, flip);

    /* // Debug Hitbox
    SDL_Renderer* renderer = window.getRenderer();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
    SDL_Rect debugHitbox = getWorldHitbox();
    debugHitbox.x = static_cast<int>(round(debugHitbox.x - cameraX));
    debugHitbox.y = static_cast<int>(round(debugHitbox.y - cameraY));
    SDL_RenderFillRect(renderer, &debugHitbox);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    */
}