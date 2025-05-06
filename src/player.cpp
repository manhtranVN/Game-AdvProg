#include "Player.hpp"
#include "RenderWindow.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <set>
#include <utility>

using namespace std;

const int TILE_EMPTY = 0; const int TILE_GRASS = 1;
const int TILE_UNKNOWN_SOLID = 2; const int TILE_WATER_SURFACE = 3;

// --- Constructor (Đảm bảo khớp với .hpp) ---
Player::Player(vector2d p_pos,
           SDL_Texture* p_runTex, int p_runSheetCols,
           SDL_Texture* p_jumpTex, int p_jumpSheetCols,
           SDL_Texture* p_enterWaterTex, int p_enterWaterSheetCols,
           SDL_Texture* p_swimTex, int p_swimSheetCols,
           SDL_Texture* p_aimUpTex, int p_aimUpSheetCols,
           SDL_Texture* p_aimDiagUpTex, int p_aimDiagUpSheetCols,
           SDL_Texture* p_aimDiagDownTex, int p_aimDiagDownSheetCols,
           SDL_Texture* p_runAimDiagUpTex, int p_runAimDiagUpSheetCols,
           SDL_Texture* p_runAimDiagDownTex, int p_runAimDiagDownSheetCols,
           SDL_Texture* p_standShootHorizTex, int p_standShootHorizSheetCols,
           SDL_Texture* p_runShootHorizTex, int p_runShootHorizSheetCols,
           SDL_Texture* p_shootUpTex, int p_shootUpSheetCols,
           SDL_Texture* p_shootDiagUpTex, int p_shootDiagUpSheetCols,
           SDL_Texture* p_shootDiagDownTex, int p_shootDiagDownSheetCols,
           SDL_Texture* p_lyingDownTex, int p_lyingDownSheetCols,
           SDL_Texture* p_lyingShootTex, int p_lyingShootSheetCols,
           int p_standardFrameW, int p_standardFrameH,
           int p_lyingFrameW, int p_lyingFrameH)
    : entity(p_pos, p_runTex, p_standardFrameW, p_standardFrameH, p_runSheetCols), // entity dùng kích thước chuẩn
      // Khởi tạo textures
      runTexture(p_runTex), jumpTexture(p_jumpTex), enterWaterTexture(p_enterWaterTex), swimTexture(p_swimTex),
      aimUpTexture(p_aimUpTex), aimDiagUpTexture(p_aimDiagUpTex), aimDiagDownTexture(p_aimDiagDownTex),
      runAimDiagUpTexture(p_runAimDiagUpTex), runAimDiagDownTexture(p_runAimDiagDownTex),
      standShootHorizTexture(p_standShootHorizTex), runShootHorizTexture(p_runShootHorizTex),
      shootUpTexture(p_shootUpTex), shootDiagUpTexture(p_shootDiagUpTex), shootDiagDownTexture(p_shootDiagDownTex),
      lyingDownTexture(p_lyingDownTex), lyingShootTexture(p_lyingShootTex),
      // Khởi tạo số cột sheet
      runSheetColumns(p_runSheetCols), jumpSheetColumns(p_jumpSheetCols), enterWaterSheetColumns(p_enterWaterSheetCols), swimSheetColumns(p_swimSheetCols),
      aimUpSheetColumns(p_aimUpSheetCols), aimDiagUpSheetColumns(p_aimDiagUpSheetCols), aimDiagDownSheetColumns(p_aimDiagDownSheetCols),
      runAimDiagUpSheetColumns(p_runAimDiagUpSheetCols), runAimDiagDownSheetColumns(p_runAimDiagDownSheetCols),
      standShootHorizSheetColumns(p_standShootHorizSheetCols), runShootHorizSheetColumns(p_runShootHorizSheetCols),
      shootUpSheetColumns(p_shootUpSheetCols), shootDiagUpSheetColumns(p_shootDiagUpSheetCols), shootDiagDownSheetColumns(p_shootDiagDownSheetCols),
      lyingDownSheetColumns(p_lyingDownSheetCols), lyingShootSheetColumns(p_lyingShootSheetCols),
      // Khởi tạo kích thước frame
      standardFrameWidth(p_standardFrameW), standardFrameHeight(p_standardFrameH),
      lyingFrameWidth(p_lyingFrameW), lyingFrameHeight(p_lyingFrameH),
      // Khởi tạo trạng thái vật lý và logic
      velocity({0.0, 0.0}), currentState(PlayerState::FALLING), facing(FacingDirection::RIGHT),
      isOnGround(false), isInWaterState(false), waterSurfaceY(0.0),
      // Khởi tạo các cờ input
      shootRequested(false), aimUpHeld(false), aimDownHeld(false), aimStraightUpHeld(false), isShootingHeld(false),
      isLyingDownState(false), wantsToLieDown(false), wantsToStandUp(false),
      // Khởi tạo timers và animation
      shootCooldownTimer(0.0), animTimer(0.0), currentAnimFrameIndex(0),
      // Khởi tạo thông tin map
      currentMapData(nullptr), currentMapRows(0), currentMapCols(0),
      currentTileWidth(0), currentTileHeight(0)
{
    // Hitbox ban đầu là hitbox khi đứng
    hitbox = {10, 4, 13, 78}; // Ví dụ (x, y, w, h tương đối)
    originalStandingHitboxDef = hitbox;
    // SourceRect ban đầu
    currentSourceRect = {0, 0, standardFrameWidth, standardFrameHeight};
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
    aimUpHeld = keyStates[SDL_SCANCODE_UP];
    aimDownHeld = keyStates[SDL_SCANCODE_DOWN];
    aimStraightUpHeld = keyStates[SDL_SCANCODE_E];
    isShootingHeld = keyStates[SDL_SCANCODE_F];

    if (isShootingHeld && shootCooldownTimer <= 0.0) {
        shootRequested = true;
        shootCooldownTimer = SHOOT_COOLDOWN;
    }

    // Xác định vận tốc ngang cơ bản
    if (isInWaterState) {
        if (keyStates[SDL_SCANCODE_LEFT]) { velocity.x = -MOVE_SPEED * 0.7; facing = FacingDirection::LEFT; }
        else if (keyStates[SDL_SCANCODE_RIGHT]) { velocity.x = MOVE_SPEED * 0.7; facing = FacingDirection::RIGHT; }
        else { velocity.x *= WATER_DRAG_X; if (abs(velocity.x) < 1.0) velocity.x = 0.0; }
    } else {
        if (keyStates[SDL_SCANCODE_LEFT]) { velocity.x = -MOVE_SPEED; facing = FacingDirection::LEFT; }
        else if (keyStates[SDL_SCANCODE_RIGHT]) { velocity.x = MOVE_SPEED; facing = FacingDirection::RIGHT; }
        else { velocity.x = 0.0; }
    }

    // Áp dụng chặn di chuyển (sẽ được gọi lại sau updateState)
    // applyStateBasedMovementRestrictions(); // Không gọi ở đây nữa
}

