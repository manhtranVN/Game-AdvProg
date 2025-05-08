#include "Player.hpp"
#include "RenderWindow.hpp"
#include "utils.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <set>
#include <utility>

// Tile Type Constants
const int TILE_EMPTY_P = 0; const int TILE_GRASS_P = 1; const int TILE_UNKNOWN_SOLID_P = 2;
const int TILE_WATER_SURFACE_P = 3; const int TILE_ABYSS_P = 5;

Player::Player(vector2d p_pos,
           SDL_Texture* p_runTex, int p_runSheetCols, SDL_Texture* p_jumpTex, int p_jumpSheetCols,
           SDL_Texture* p_enterWaterTex, int p_enterWaterSheetCols, SDL_Texture* p_swimTex, int p_swimSheetCols,
           SDL_Texture* p_standAimShootUpTex, int p_standAimShootUpSheetCols, SDL_Texture* p_standAimShootDiagUpTex, int p_standAimShootDiagUpSheetCols,
           SDL_Texture* p_standAimShootDiagDownTex, int p_standAimShootDiagDownSheetCols, SDL_Texture* p_runAimShootDiagUpTex, int p_runAimShootDiagUpSheetCols,
           SDL_Texture* p_runAimShootDiagDownTex, int p_runAimShootDiagDownSheetCols, SDL_Texture* p_standAimShootHorizTex, int p_standAimShootHorizSheetCols,
           SDL_Texture* p_runAimShootHorizTex, int p_runAimShootHorizSheetCols, SDL_Texture* p_lyingDownTex, int p_lyingDownSheetCols,
           SDL_Texture* p_lyingAimShootTex, int p_lyingAimShootSheetCols,
           // SDL_Texture* p_deathTex, int p_deathSheetCols, // Đã xóa
           int p_standardFrameW, int p_standardFrameH, int p_lyingFrameW, int p_lyingFrameH)
    : // Initializer list (đã sửa thứ tự)
      pos(p_pos),
      runTexture(p_runTex), jumpTexture(p_jumpTex), enterWaterTexture(p_enterWaterTex), swimTexture(p_swimTex),
      standAimShootHorizTexture(p_standAimShootHorizTex), runAimShootHorizTexture(p_runAimShootHorizTex),
      standAimShootUpTexture(p_standAimShootUpTex),
      standAimShootDiagUpTexture(p_standAimShootDiagUpTex), runAimShootDiagUpTexture(p_runAimShootDiagUpTex),
      standAimShootDiagDownTexture(p_standAimShootDiagDownTex), runAimShootDiagDownTexture(p_runAimShootDiagDownTex),
      lyingDownTexture(p_lyingDownTex), lyingAimShootTexture(p_lyingAimShootTex),
      runSheetColumns(p_runSheetCols), jumpSheetColumns(p_jumpSheetCols), enterWaterSheetColumns(p_enterWaterSheetCols), swimSheetColumns(p_swimSheetCols),
      standAimShootHorizSheetColumns(p_standAimShootHorizSheetCols), runAimShootHorizSheetColumns(p_runAimShootHorizSheetCols),
      standAimShootUpSheetColumns(p_standAimShootUpSheetCols),
      standAimShootDiagUpSheetColumns(p_standAimShootDiagUpSheetCols), runAimShootDiagUpSheetColumns(p_runAimShootDiagUpSheetCols),
      standAimShootDiagDownSheetColumns(p_standAimShootDiagDownSheetCols), runAimShootDiagDownSheetColumns(p_runAimShootDiagDownSheetCols),
      lyingDownSheetColumns(p_lyingDownSheetCols), lyingAimShootSheetColumns(p_lyingAimShootSheetCols),
      currentSourceRect({0, 0, p_standardFrameW, p_standardFrameH}),
      standardFrameWidth(p_standardFrameW), standardFrameHeight(p_standardFrameH),
      lyingFrameWidth(p_lyingFrameW), lyingFrameHeight(p_lyingFrameH),
      animTimer(0.0f), currentAnimFrameIndex(0),
      velocity{0.0f, 0.0f},
      hitbox({10, 4, p_standardFrameW - 20, p_standardFrameH - 8}),
      originalStandingHitboxDef({10, 4, p_standardFrameW - 20, p_standardFrameH - 8}),
      isOnGround(false), isInWaterState(false), waterSurfaceY(0.0f),
      currentState(PlayerState::FALLING), facing(FacingDirection::RIGHT),
      shootRequested(false), aimUpHeld(false), aimDownHeld(false), isShootingHeld(false),
      isLyingDownState(false), isAimingStraightUpState(false),
      wantsToLieDown(false), wantsToStandUp(false), wantsToAimStraightUp(false), wantsToStopAimStraightUp(false),
      shootCooldownTimer(0.0f), lives(4),
      invulnerable(false), invulnerableTimer(0.0f),
      isVisible(true), dyingTimer(0.0f),
      currentMapData(nullptr), currentMapRows(0), currentMapCols(0), currentTileWidth(0), currentTileHeight(0)
{}