// --- Handle Key Down Event ---
void Player::handleKeyDown(SDL_Keycode key) {
    if (isInWaterState) {
        if (key == SDLK_SPACE) {
             velocity.y = -WATER_JUMP_STRENGTH;
             // currentState = PlayerState::WATER_JUMP; // Để updateState xử lý
             currentAnimFrameIndex = 0;
        }
        return;
    }

    if (key == SDLK_SPACE && isOnGround && !isLyingDownState) {
        velocity.y = -JUMP_STRENGTH;
        isOnGround = false;
        // currentState = PlayerState::JUMPING; // Để updateState xử lý
        currentAnimFrameIndex = 0;
    }
    else if (key == SDLK_d && isOnGround && !isLyingDownState) { // Drop bằng 'D'
        SDL_Rect hb_check = getWorldHitbox();
        double checkX = hb_check.x + hb_check.w / 2.0;
        double checkY_below = hb_check.y + hb_check.h + 1.0;
        int groundRow = static_cast<int>(floor(checkY_below / currentTileHeight));
        int groundCol = static_cast<int>(floor(checkX / currentTileWidth));
        if (groundRow >= 0 && groundRow < currentMapRows && groundCol >= 0 && groundCol < currentMapCols) {
             if (groundCol < (*currentMapData)[groundRow].size()) {
                 int groundTileType = (*currentMapData)[groundRow][groundCol];
                 if (groundTileType == TILE_GRASS) {
                     temporarilyDisabledTiles.insert({groundRow, groundCol});
                     isOnGround = false;
                     // currentState = PlayerState::DROPPING; // Để updateState xử lý
                     currentAnimFrameIndex = 0;
                 }
             }
        }
    }
    else if (key == SDLK_c && isOnGround && !isInWaterState) { // Toggle nằm bằng 'C'
        if (!isLyingDownState) {
            wantsToLieDown = true; wantsToStandUp = false;
        } else {
            wantsToStandUp = true; wantsToLieDown = false;
        }
    }
}

// --- Wants To Shoot ---
bool Player::wantsToShoot(vector2d& out_bulletStartPos, vector2d& out_bulletVelocity) {
    if (!shootRequested) { return false; }
    shootRequested = false;
    SDL_Rect hb = getWorldHitbox();
    double startX = 0, startY = 0, bulletVelX = 0, bulletVelY = 0;
    PlayerState shootingDirectionState = determineShootingState();

    switch (shootingDirectionState) {
        case PlayerState::LYING_SHOOTING:
            startX = (facing == FacingDirection::RIGHT) ? (hb.x + hb.w) : (hb.x - 10.0);
            startY = hb.y + hb.h * 0.5;
            bulletVelX = (facing == FacingDirection::RIGHT) ? BULLET_SPEED : -BULLET_SPEED; bulletVelY = 0;
            break;
        case PlayerState::SHOOTING_UP:
            startX = hb.x + hb.w / 2.0 - 5.0; startY = hb.y - 10.0;
            bulletVelX = 0; bulletVelY = -BULLET_SPEED;
            break;
        case PlayerState::SHOOTING_DIAG_UP:
            startX = (facing == FacingDirection::RIGHT) ? (hb.x + hb.w * 0.8) : (hb.x + hb.w * 0.2 - 10.0); startY = hb.y;
            bulletVelX = (facing == FacingDirection::RIGHT) ? BULLET_SPEED_DIAG_COMPONENT : -BULLET_SPEED_DIAG_COMPONENT; bulletVelY = -BULLET_SPEED_DIAG_COMPONENT;
            break;
        case PlayerState::SHOOTING_DIAG_DOWN:
            startX = (facing == FacingDirection::RIGHT) ? (hb.x + hb.w) : (hb.x - 10.0); startY = hb.y + hb.h * 0.8;
            bulletVelX = (facing == FacingDirection::RIGHT) ? BULLET_SPEED_DIAG_COMPONENT : -BULLET_SPEED_DIAG_COMPONENT; bulletVelY = BULLET_SPEED_DIAG_COMPONENT;
            break;
        case PlayerState::SHOOTING_HORIZ: case PlayerState::RUN_SHOOTING_HORIZ: default:
            startX = (facing == FacingDirection::RIGHT) ? (hb.x + hb.w) : (hb.x - 10.0); startY = hb.y + hb.h * 0.4;
            bulletVelX = (facing == FacingDirection::RIGHT) ? BULLET_SPEED : -BULLET_SPEED; bulletVelY = 0;
            break;
    }
    out_bulletStartPos = {startX, startY}; out_bulletVelocity = {bulletVelX, bulletVelY};
    return true;
}

// --- Determine Shooting State ---
PlayerState Player::determineShootingState() const {
    if (isLyingDownState) return PlayerState::LYING_SHOOTING;
    // Ưu tiên các trạng thái aiming hiện tại
    if (currentState == PlayerState::AIMING_UP || currentState == PlayerState::SHOOTING_UP) return PlayerState::SHOOTING_UP;
    if (currentState == PlayerState::AIMING_DIAG_UP || currentState == PlayerState::RUN_AIMING_DIAG_UP || currentState == PlayerState::SHOOTING_DIAG_UP) return PlayerState::SHOOTING_DIAG_UP;
    if (currentState == PlayerState::AIMING_DIAG_DOWN || currentState == PlayerState::RUN_AIMING_DIAG_DOWN || currentState == PlayerState::SHOOTING_DIAG_DOWN) return PlayerState::SHOOTING_DIAG_DOWN;
    // Nếu không aim đặc biệt, xác định dựa trên di chuyển
    bool isMoving = abs(velocity.x) > 0.1 && isOnGround;
    return isMoving ? PlayerState::RUN_SHOOTING_HORIZ : PlayerState::SHOOTING_HORIZ;
}