// --- Setter ---
void Player::setInvulnerable(bool value) {
    invulnerable = value;
    invulnerableTimer = value ? INVULNERABLE_DURATION : 0.0f;
}

// --- Reset ---
void Player::resetPlayerStateForNewGame(){
    lives = 4; velocity = {0.0f, 0.0f}; currentState = PlayerState::FALLING;
    isOnGround = false; isInWaterState = false; setInvulnerable(false);
    shootCooldownTimer = 0.0f; isLyingDownState = false; isAimingStraightUpState = false;
    aimUpHeld = false; aimDownHeld = false; isShootingHeld = false; shootRequested = false;
    facing = FacingDirection::RIGHT; currentAnimFrameIndex = 0; animTimer = 0.0f;
    hitbox = originalStandingHitboxDef; currentSourceRect = {0, 0, standardFrameWidth, standardFrameHeight};
    temporarilyDisabledTiles.clear(); isVisible = true; dyingTimer = 0.0f;
    std::cout << "Player state reset for new game. Lives: " << lives << std::endl;
}

// --- Getters ---
SDL_Rect Player::getWorldHitbox() {
    SDL_Rect worldHB; worldHB.x = static_cast<int>(round(pos.x + hitbox.x)); worldHB.y = static_cast<int>(round(pos.y + hitbox.y)); worldHB.w = hitbox.w; worldHB.h = hitbox.h; return worldHB;
}

int Player::getTileAt(float worldX, float worldY) const {
    if (!currentMapData || worldX < 0.0f || worldY < 0.0f || currentTileWidth <= 0 || currentTileHeight <= 0 || currentMapData->empty()) return TILE_EMPTY_P;
    int c = static_cast<int>(floor(worldX / currentTileWidth)); int r = static_cast<int>(floor(worldY / currentTileHeight));
    if (r >= 0 && static_cast<size_t>(r) < currentMapData->size()) { const auto& rowData = (*currentMapData)[r]; if (c >= 0 && static_cast<size_t>(c) < rowData.size()) { return rowData[c]; } } return TILE_EMPTY_P;
}

// --- Input Handling ---
void Player::handleInput(const Uint8* keyStates) { /* ... như cũ ... */
    if (currentState == PlayerState::DYING || currentState == PlayerState::DEAD) return;
    aimUpHeld = keyStates[SDL_SCANCODE_UP]; aimDownHeld = keyStates[SDL_SCANCODE_DOWN]; isShootingHeld = keyStates[SDL_SCANCODE_F];
    if (isShootingHeld && shootCooldownTimer <= 0.0f) { shootRequested = true; shootCooldownTimer = SHOOT_COOLDOWN; }
    if (isInWaterState) { if (keyStates[SDL_SCANCODE_LEFT]) { velocity.x = -MOVE_SPEED * 0.7f; facing = FacingDirection::LEFT; } else if (keyStates[SDL_SCANCODE_RIGHT]) { velocity.x = MOVE_SPEED * 0.7f; facing = FacingDirection::RIGHT; } else { velocity.x *= WATER_DRAG_X; if (std::abs(velocity.x) < 1.0f) velocity.x = 0.0f; } }
    else { if (!isLyingDownState && !isAimingStraightUpState) { if (keyStates[SDL_SCANCODE_LEFT]) { velocity.x = -MOVE_SPEED; facing = FacingDirection::LEFT; } else if (keyStates[SDL_SCANCODE_RIGHT]) { velocity.x = MOVE_SPEED; facing = FacingDirection::RIGHT; } else { velocity.x = 0.0f; } } else { velocity.x = 0.0f; } }
}