// --- Update Function ---
void Player::update(double dt, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) {
    currentMapData = &mapData; currentTileWidth = tileWidth; currentTileHeight = tileHeight;
    currentMapRows = mapData.size(); if (currentMapRows > 0) currentMapCols = mapData[0].size(); else currentMapCols = 0;
    if (shootCooldownTimer > 0.0) { shootCooldownTimer -= dt; }

    // --- Xử lý chuyển đổi trạng thái Nằm/Đứng ---
    if (wantsToLieDown && isOnGround && !isLyingDownState && !isInWaterState) {
        isLyingDownState = true; wantsToLieDown = false;
        SDL_Rect worldHBBefore = getWorldHitbox();
        // Đặt hitbox khi nằm (ví dụ, cần tinh chỉnh theo sprite 78x40)
        hitbox.w = lyingFrameWidth - 18; hitbox.h = lyingFrameHeight - 10;
        hitbox.x = 9; hitbox.y = standardFrameHeight - lyingFrameHeight + 5;
        getPos().y = (worldHBBefore.y + worldHBBefore.h) - hitbox.y - hitbox.h;
        velocity.x = 0;
        currentState = PlayerState::LYING_DOWN;
        currentAnimFrameIndex = 0; animTimer = 0.0;
        cout << "[State] Entered Lying Down." << endl;
    } else if (wantsToStandUp && isLyingDownState) {
        isLyingDownState = false; wantsToStandUp = false;
        SDL_Rect worldHBBefore = getWorldHitbox();
        hitbox = originalStandingHitboxDef;
        getPos().y = (worldHBBefore.y + worldHBBefore.h) - hitbox.y - hitbox.h;
        currentState = PlayerState::IDLE;
        currentAnimFrameIndex = 0; animTimer = 0.0;
        cout << "[State] Stood Up." << endl;
    }
    // Reset cờ yêu cầu nếu điều kiện không đúng
    if (!isOnGround || isInWaterState) {
        wantsToLieDown = false;
        if(isLyingDownState) { // Tự động đứng dậy nếu rơi/vào nước khi nằm
            isLyingDownState = false; wantsToStandUp = false;
            hitbox = originalStandingHitboxDef;
            // currentState = PlayerState::FALLING; // Sẽ được đặt bởi updateState
            cout << "[State] Force stand up due to falling/entering water." << endl;
        }
    }
    wantsToLieDown = false; // Reset cờ yêu cầu sau khi kiểm tra
    wantsToStandUp = false;
    // --- Kết thúc xử lý Nằm/Đứng ---

    applyGravity(dt);
    move(dt);
    checkMapCollision();
    updateState(); // Cập nhật currentState
    applyStateBasedMovementRestrictions(); // Chặn di chuyển dựa trên state MỚI
    updateAnimation(dt);
    restoreDisabledTiles();
}

// --- Apply Gravity ---
void Player::applyGravity(double dt) {
    if (isLyingDownState && isOnGround) { velocity.y = 0; return; }
    if (isInWaterState) {
        velocity.y += GRAVITY * WATER_GRAVITY_MULTIPLIER * dt;
        double maxWaterSpeed = MAX_FALL_SPEED * WATER_MAX_SPEED_MULTIPLIER;
        velocity.y = max(-maxWaterSpeed, min(velocity.y, maxWaterSpeed));
    } else if (!isOnGround) {
        velocity.y += GRAVITY * dt;
        velocity.y = min(velocity.y, MAX_FALL_SPEED);
    }
}

// --- Move Function ---
void Player::move(double dt) {
    vector2d& posRef = getPos();
    posRef.x += velocity.x * dt;
    posRef.y += velocity.y * dt;
}

// --- Apply State Based Movement Restrictions ---
void Player::applyStateBasedMovementRestrictions() {
    bool block = false;
    if (isLyingDownState) { block = true; }
    else {
        switch (currentState) {
            // Các state đứng yên khi aim/bắn lên
            case PlayerState::AIMING_UP:
            case PlayerState::SHOOTING_UP:
            case PlayerState::AIMING_DIAG_UP: // Đứng yên aim chéo lên
            case PlayerState::SHOOTING_DIAG_UP:
                block = true;
                break;
            default:
                break; // Các state khác không tự động chặn ở đây
        }
        // Thêm chặn nếu giữ phím E (aim thẳng lên)
        if (aimStraightUpHeld && !isInWaterState) {
             block = true;
        }
    }
    if (block) { velocity.x = 0.0; }
}