void Player::handleKeyDown(SDL_Keycode key) { /* ... như cũ ... */
    if (currentState == PlayerState::DYING || currentState == PlayerState::DEAD) return;
    if (isInWaterState) { if (key == SDLK_SPACE) { velocity.y = -WATER_JUMP_STRENGTH; currentAnimFrameIndex = 0; animTimer = 0.0f;} return; }
    if (key == SDLK_SPACE && isOnGround && !isLyingDownState && !isAimingStraightUpState) { velocity.y = -JUMP_STRENGTH; isOnGround = false; currentAnimFrameIndex = 0; animTimer = 0.0f; }
    else if (key == SDLK_d && isOnGround && !isLyingDownState && !isAimingStraightUpState) { SDL_Rect hb_check = getWorldHitbox(); float cX = static_cast<float>(hb_check.x+hb_check.w/2.f), cY = static_cast<float>(hb_check.y+hb_check.h+1.f); int r = static_cast<int>(floor(cY/currentTileHeight)), c = static_cast<int>(floor(cX/currentTileWidth)); if (currentMapData && r>=0 && r<currentMapRows && c>=0 && c<currentMapCols && static_cast<size_t>(c)<(*currentMapData)[r].size() && (*currentMapData)[r][c] == TILE_GRASS_P) { temporarilyDisabledTiles.insert({r, c}); isOnGround=false; currentState=PlayerState::DROPPING; currentAnimFrameIndex=0; animTimer=0.f;} }
    else if (key == SDLK_c && isOnGround && !isInWaterState && !isAimingStraightUpState) { if (!isLyingDownState) wantsToLieDown = true; else wantsToStandUp = true; wantsToStandUp = !wantsToLieDown; }
    else if (key == SDLK_e && isOnGround && !isInWaterState && !isLyingDownState) { if (!isAimingStraightUpState) wantsToAimStraightUp = true; else wantsToStopAimStraightUp = true; wantsToStopAimStraightUp = !wantsToAimStraightUp;}
}

// --- Shooting ---
bool Player::wantsToShoot(vector2d& out_bulletStartPos, vector2d& out_bulletVelocity) { /* ... như cũ ... */
    if (!shootRequested || getIsDead()) { shootRequested = false; return false; } shootRequested = false;
    SDL_Rect hb = getWorldHitbox(); float startX=0.f, startY=0.f, bulletVelX=0.f, bulletVelY=0.f; PlayerState effAim = determineAimingOrShootingState();
    float bOffX=10.f, bOffYStandH=hb.h*0.4f, bOffYStandDU=hb.h*0.1f, bOffYStandDD=hb.h*0.7f, bOffYLying=hb.h*0.5f, bOffYUp=-5.f;
    switch (effAim) {
        case PlayerState::LYING_AIM_SHOOT: startX=(facing==FacingDirection::RIGHT)?(hb.x+hb.w+bOffX/2.f):(hb.x-bOffX); startY=static_cast<float>(hb.y)+bOffYLying; bulletVelX=(facing==FacingDirection::RIGHT)?BULLET_SPEED:-BULLET_SPEED; bulletVelY=0.f; break;
        case PlayerState::STAND_AIM_UP: startX=static_cast<float>(hb.x+hb.w/2.f-2.f); startY=static_cast<float>(hb.y)+bOffYUp; bulletVelX=0.f; bulletVelY=-BULLET_SPEED; break;
        case PlayerState::STAND_AIM_DIAG_UP: case PlayerState::RUN_AIM_DIAG_UP: startX=(facing==FacingDirection::RIGHT)?(hb.x+hb.w*0.8f):(hb.x+hb.w*0.2f-bOffX); startY=static_cast<float>(hb.y)+bOffYStandDU; bulletVelX=(facing==FacingDirection::RIGHT)?BULLET_SPEED_DIAG_COMPONENT:-BULLET_SPEED_DIAG_COMPONENT; bulletVelY=-BULLET_SPEED_DIAG_COMPONENT; break;
        case PlayerState::STAND_AIM_DIAG_DOWN: case PlayerState::RUN_AIM_DIAG_DOWN: startX=(facing==FacingDirection::RIGHT)?(hb.x+hb.w):(hb.x-bOffX); startY=static_cast<float>(hb.y)+bOffYStandDD; bulletVelX=(facing==FacingDirection::RIGHT)?BULLET_SPEED_DIAG_COMPONENT:-BULLET_SPEED_DIAG_COMPONENT; bulletVelY=BULLET_SPEED_DIAG_COMPONENT; break;
        default: startX=(facing==FacingDirection::RIGHT)?(hb.x+hb.w):(hb.x-bOffX); startY=static_cast<float>(hb.y)+bOffYStandH; bulletVelX=(facing==FacingDirection::RIGHT)?BULLET_SPEED:-BULLET_SPEED; bulletVelY=0.f; break;
    } out_bulletStartPos={startX,startY}; out_bulletVelocity={bulletVelX,bulletVelY}; return true;
}