// --- Check Map Collision ---
void Player::checkMapCollision() {
    if (!currentMapData || currentTileWidth <= 0 || currentTileHeight <= 0 || currentMapRows <= 0) return;
    vector2d& posRef = getPos(); SDL_Rect playerHB = getWorldHitbox();
    double feetY=playerHB.y+playerHB.h, headY=playerHB.y, midX=playerHB.x+playerHB.w/2.0;
    double leftX=playerHB.x, rightX=playerHB.x+playerHB.w, midY=playerHB.y+playerHB.h/2.0;
    bool onG=false, inW=isInWaterState; double finY=posRef.y; bool vCol=false;

    if(velocity.y<0 && !inW){ /* Check trần */
        int r=floor(headY/currentTileHeight), c=floor(midX/currentTileWidth);
        if(r>=0 && r<currentMapRows && c>=0 && c<currentMapCols && c<(*currentMapData)[r].size()){
            if((*currentMapData)[r][c]==TILE_UNKNOWN_SOLID){
                double ceilY=(r+1)*currentTileHeight; if(headY<=ceilY){finY=ceilY-hitbox.y; velocity.y=50; vCol=true;}
            }
        }
    }
    if(!vCol && velocity.y>=0){ /* Check sàn/nước */
        double chkY=feetY+((velocity.y==0 && !isLyingDownState)||(isLyingDownState&&velocity.y==0)?0.1:0.0);
        int r=floor(chkY/currentTileHeight), c=floor(midX/currentTileWidth);
        r=max(0,min(r,currentMapRows-1)); c=max(0,min(c,currentMapCols-1));
        int tType=TILE_EMPTY; bool tDis=false;
        if(r<currentMapRows && c<currentMapCols && c<(*currentMapData)[r].size()){
            tType=(*currentMapData)[r][c]; tDis=temporarilyDisabledTiles.count({r,c})>0;
        }
        double tileTop=r*currentTileHeight;
        if(feetY>=tileTop){
            if(tType==TILE_GRASS || tType==TILE_UNKNOWN_SOLID){
                if(!(currentState==PlayerState::DROPPING && tDis)){
                    onG=true; inW=false; finY=tileTop-hitbox.h-hitbox.y;
                    if(!isLyingDownState || velocity.y>0) velocity.y=0; vCol=true;
                } else { onG=false; }
            } else if(tType==TILE_WATER_SURFACE){
                onG=false; if(!inW){ if(midY>tileTop){inW=true; velocity.y*=0.3; waterSurfaceY=tileTop; finY=waterSurfaceY-hitbox.h*0.8-hitbox.y; currentAnimFrameIndex=0; vCol=true;} else {inW=false;}}
                else {inW=true; double absBot=currentMapRows*currentTileHeight; if(feetY>=absBot){finY=absBot-hitbox.h-hitbox.y; velocity.y=max(0.0,velocity.y); vCol=true;}}
            } else { onG=false; }
        } else { onG=false; }
        double mapBot=currentMapRows*currentTileHeight;
        if(!vCol && feetY>=mapBot){onG=true; inW=false; finY=mapBot-hitbox.h-hitbox.y; velocity.y=0; vCol=true;}
    }
    if(vCol) posRef.y=finY;
    isOnGround=onG; isInWaterState=inW;
    if(isLyingDownState && isOnGround) velocity.y=0;

    playerHB = getWorldHitbox(); /* Check ngang */
    leftX=playerHB.x; rightX=playerHB.x+playerHB.w; midY=playerHB.y+playerHB.h/2.0;
    int midR=floor(midY/currentTileHeight); midR=max(0,min(midR,currentMapRows-1));
    if(velocity.x>0){
        int c=floor(rightX/currentTileWidth); c=max(0,min(c,currentMapCols-1));
        if(c<(*currentMapData)[midR].size() && (*currentMapData)[midR][c]==TILE_UNKNOWN_SOLID){
            double wallX=c*currentTileWidth; if(rightX>=wallX){velocity.x=0; posRef.x=wallX-hitbox.w-hitbox.x-0.1;}
        }
    } else if(velocity.x<0){
        int c=floor(leftX/currentTileWidth); c=max(0,min(c,currentMapCols-1));
        if(c<(*currentMapData)[midR].size() && (*currentMapData)[midR][c]==TILE_UNKNOWN_SOLID){
            double wallX=(c+1)*currentTileWidth; if(leftX<=wallX){velocity.x=0; posRef.x=wallX-hitbox.x+0.1;}
        }
    }
    if(isInWaterState && velocity.y<0){ /* Thoát nước */
        playerHB=getWorldHitbox(); headY=playerHB.y; midX=playerHB.x+playerHB.w/2.0;
        int r=floor((headY-1.0)/currentTileHeight); int c=floor(midX/currentTileWidth);
        r=max(0,r); c=max(0,min(c,currentMapCols-1));
        if(r<currentMapRows && c<currentMapCols && c<(*currentMapData)[r].size()){
            if(headY<waterSurfaceY && (*currentMapData)[r][c]==TILE_EMPTY) {isInWaterState=false;}
        }
    }
}