// --- State Determination ---
PlayerState Player::determineAimingOrShootingState() const {
    if (isLyingDownState) return (isShootingHeld || aimUpHeld || aimDownHeld) ? PlayerState::LYING_AIM_SHOOT : PlayerState::LYING_DOWN;
    if (isAimingStraightUpState) return PlayerState::STAND_AIM_UP;

    if (!isOnGround || currentState == PlayerState::JUMPING || currentState == PlayerState::FALLING || currentState == PlayerState::DROPPING) {
        // Khi trên không, trạng thái chính là JUMPING/FALLING/DROPPING
        // Hướng bắn sẽ là ngang (xử lý trong wantsToShoot)
        if (velocity.y < -0.1f) return PlayerState::JUMPING;
        if (velocity.y > 0.1f) return (currentState == PlayerState::DROPPING && !isOnGround) ? PlayerState::DROPPING : PlayerState::FALLING;
        return PlayerState::FALLING; // Mặc định khi v.y gần 0 trên không
    }

    // Trên mặt đất
    bool isMoving = std::abs(velocity.x) > 0.1f;
    if (aimUpHeld) return isMoving ? PlayerState::RUN_AIM_DIAG_UP : PlayerState::STAND_AIM_DIAG_UP;
    if (aimDownHeld) return isMoving ? PlayerState::RUN_AIM_DIAG_DOWN : PlayerState::STAND_AIM_DIAG_DOWN;
    if (isShootingHeld) return isMoving ? PlayerState::RUN_AIM_HORIZ : PlayerState::STAND_AIM_HORIZ;
    return isMoving ? PlayerState::RUNNING : PlayerState::IDLE; // Trạng thái cơ bản
}

// --- Update Logic ---
void Player::update(float dt, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight) {
    currentMapData = &mapData; currentTileWidth = tileWidth; currentTileHeight = tileHeight;
    currentMapRows = mapData.size(); if (currentMapRows > 0) currentMapCols = mapData[0].size(); else currentMapCols = 0;

    if (shootCooldownTimer > 0.0f) { shootCooldownTimer -= dt; }

    if (invulnerable) {
        invulnerableTimer -= dt;
        if (invulnerableTimer <= 0.0f) { invulnerable = false; }
    }

    if (currentState == PlayerState::DYING) {
        dyingTimer += dt;
        if (dyingTimer >= DYING_DURATION) { currentState = PlayerState::DEAD; isVisible = false; }
        else { isVisible = (static_cast<int>(floor(dyingTimer / BLINK_INTERVAL)) % 2 == 0); }
        velocity = {0.0f, 0.0f}; // Đứng yên khi chết
        // Không update animation để giữ frame cuối
        return;
    }

    if (currentState == PlayerState::DEAD) {
        velocity = {0.0f, 0.0f}; isVisible = false;
        return;
    }

    // Xử lý chuyển đổi state nằm/đứng/aim
    if (wantsToStandUp && isLyingDownState) { isLyingDownState = false; wantsToStandUp = false; SDL_Rect hbB = getWorldHitbox(); hitbox = originalStandingHitboxDef; pos.y = static_cast<float>(hbB.y + hbB.h) - hitbox.y - hitbox.h; currentAnimFrameIndex = 0; animTimer = 0.0f; }
    else if (wantsToStopAimStraightUp && isAimingStraightUpState) { isAimingStraightUpState = false; wantsToStopAimStraightUp = false; currentAnimFrameIndex = 0; animTimer = 0.0f; }
    else if (wantsToLieDown && isOnGround && !isLyingDownState && !isInWaterState && !isAimingStraightUpState) { isLyingDownState = true; wantsToLieDown = false; SDL_Rect hbB = getWorldHitbox(); hitbox.w = lyingFrameWidth - 18; hitbox.h = lyingFrameHeight - 10; hitbox.x = 9; hitbox.y = (lyingFrameHeight - hitbox.h); pos.y = static_cast<float>(hbB.y + hbB.h) - hitbox.y - hitbox.h; velocity.x = 0.0f; currentAnimFrameIndex = 0; animTimer = 0.0f; }
    else if (wantsToAimStraightUp && isOnGround && !isAimingStraightUpState && !isInWaterState && !isLyingDownState) { isAimingStraightUpState = true; wantsToAimStraightUp = false; velocity.x = 0.0f; currentAnimFrameIndex = 0; animTimer = 0.0f; }
    wantsToLieDown = wantsToStandUp = wantsToAimStraightUp = wantsToStopAimStraightUp = false;

    if ((!isOnGround || isInWaterState) && (isLyingDownState || isAimingStraightUpState)) { if(isLyingDownState) { isLyingDownState = false; hitbox = originalStandingHitboxDef; } if(isAimingStraightUpState) { isAimingStraightUpState = false; } currentAnimFrameIndex = 0; animTimer = 0.0f; }

    applyGravity(dt);
    movePlayer(dt);
    checkMapCollision();
    if (currentState != PlayerState::DYING && currentState != PlayerState::DEAD) { updateCurrentState(); }
    applyStateBasedMovementRestrictions();
    updatePlayerAnimation(dt);
    restoreDisabledTiles();
}

// --- Physics & Collision ---
void Player::applyGravity(float dt) {
    if ((isLyingDownState && isOnGround) || currentState == PlayerState::DEAD || currentState == PlayerState::DYING) {
        if(isOnGround && currentState != PlayerState::DYING) velocity.y = 0.0f; return;
    }
    if (isInWaterState) { velocity.y += GRAVITY * WATER_GRAVITY_MULTIPLIER * dt; float maxV = MAX_FALL_SPEED * WATER_MAX_SPEED_MULTIPLIER; velocity.y = utils::clamp(velocity.y, -maxV, maxV); }
    else if (!isOnGround) { velocity.y += GRAVITY * dt; velocity.y = std::min(velocity.y, MAX_FALL_SPEED); }
}

void Player::movePlayer(float dt) {
    if (currentState == PlayerState::DEAD || currentState == PlayerState::DYING) return;
    pos.x += velocity.x * dt; pos.y += velocity.y * dt;
}

void Player::applyStateBasedMovementRestrictions() {
    if (getIsDead()) { velocity.x = 0.0f; return; }
    bool blockHorizontal = (isLyingDownState || isAimingStraightUpState);
    if (blockHorizontal) { velocity.x = 0.0f; }
}

void Player::checkMapCollision() {
    if (!currentMapData || currentTileWidth <= 0 || currentTileHeight <= 0 || currentMapRows == 0) return;
    if (currentState == PlayerState::DEAD || currentState == PlayerState::DYING) return;

    SDL_Rect playerHB = getWorldHitbox();

    // Check Ceiling
    if (velocity.y < 0.0f && !isInWaterState) {
        float headY = static_cast<float>(playerHB.y); float midX = static_cast<float>(playerHB.x + playerHB.w / 2.0f); int tileRow = static_cast<int>(floor(headY / currentTileHeight));
        if (getTileAt(midX, headY) == TILE_UNKNOWN_SOLID_P) { pos.y = static_cast<float>((tileRow + 1) * currentTileHeight) - hitbox.y; velocity.y = 50.0f; }
    }
    // Check Floor/Water/Abyss
    else if (velocity.y >= 0.0f || isOnGround) {
        float feetY_center = static_cast<float>(playerHB.y + playerHB.h + ((velocity.y == 0.0f && isOnGround && !isLyingDownState) ? 0.1f : 0.0f));
        float midX = static_cast<float>(playerHB.x + playerHB.w / 2.0f); float feetX_left = static_cast<float>(playerHB.x + 1.0f); float feetX_right = static_cast<float>(playerHB.x + playerHB.w - 1.0f);
        int tileBelowCenter = getTileAt(midX, feetY_center); int tileBelowLeft = getTileAt(feetX_left, feetY_center); int tileBelowRight = getTileAt(feetX_right, feetY_center);
        int landingTileRow = static_cast<int>(floor(feetY_center / currentTileHeight)); bool landedOnSolidThisFrame = false; int effectiveTileBelow = tileBelowCenter;
        if (effectiveTileBelow < TILE_GRASS_P || effectiveTileBelow > TILE_ABYSS_P) { if ((tileBelowLeft >= TILE_GRASS_P && tileBelowLeft <= TILE_ABYSS_P)) effectiveTileBelow = tileBelowLeft; else if ((tileBelowRight >= TILE_GRASS_P && tileBelowRight <= TILE_ABYSS_P)) effectiveTileBelow = tileBelowRight; }
        bool isDroppingThroughThisTile = false;
        if (effectiveTileBelow == TILE_GRASS_P) { int checkCol = static_cast<int>(floor(midX / currentTileWidth)); if (temporarilyDisabledTiles.count({landingTileRow, checkCol})) isDroppingThroughThisTile = true; }
        if (isDroppingThroughThisTile) isOnGround = false;
        else if (effectiveTileBelow == TILE_GRASS_P || effectiveTileBelow == TILE_UNKNOWN_SOLID_P) { pos.y = static_cast<float>(landingTileRow * currentTileHeight) - hitbox.h - hitbox.y; if (velocity.y > 0.0f) velocity.y = 0.0f; isOnGround = true; isInWaterState = false; landedOnSolidThisFrame = true; }
        else if (effectiveTileBelow == TILE_WATER_SURFACE_P) { if (!isInWaterState) { isInWaterState = true; isOnGround = false; waterSurfaceY = static_cast<float>(landingTileRow * currentTileHeight); pos.y = waterSurfaceY - hitbox.h * 0.7f; velocity.y *= 0.3f; updateCurrentState(); /* Force state update */ } } // Gọi updateCurrentState để vào ENTERING_WATER ngay
        else if (effectiveTileBelow == TILE_ABYSS_P) { takeHit(true); return; }
        if (!landedOnSolidThisFrame && effectiveTileBelow != TILE_WATER_SURFACE_P) isOnGround = false;
    }
    if (pos.y + standardFrameHeight > currentMapRows * currentTileHeight) { takeHit(true); return; }

    // Check Walls
    playerHB = getWorldHitbox(); float checkY_mid_wall = static_cast<float>(playerHB.y + playerHB.h / 2.0f); float checkY_top_wall = static_cast<float>(playerHB.y + 1.0f); float checkY_bot_wall = static_cast<float>(playerHB.y + playerHB.h - 1.0f);
    if (velocity.x > 0.0f) { float rightEdge = static_cast<float>(playerHB.x + playerHB.w); if (getTileAt(rightEdge, checkY_mid_wall) == TILE_UNKNOWN_SOLID_P || getTileAt(rightEdge, checkY_top_wall) == TILE_UNKNOWN_SOLID_P || getTileAt(rightEdge, checkY_bot_wall) == TILE_UNKNOWN_SOLID_P) { int tc = static_cast<int>(floor(rightEdge/currentTileWidth)); pos.x = static_cast<float>(tc*currentTileWidth) - hitbox.w - hitbox.x - 0.1f; velocity.x = 0.0f;} }
    else if (velocity.x < 0.0f) { float leftEdge = static_cast<float>(playerHB.x); if (getTileAt(leftEdge, checkY_mid_wall) == TILE_UNKNOWN_SOLID_P || getTileAt(leftEdge, checkY_top_wall) == TILE_UNKNOWN_SOLID_P || getTileAt(leftEdge, checkY_bot_wall) == TILE_UNKNOWN_SOLID_P) { int tc = static_cast<int>(floor(leftEdge/currentTileWidth)); pos.x = static_cast<float>((tc+1)*currentTileWidth) - hitbox.x + 0.1f; velocity.x = 0.0f;} }

    // Exit Water Check
    if (isInWaterState && velocity.y < 0.0f) { if (static_cast<float>(playerHB.y) < waterSurfaceY && getTileAt(static_cast<float>(playerHB.x + playerHB.w/2.0f), static_cast<float>(playerHB.y -1.0f)) == TILE_EMPTY_P) { isInWaterState = false; } }
}