// --- Restore Disabled Tiles ---
void Player::restoreDisabledTiles() {
    if (temporarilyDisabledTiles.empty()) return;
    SDL_Rect playerHB = getWorldHitbox(); double feetY=playerHB.y+playerHB.h; double eps=0.1;
    for (auto it=temporarilyDisabledTiles.begin(); it!=temporarilyDisabledTiles.end();){
        double tileBot=(it->first+1)*currentTileHeight;
        if(feetY>=tileBot+eps){it=temporarilyDisabledTiles.erase(it);} else{++it;}
    }
}

// --- Update State ---
void Player::updateState() {
    PlayerState previousState = currentState;
    PlayerState nextState = currentState;

    if (isInWaterState) {
        if (previousState == PlayerState::ENTERING_WATER) {
            if (currentAnimFrameIndex >= ENTER_WATER_FRAMES - 1) { nextState = PlayerState::SWIMMING; }
            else if (velocity.y < -1.0) { nextState = PlayerState::WATER_JUMP; }
            else { nextState = PlayerState::ENTERING_WATER; }
        } else {
            nextState = (velocity.y < -1.0) ? PlayerState::WATER_JUMP : PlayerState::SWIMMING;
        }
    }
    else if (isLyingDownState) {
        nextState = isShootingHeld ? PlayerState::LYING_SHOOTING : PlayerState::LYING_DOWN;
    }
    else if (!isOnGround) { // Trên không
        if (isShootingHeld) {
            if (aimStraightUpHeld || aimUpHeld) { nextState = PlayerState::SHOOTING_DIAG_UP; }
            else if (aimDownHeld) { nextState = PlayerState::SHOOTING_DIAG_DOWN; }
            else { nextState = PlayerState::SHOOTING_HORIZ; }
        } else {
            if (velocity.y < -1.0) { nextState = PlayerState::JUMPING; }
            else if (velocity.y > 1.0) { nextState = (previousState == PlayerState::DROPPING && !isOnGround) ? PlayerState::DROPPING : PlayerState::FALLING; }
            else { nextState = (previousState == PlayerState::JUMPING) ? PlayerState::JUMPING : PlayerState::FALLING; } // Giữ JUMPING ở đỉnh
        }
    }
    else { // Trên mặt đất
        bool isMoving = abs(velocity.x) > 0.1;
        if (isShootingHeld) {
            if (aimStraightUpHeld) { nextState = PlayerState::SHOOTING_UP; }
            else if (aimUpHeld) { nextState = PlayerState::SHOOTING_DIAG_UP; } // Đứng yên bắn chéo lên
            else if (aimDownHeld) { nextState = PlayerState::SHOOTING_DIAG_DOWN;} // Bắn chéo xuống (đứng/chạy dùng chung state & anim)
            else { nextState = isMoving ? PlayerState::RUN_SHOOTING_HORIZ : PlayerState::SHOOTING_HORIZ; }
        } else { // Không bắn
            if (aimStraightUpHeld) { nextState = PlayerState::AIMING_UP; }
            else if (aimUpHeld) { nextState = isMoving ? PlayerState::RUN_AIMING_DIAG_UP : PlayerState::AIMING_DIAG_UP; }
            else if (aimDownHeld) { nextState = isMoving ? PlayerState::RUN_AIMING_DIAG_DOWN : PlayerState::AIMING_DIAG_DOWN; }
            else { nextState = isMoving ? PlayerState::RUNNING : PlayerState::IDLE; }
        }
    }

    if (nextState != previousState) {
        currentState = nextState;
        animTimer = 0.0; currentAnimFrameIndex = 0;
        // cout << "[State Change] -> " << static_cast<int>(currentState) << endl;
    }
}

// --- Get Tile At ---
int Player::getTileAt(double worldX, double worldY) const {
    if (!currentMapData||worldX<0||worldY<0||currentTileWidth<=0||currentTileHeight<=0) return TILE_EMPTY;
    int c=floor(worldX/currentTileWidth), r=floor(worldY/currentTileHeight);
    if(r>=0 && r<currentMapRows && c>=0 && c<(*currentMapData)[r].size()) return (*currentMapData)[r][c];
    return TILE_EMPTY;
}

// --- Update Animation ---
void Player::updateAnimation(double dt) {
    animTimer += dt;
    if (animTimer >= ANIM_SPEED) {
        animTimer -= ANIM_SPEED;
        int sheetCols = 1; int numFrames = 1; bool loopAnim = true;
        SDL_Texture* currentTexture = runTexture;
        int currentFrameW = standardFrameWidth; int currentFrameH = standardFrameHeight;

        switch(currentState) {
            case PlayerState::IDLE: sheetCols=runSheetColumns; numFrames=1; loopAnim=false; currentTexture=runTexture; currentAnimFrameIndex=0; break;
            case PlayerState::RUNNING: sheetCols=runSheetColumns; numFrames=RUN_FRAMES; loopAnim=true; currentTexture=runTexture; break;
            case PlayerState::JUMPING: case PlayerState::FALLING: case PlayerState::DROPPING: sheetCols=jumpSheetColumns; numFrames=JUMP_FRAMES; loopAnim=false; currentTexture=jumpTexture; break;
            case PlayerState::ENTERING_WATER: sheetCols=enterWaterSheetColumns; numFrames=ENTER_WATER_FRAMES; loopAnim=false; currentTexture=enterWaterTexture; break;
            case PlayerState::SWIMMING: case PlayerState::WATER_JUMP: sheetCols=swimSheetColumns; numFrames=SWIM_FRAMES; loopAnim=true; currentTexture=swimTexture; break;
            case PlayerState::AIMING_UP: sheetCols=aimUpSheetColumns; numFrames=AIM_UP_FRAMES; loopAnim=false; currentTexture=aimUpTexture; break;
            case PlayerState::AIMING_DIAG_UP: sheetCols=aimDiagUpSheetColumns; numFrames=AIM_DIAG_UP_FRAMES; loopAnim=false; currentTexture=aimDiagUpTexture; break;
            case PlayerState::AIMING_DIAG_DOWN: sheetCols=aimDiagDownSheetColumns; numFrames=AIM_DIAG_DOWN_FRAMES; loopAnim=false; currentTexture=aimDiagDownTexture; break;
            case PlayerState::RUN_AIMING_DIAG_UP: sheetCols=runAimDiagUpSheetColumns; numFrames=RUN_AIM_DIAG_UP_FRAMES; loopAnim=true; currentTexture=runAimDiagUpTexture; break;
            case PlayerState::RUN_AIMING_DIAG_DOWN: sheetCols=runAimDiagDownSheetColumns; numFrames=RUN_AIM_DIAG_DOWN_FRAMES; loopAnim=true; currentTexture=runAimDiagDownTexture; break;
            case PlayerState::SHOOTING_HORIZ: sheetCols=standShootHorizSheetColumns; numFrames=STAND_SHOOT_HORIZ_FRAMES; loopAnim=false; currentTexture=standShootHorizTexture; break;
            case PlayerState::RUN_SHOOTING_HORIZ: sheetCols=runShootHorizSheetColumns; numFrames=RUN_SHOOT_HORIZ_FRAMES; loopAnim=true; currentTexture=runShootHorizTexture; break;
            case PlayerState::SHOOTING_UP: sheetCols=shootUpSheetColumns; numFrames=SHOOT_UP_FRAMES; loopAnim=false; currentTexture=shootUpTexture; break;
            case PlayerState::SHOOTING_DIAG_UP: sheetCols=shootDiagUpSheetColumns; numFrames=SHOOT_DIAG_UP_FRAMES; loopAnim=false; currentTexture=shootDiagUpTexture; break;
            case PlayerState::SHOOTING_DIAG_DOWN: sheetCols=shootDiagDownSheetColumns; numFrames=SHOOT_DIAG_DOWN_FRAMES; loopAnim=false; currentTexture=shootDiagDownTexture; break;
            case PlayerState::LYING_DOWN: sheetCols=lyingDownSheetColumns; numFrames=LYING_DOWN_FRAMES; loopAnim=false; currentTexture=lyingDownTexture; currentFrameW=lyingFrameWidth; currentFrameH=lyingFrameHeight; break;
            case PlayerState::LYING_SHOOTING: sheetCols=lyingShootSheetColumns; numFrames=LYING_SHOOT_FRAMES; loopAnim=isShootingHeld; currentTexture=lyingShootTexture; currentFrameW=lyingFrameWidth; currentFrameH=lyingFrameHeight; break;
            default: cerr << "!!!! [ANIM] Unknown Player State: " << static_cast<int>(currentState) << endl; sheetCols=runSheetColumns; numFrames=1; currentAnimFrameIndex=0; loopAnim=false; currentTexture=runTexture; break;
        }

        if (!loopAnim) { if (currentAnimFrameIndex < numFrames - 1) { currentAnimFrameIndex++; } else { currentAnimFrameIndex = numFrames - 1; } }
        else { currentAnimFrameIndex = (currentAnimFrameIndex + 1) % numFrames; }

        if (currentTexture) {
            int texW_query = 0; SDL_QueryTexture(currentTexture, NULL, NULL, &texW_query, NULL);
            int actualSheetCols = (currentFrameW > 0 && texW_query > 0) ? (texW_query / currentFrameW) : sheetCols;
            if (actualSheetCols <= 0) actualSheetCols = 1;
            currentSourceRect.x = (currentAnimFrameIndex % actualSheetCols) * currentFrameW;
            currentSourceRect.y = (numFrames > actualSheetCols && actualSheetCols > 0) ? ((currentAnimFrameIndex / actualSheetCols) * currentFrameH) : 0;
            currentSourceRect.w = currentFrameW; currentSourceRect.h = currentFrameH;
        } else { currentSourceRect = {0, 0, standardFrameWidth, standardFrameHeight}; }
    }
}