void Player::restoreDisabledTiles() {
    if (temporarilyDisabledTiles.empty() || !currentMapData) return;
    SDL_Rect playerHB = getWorldHitbox(); float feetY = static_cast<float>(playerHB.y + playerHB.h); float headY = static_cast<float>(playerHB.y);
    for (auto it = temporarilyDisabledTiles.begin(); it != temporarilyDisabledTiles.end();) { float tileTopY = static_cast<float>(it->first * currentTileHeight); float tileBotY = static_cast<float>((it->first + 1) * currentTileHeight); if (headY >= tileBotY + 1.0f || feetY <= tileTopY - 1.0f) { it = temporarilyDisabledTiles.erase(it); } else { ++it; } }
}

// ***** HÀM UPDATEDCURRENTSTATE ĐÃ SỬA *****
void Player::updateCurrentState() {
    if (currentState == PlayerState::DYING || currentState == PlayerState::DEAD) return;
    PlayerState previousState = currentState;
    PlayerState nextState = currentState;

    if (isInWaterState) {
        if (previousState == PlayerState::ENTERING_WATER && animTimer < ANIM_SPEED * ENTER_WATER_FRAMES) {
             nextState = PlayerState::ENTERING_WATER; // Giữ state nếu animation chưa xong
        } else {
            // Đã trong nước hoặc vừa xong anim ENTERING_WATER
            nextState = (velocity.y < -1.0f && std::abs(velocity.y) > std::abs(velocity.x * 0.5f)) ? PlayerState::WATER_JUMP : PlayerState::SWIMMING;
        }
    } else if (isLyingDownState) {
        nextState = (isShootingHeld || aimUpHeld || aimDownHeld) ? PlayerState::LYING_AIM_SHOOT : PlayerState::LYING_DOWN;
    } else if (isAimingStraightUpState) {
        nextState = PlayerState::STAND_AIM_UP;
    } else if (!isOnGround) { // Trên không
        // <<< SỬA: Ưu tiên DROPPING nếu đang drop >>>
        if (previousState == PlayerState::DROPPING && !temporarilyDisabledTiles.empty()) { // Kiểm tra thêm temporarilyDisabledTiles để chắc chắn đang drop
            nextState = PlayerState::DROPPING;
        } else if (velocity.y < -0.1f) { // Bay lên
            nextState = PlayerState::JUMPING;
        } else { // Rơi xuống hoặc lơ lửng
            nextState = PlayerState::FALLING;
        }
    } else { // Trên mặt đất
        // <<< SỬA: Logic khi tiếp đất >>>
        // Nếu vừa tiếp đất từ trạng thái trên không
        if (previousState == PlayerState::JUMPING || previousState == PlayerState::FALLING || previousState == PlayerState::DROPPING) {
            bool isMovingHorizontally = std::abs(velocity.x) > 0.1f;
            nextState = isMovingHorizontally ? PlayerState::RUNNING : PlayerState::IDLE;
        } else { // Nếu đã ở trên mặt đất từ trước
            nextState = determineAimingOrShootingState(); // Xác định state dựa trên aim/shoot/move
        }
    }

    if (nextState != previousState) {
        currentState = nextState;
        animTimer = 0.0f;
        // Chỉ reset frame index nếu state mới không phải là tiếp nối của state cũ (ví dụ từ RUNNING -> RUN_AIM_HORIZ thì không cần reset)
        // Logic này có thể phức tạp, tạm thời reset frame cho mọi thay đổi state (trừ khi vào aim/shoot từ idle/run?)
        currentAnimFrameIndex = 0;
    }
}


// updatePlayerAnimation (Đã sửa, không còn case DYING)
void Player::updatePlayerAnimation(float dt) {
    if (currentState == PlayerState::DYING || currentState == PlayerState::DEAD) return;

    animTimer += dt;
    float currentLocalAnimSpeed = ANIM_SPEED;
    bool loopAnim = false;
    int numFramesForState = 1;
    int frameW_anim = standardFrameWidth;
    int frameH_anim = standardFrameHeight;

    switch(currentState) {
        case PlayerState::IDLE: numFramesForState = 1; currentAnimFrameIndex = 0; break;
        case PlayerState::RUNNING: numFramesForState = RUN_FRAMES; loopAnim = true; break;
        case PlayerState::JUMPING: case PlayerState::FALLING: case PlayerState::DROPPING: numFramesForState = JUMP_FRAMES; loopAnim = false; break;
        case PlayerState::ENTERING_WATER: numFramesForState = ENTER_WATER_FRAMES; loopAnim = false; break;
        case PlayerState::SWIMMING: case PlayerState::WATER_JUMP: numFramesForState = SWIM_FRAMES; loopAnim = true; break;
        case PlayerState::STAND_AIM_HORIZ: numFramesForState = STAND_AIM_SHOOT_HORIZ_FRAMES; loopAnim = isShootingHeld; break;
        case PlayerState::RUN_AIM_HORIZ: numFramesForState = RUN_AIM_SHOOT_HORIZ_FRAMES; loopAnim = true; break;
        case PlayerState::STAND_AIM_UP: numFramesForState = STAND_AIM_SHOOT_UP_FRAMES; loopAnim = isShootingHeld; break;
        case PlayerState::STAND_AIM_DIAG_UP: numFramesForState = STAND_AIM_SHOOT_DIAG_UP_FRAMES; loopAnim = isShootingHeld; break;
        case PlayerState::RUN_AIM_DIAG_UP: numFramesForState = RUN_AIM_SHOOT_DIAG_UP_FRAMES; loopAnim = true; break;
        case PlayerState::STAND_AIM_DIAG_DOWN: numFramesForState = STAND_AIM_SHOOT_DIAG_DOWN_FRAMES; loopAnim = isShootingHeld; break;
        case PlayerState::RUN_AIM_DIAG_DOWN: numFramesForState = RUN_AIM_SHOOT_DIAG_DOWN_FRAMES; loopAnim = true; break;
        case PlayerState::LYING_DOWN: numFramesForState = LYING_DOWN_FRAMES; frameW_anim = lyingFrameWidth; frameH_anim = lyingFrameHeight; currentAnimFrameIndex = 0; break;
        case PlayerState::LYING_AIM_SHOOT: numFramesForState = LYING_AIM_SHOOT_FRAMES; loopAnim = isShootingHeld; frameW_anim = lyingFrameWidth; frameH_anim = lyingFrameHeight; break;
        default: break;
    }

    if (numFramesForState > 0 && animTimer >= currentLocalAnimSpeed) { // Thêm check numFrames > 0
        animTimer -= currentLocalAnimSpeed;
        if (loopAnim) currentAnimFrameIndex = (currentAnimFrameIndex + 1) % numFramesForState;
        else if (currentAnimFrameIndex < numFramesForState - 1) currentAnimFrameIndex++;
    }
    if ((currentState == PlayerState::IDLE) || (currentState == PlayerState::LYING_DOWN)) {
        currentAnimFrameIndex = 0; // Đảm bảo frame 0
    }

    currentSourceRect.x = currentAnimFrameIndex * frameW_anim;
    currentSourceRect.y = 0;
    currentSourceRect.w = frameW_anim;
    currentSourceRect.h = frameH_anim;
}