// --- Render ---
void Player::render(RenderWindow& window, double cameraX, double cameraY) {
    SDL_Texture* textureToRender = nullptr;
    switch(currentState) {
        case PlayerState::IDLE: textureToRender = runTexture; break;
        case PlayerState::RUNNING: textureToRender = runTexture; break;
        case PlayerState::JUMPING: case PlayerState::FALLING: case PlayerState::DROPPING: textureToRender = jumpTexture; break;
        case PlayerState::ENTERING_WATER: textureToRender = enterWaterTexture; break;
        case PlayerState::SWIMMING: case PlayerState::WATER_JUMP: textureToRender = swimTexture; break;
        case PlayerState::AIMING_UP: textureToRender = aimUpTexture; break;
        case PlayerState::AIMING_DIAG_UP: textureToRender = aimDiagUpTexture; break;
        case PlayerState::AIMING_DIAG_DOWN: textureToRender = aimDiagDownTexture; break;
        case PlayerState::RUN_AIMING_DIAG_UP: textureToRender = runAimDiagUpTexture; break;
        case PlayerState::RUN_AIMING_DIAG_DOWN: textureToRender = runAimDiagDownTexture; break;
        case PlayerState::SHOOTING_HORIZ: textureToRender = standShootHorizTexture; break;
        case PlayerState::RUN_SHOOTING_HORIZ: textureToRender = runShootHorizTexture; break;
        case PlayerState::SHOOTING_UP: textureToRender = shootUpTexture; break;
        case PlayerState::SHOOTING_DIAG_UP: textureToRender = shootDiagUpTexture; break;
        case PlayerState::SHOOTING_DIAG_DOWN: textureToRender = shootDiagDownTexture; break;
        case PlayerState::LYING_DOWN: textureToRender = lyingDownTexture; break;
        case PlayerState::LYING_SHOOTING: textureToRender = lyingShootTexture; break;
        default: textureToRender = runTexture; break;
    }

    if (!textureToRender) { /* ... Error handling ... */ return; }

    SDL_Rect destRect = { static_cast<int>(round(getPos().x-cameraX)), static_cast<int>(round(getPos().y-cameraY)),
                          currentSourceRect.w, currentSourceRect.h };
    SDL_RendererFlip flip = (facing == FacingDirection::LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(window.getRenderer(), textureToRender, &currentSourceRect, &destRect, 0.0, NULL, flip);
}