// render (Đã sửa case DYING)
void Player::render(RenderWindow& window, float cameraX, float cameraY) {
    if (currentState == PlayerState::DEAD && lives <= 0) return;
    if (currentState == PlayerState::DYING && !isVisible) return;
    if (currentState == PlayerState::DEAD && lives > 0 && !invulnerable ) return;

    SDL_Texture* textureToUse = nullptr;
    switch(currentState) {
        case PlayerState::IDLE: textureToUse = runTexture; break;
        case PlayerState::RUNNING: textureToUse = runTexture; break;
        case PlayerState::JUMPING: case PlayerState::FALLING: case PlayerState::DROPPING: textureToUse = jumpTexture; break;
        case PlayerState::ENTERING_WATER: textureToUse = enterWaterTexture; break;
        case PlayerState::SWIMMING: case PlayerState::WATER_JUMP: textureToUse = swimTexture; break;
        case PlayerState::STAND_AIM_HORIZ: textureToUse = standAimShootHorizTexture; break;
        case PlayerState::RUN_AIM_HORIZ: textureToUse = runAimShootHorizTexture; break;
        case PlayerState::STAND_AIM_UP: textureToUse = standAimShootUpTexture; break;
        case PlayerState::STAND_AIM_DIAG_UP: textureToUse = standAimShootDiagUpTexture; break;
        case PlayerState::RUN_AIM_DIAG_UP: textureToUse = runAimShootDiagUpTexture; break;
        case PlayerState::STAND_AIM_DIAG_DOWN: textureToUse = standAimShootDiagDownTexture; break;
        case PlayerState::RUN_AIM_DIAG_DOWN: textureToUse = runAimShootDiagDownTexture; break;
        case PlayerState::LYING_DOWN: textureToUse = lyingDownTexture; break;
        case PlayerState::LYING_AIM_SHOOT: textureToUse = lyingAimShootTexture; break;
        case PlayerState::DYING: textureToUse = runTexture; break; // Nhấp nháy với texture đứng yên
        default: textureToUse = runTexture; break;
    }

    if (!textureToUse && currentState != PlayerState::DEAD && currentState != PlayerState::DYING) { std::cerr << "!!!! [RENDER PLAYER] Error: Texture is NULL for state " << static_cast<int>(currentState) << "." << std::endl; return; }
    if (currentState == PlayerState::DYING && !textureToUse) return; // Không vẽ nếu DYING mà texture fallback bị null
    if (currentState == PlayerState::DEAD && !invulnerable) return;

    SDL_Rect destRect = { static_cast<int>(round(pos.x - cameraX)), static_cast<int>(round(pos.y - cameraY)), currentSourceRect.w, currentSourceRect.h };
    SDL_RendererFlip flip = (facing == FacingDirection::LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (invulnerable && currentState != PlayerState::DYING) { bool showPlayer = static_cast<int>(floor(invulnerableTimer / 0.1f)) % 2 == 0; if (!showPlayer) return; }

    if(textureToUse) { SDL_RenderCopyEx(window.getRenderer(), textureToUse, &currentSourceRect, &destRect, 0.0, NULL, flip); }
}

// takeHit (Đã sửa, không knockback)
void Player::takeHit(bool isFallDamage) {
    if (invulnerable || currentState == PlayerState::DYING || currentState == PlayerState::DEAD) return;
    lives--;
    std::cout << "Player hit! Lives remaining: " << lives << std::endl;
    currentState = PlayerState::DYING;
    dyingTimer = 0.0f;
    isVisible = true;
    setInvulnerable(false);
    velocity = {0.0f, 0.0f}; // Dừng chuyển động
}

// respawn (Đã sửa, dùng setter)
void Player::respawn(float p_camX, float initialPlayerY_top, float playerStartXOffset) {
    pos.x = p_camX + playerStartXOffset;
    pos.y = initialPlayerY_top;
    velocity = {0.0f, 0.0f};
    currentState = PlayerState::FALLING;
    isOnGround = false;
    currentAnimFrameIndex = 0;
    animTimer = 0.0f;
    setInvulnerable(true); // Dùng setter
    facing = FacingDirection::RIGHT;
    isLyingDownState = false; isAimingStraightUpState = false;
    hitbox = originalStandingHitboxDef;
    currentSourceRect = {0, 0, standardFrameWidth, standardFrameHeight};
    isVisible = true; // Reset isVisible
    dyingTimer = 0.0f; // Reset dyingTimer
    std::cout << "Player respawned. Lives: " << lives << std::endl;
}