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
#include <SDL2/SDL_mixer.h>

const int TILE_EMPTY_P = 0;
const int TILE_GRASS_P = 1;
const int TILE_UNKNOWN_SOLID_P = 2;
const int TILE_WATER_SURFACE_P = 3;
const int TILE_ABYSS_P = 6;

extern Mix_Chunk* gPlayerDeathSound;

Player::Player(vector2d p_pos,
           SDL_Texture* p_runTex, int p_runSheetCols, SDL_Texture* p_jumpTex, int p_jumpSheetCols,
           SDL_Texture* p_enterWaterTex, int p_enterWaterSheetCols, SDL_Texture* p_swimTex, int p_swimSheetCols,
           SDL_Texture* p_standAimShootUpTex, int p_standAimShootUpSheetCols, SDL_Texture* p_standAimShootDiagUpTex, int p_standAimShootDiagUpSheetCols,
           SDL_Texture* p_standAimShootDiagDownTex, int p_standAimShootDiagDownSheetCols, SDL_Texture* p_runAimShootDiagUpTex, int p_runAimShootDiagUpSheetCols,
           SDL_Texture* p_runAimShootDiagDownTex, int p_runAimShootDiagDownSheetCols, SDL_Texture* p_standAimShootHorizTex, int p_standAimShootHorizSheetCols,
           SDL_Texture* p_runAimShootHorizTex, int p_runAimShootHorizSheetCols, SDL_Texture* p_lyingDownTex, int p_lyingDownSheetCols,
           SDL_Texture* p_lyingAimShootTex, int p_lyingAimShootSheetCols,
           int p_standardFrameW, int p_standardFrameH, int p_lyingFrameW, int p_lyingFrameH)
    :
      pos(p_pos),
      runTexture(p_runTex),
      jumpTexture(p_jumpTex),
      enterWaterTexture(p_enterWaterTex),
      swimTexture(p_swimTex),
      standAimShootHorizTexture(p_standAimShootHorizTex),
      runAimShootHorizTexture(p_runAimShootHorizTex),
      standAimShootUpTexture(p_standAimShootUpTex),
      standAimShootDiagUpTexture(p_standAimShootDiagUpTex),
      runAimShootDiagUpTexture(p_runAimShootDiagUpTex),
      standAimShootDiagDownTexture(p_standAimShootDiagDownTex),
      runAimShootDiagDownTexture(p_runAimShootDiagDownTex),
      lyingDownTexture(p_lyingDownTex),
      lyingAimShootTexture(p_lyingAimShootTex),
      runSheetColumns(p_runSheetCols),
      jumpSheetColumns(p_jumpSheetCols),
      enterWaterSheetColumns(p_enterWaterSheetCols),
      swimSheetColumns(p_swimSheetCols),
      standAimShootHorizSheetColumns(p_standAimShootHorizSheetCols),
      runAimShootHorizSheetColumns(p_runAimShootHorizSheetCols),
      standAimShootUpSheetColumns(p_standAimShootUpSheetCols),
      standAimShootDiagUpSheetColumns(p_standAimShootDiagUpSheetCols),
      runAimShootDiagUpSheetColumns(p_runAimShootDiagUpSheetCols),
      standAimShootDiagDownSheetColumns(p_standAimShootDiagDownSheetCols),
      runAimShootDiagDownSheetColumns(p_runAimShootDiagDownSheetCols),
      lyingDownSheetColumns(p_lyingDownSheetCols),
      lyingAimShootSheetColumns(p_lyingAimShootSheetCols),
      currentSourceRect({0, 0, p_standardFrameW, p_standardFrameH}),
      standardFrameWidth(p_standardFrameW),
      standardFrameHeight(p_standardFrameH),
      lyingFrameWidth(p_lyingFrameW),
      lyingFrameHeight(p_lyingFrameH),
      animTimer(0.0f),
      currentAnimFrameIndex(0),
      velocity{0.0f, 0.0f},
      hitbox({10, 4, p_standardFrameW - 20, p_standardFrameH - 8}),
      originalStandingHitboxDef({10, 4, p_standardFrameW - 20, p_standardFrameH - 8}),
      isOnGround(false),
      isInWaterState(false),
      waterSurfaceY(0.0f),
      currentState(PlayerState::FALLING),
      facing(FacingDirection::RIGHT),
      shootRequested(false),
      aimUpHeld(false),
      aimDownHeld(false),
      isShootingHeld(false),
      isLyingDownState(false),
      isAimingStraightUpState(false),
      wantsToLieDown(false),
      wantsToStandUp(false),
      wantsToAimStraightUp(false),
      wantsToStopAimStraightUp(false),
      shootCooldownTimer(0.0f),
      lives(4),
      invulnerable(false),
      invulnerableTimer(0.0f),
      isVisible(true),
      dyingTimer(0.0f),
      currentMapData(nullptr),
      currentMapRows(0),
      currentMapCols(0),
      currentTileWidth(0),
      currentTileHeight(0)
{}

void Player::setInvulnerable(bool value) {
    invulnerable = value;
    invulnerableTimer = value ? INVULNERABLE_DURATION : 0.0f;
}

void Player::resetPlayerStateForNewGame(){
    lives = 4;
    velocity = {0.0f, 0.0f};
    currentState = PlayerState::FALLING;
    isOnGround = false;
    isInWaterState = false;
    setInvulnerable(false);
    shootCooldownTimer = 0.0f;
    isLyingDownState = false;
    isAimingStraightUpState = false;
    aimUpHeld = false;
    aimDownHeld = false;
    isShootingHeld = false;
    shootRequested = false;
    facing = FacingDirection::RIGHT;
    currentAnimFrameIndex = 0;
    animTimer = 0.0f;
    hitbox = originalStandingHitboxDef;
    currentSourceRect = {0, 0, standardFrameWidth, standardFrameHeight};
    temporarilyDisabledTiles.clear();
    isVisible = true;
    dyingTimer = 0.0f;
    std::cout << "Player state reset for new game. Lives: " << lives << std::endl;
}

SDL_Rect Player::getWorldHitbox() {
    SDL_Rect worldHB;
    worldHB.x = static_cast<int>(round(pos.x + hitbox.x));
    worldHB.y = static_cast<int>(round(pos.y + hitbox.y));
    worldHB.w = hitbox.w;
    worldHB.h = hitbox.h;
    return worldHB;
}

int Player::getTileAt(float worldX, float worldY, bool ignoreDisabledTiles ) const {
    if (!currentMapData || worldX < 0.0f || worldY < 0.0f || currentTileWidth <= 0 || currentTileHeight <= 0 || currentMapData->empty())
    {
        return TILE_EMPTY_P;
    }
    int c = static_cast<int>(floor(worldX / currentTileWidth));
    int r = static_cast<int>(floor(worldY / currentTileHeight));
    if (r >= 0 && static_cast<size_t>(r) < currentMapData->size()) {
        const auto& rowData = (*currentMapData)[r];
        if (c >= 0 && static_cast<size_t>(c) < rowData.size()) {
            if (!ignoreDisabledTiles && temporarilyDisabledTiles.count({r, c})) {
                return TILE_EMPTY_P;
            }
            return rowData[c];
        }
    }
    return TILE_EMPTY_P;
}

void Player::handleInput(const Uint8* keyStates) {
    if (currentState == PlayerState::DYING || currentState == PlayerState::DEAD)
    {
        return;
    }
    aimUpHeld = keyStates[SDL_SCANCODE_UP];
    aimDownHeld = keyStates[SDL_SCANCODE_DOWN];
    isShootingHeld = keyStates[SDL_SCANCODE_F];
    if (isShootingHeld && shootCooldownTimer <= 0.0f)
    {
        shootRequested = true;
        shootCooldownTimer = SHOOT_COOLDOWN;
    }
    if (isInWaterState)
    {
        if (keyStates[SDL_SCANCODE_LEFT])
        {
            velocity.x = -MOVE_SPEED * 0.7f;
            facing = FacingDirection::LEFT;
        }
        else if (keyStates[SDL_SCANCODE_RIGHT])
        {
            velocity.x = MOVE_SPEED * 0.7f;
            facing = FacingDirection::RIGHT;
        }
        else
        {
            velocity.x *= WATER_DRAG_X;
            if (std::abs(velocity.x) < 1.0f)
            {
                velocity.x = 0.0f;
            }
        }
    }
    else
    {
        if (!isLyingDownState && !isAimingStraightUpState)
        {
            if (keyStates[SDL_SCANCODE_LEFT])
            {
                velocity.x = -MOVE_SPEED;
                facing = FacingDirection::LEFT;
            }
            else if (keyStates[SDL_SCANCODE_RIGHT])
            {
                velocity.x = MOVE_SPEED;
                facing = FacingDirection::RIGHT;
            }
            else
            {
                velocity.x = 0.0f;
            }
        }
        else
        {
            velocity.x = 0.0f;
        }
    }
}

void Player::handleKeyDown(SDL_Keycode key) {
    if (currentState == PlayerState::DYING || currentState == PlayerState::DEAD)
    {
        return;
    }
    if (isInWaterState)
    {
        if (key == SDLK_SPACE)
        {
            velocity.y = -WATER_JUMP_STRENGTH;
            currentAnimFrameIndex = 0;
            animTimer = 0.0f;
        }
        return;
    }
    if (key == SDLK_SPACE && isOnGround && !isLyingDownState && !isAimingStraightUpState)
    {
        velocity.y = -JUMP_STRENGTH;
        isOnGround = false;
        currentAnimFrameIndex = 0;
        animTimer = 0.0f;
    }
   else if (key == SDLK_d && isOnGround && !isLyingDownState && !isAimingStraightUpState)
   {
        SDL_Rect hb_check = getWorldHitbox();
        float cX_center = static_cast<float>(hb_check.x + hb_check.w / 2.f);
        float cY_below = static_cast<float>(hb_check.y + hb_check.h + 1.f);

        int r_below = static_cast<int>(floor(cY_below / currentTileHeight));
        int c_center_tile = static_cast<int>(floor(cX_center / currentTileWidth));

        bool initiated_drop = false;

        if (currentMapData && r_below >= 0 && static_cast<size_t>(r_below) < currentMapData->size() &&
            c_center_tile >= 0 && static_cast<size_t>(c_center_tile) < (*currentMapData)[r_below].size() &&
            (*currentMapData)[r_below][c_center_tile] == TILE_GRASS_P)
        {
            const int radius_disable = 3;
            for (int offset = -radius_disable; offset <= radius_disable; ++offset)
            {
                int col_to_check = c_center_tile + offset;

                if (col_to_check >= 0 && col_to_check < currentMapCols)
                {
                    if ((*currentMapData)[r_below][col_to_check] == TILE_GRASS_P)
                    {
                        temporarilyDisabledTiles.insert({r_below, col_to_check});
                        initiated_drop = true;
                    }
                }
            }
        }

        if (initiated_drop)
        {
            isOnGround = false;
            currentState = PlayerState::DROPPING;
            currentAnimFrameIndex = 0;
            animTimer = 0.f;
        }
    }
    else if (key == SDLK_c && isOnGround && !isInWaterState && !isAimingStraightUpState)
    {
        if (!isLyingDownState)
        {
            wantsToLieDown = true;
        }
        else
        {
            wantsToStandUp = true;
        }
        wantsToStandUp = !wantsToLieDown;
    }
    else if (key == SDLK_e && isOnGround && !isInWaterState && !isLyingDownState)
    {
        if (!isAimingStraightUpState)
        {
            wantsToAimStraightUp = true;
        }
        else
        {
            wantsToStopAimStraightUp = true;
        }
        wantsToStopAimStraightUp = !wantsToAimStraightUp;
    }
}

bool Player::wantsToShoot(vector2d& out_bulletStartPos, vector2d& out_bulletVelocity) {
    if (!shootRequested || getIsDead())
    {
        shootRequested = false;
        return false;
    }
    shootRequested = false;
    SDL_Rect hb = getWorldHitbox();
    float startX=0.f;
    float startY=0.f;
    float bulletVelX=0.f;
    float bulletVelY=0.f;
    PlayerState effAim = determineAimingOrShootingState();
    float bOffX=10.f;
    float bOffYStandH=hb.h*0.4f;
    float bOffYStandDU=hb.h*0.1f;
    float bOffYStandDD=hb.h*0.7f;
    float bOffYLying=hb.h*0.5f;
    float bOffYUp=-5.f;

    switch (effAim) {
        case PlayerState::LYING_AIM_SHOOT:
            startX=(facing==FacingDirection::RIGHT)?(hb.x+hb.w+bOffX/2.f):(hb.x-bOffX);
            startY=static_cast<float>(hb.y)+bOffYLying;
            bulletVelX=(facing==FacingDirection::RIGHT)?BULLET_SPEED:-BULLET_SPEED;
            bulletVelY=0.f;
            break;
        case PlayerState::STAND_AIM_UP:
            startX=static_cast<float>(hb.x+hb.w/2.f-2.f);
            startY=static_cast<float>(hb.y)+bOffYUp;
            bulletVelX=0.f;
            bulletVelY=-BULLET_SPEED;
            break;
        case PlayerState::STAND_AIM_DIAG_UP:
        case PlayerState::RUN_AIM_DIAG_UP:
            startX = (facing == FacingDirection::RIGHT) ? (hb.x + hb.w * 0.8f) : (hb.x + hb.w * 0.2f - bOffX);
            startY = static_cast<float>(hb.y) + bOffYStandDU;
            bulletVelX = (facing == FacingDirection::RIGHT) ? BULLET_SPEED_DIAG_COMPONENT : -BULLET_SPEED_DIAG_COMPONENT;
            bulletVelY = -BULLET_SPEED_DIAG_COMPONENT;
            break;
        case PlayerState::STAND_AIM_DIAG_DOWN:
        case PlayerState::RUN_AIM_DIAG_DOWN:
            startX = (facing == FacingDirection::RIGHT) ? (hb.x + hb.w) : (hb.x - bOffX);
            startY = static_cast<float>(hb.y) + bOffYStandDD;
            bulletVelX = (facing == FacingDirection::RIGHT) ? BULLET_SPEED_DIAG_COMPONENT : -BULLET_SPEED_DIAG_COMPONENT;
            bulletVelY = BULLET_SPEED_DIAG_COMPONENT;
            break;
        default:
            startX=(facing==FacingDirection::RIGHT)?(hb.x+hb.w):(hb.x-bOffX);
            startY=static_cast<float>(hb.y)+bOffYStandH;
            bulletVelX=(facing==FacingDirection::RIGHT)?BULLET_SPEED:-BULLET_SPEED;
            bulletVelY=0.f;
            break;
    }
    out_bulletStartPos={startX,startY};
    out_bulletVelocity={bulletVelX,bulletVelY};
    return true;
}

PlayerState Player::determineAimingOrShootingState() const {
    if (isLyingDownState)
    {
        return (isShootingHeld || aimUpHeld || aimDownHeld) ? PlayerState::LYING_AIM_SHOOT : PlayerState::LYING_DOWN;
    }
    if (isAimingStraightUpState)
    {
        return PlayerState::STAND_AIM_UP;
    }

    if (!isOnGround || currentState == PlayerState::JUMPING || currentState == PlayerState::FALLING || currentState == PlayerState::DROPPING)
    {
        if (velocity.y < -0.1f)
        {
            return PlayerState::JUMPING;
        }
        if (velocity.y > 0.1f)
        {
            return (currentState == PlayerState::DROPPING && !isOnGround) ? PlayerState::DROPPING : PlayerState::FALLING;
        }
        return PlayerState::FALLING;
    }
    bool isMoving = std::abs(velocity.x) > 0.1f;
    if (aimUpHeld)
    {
        return isMoving ? PlayerState::RUN_AIM_DIAG_UP : PlayerState::STAND_AIM_DIAG_UP;
    }
    if (aimDownHeld)
    {
        return isMoving ? PlayerState::RUN_AIM_DIAG_DOWN : PlayerState::STAND_AIM_DIAG_DOWN;
    }
    if (isShootingHeld)
    {
        return isMoving ? PlayerState::RUN_AIM_HORIZ : PlayerState::STAND_AIM_HORIZ;
    }
    return isMoving ? PlayerState::RUNNING : PlayerState::IDLE;
}

void Player::update(float dt, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight) {
    currentMapData = &mapData;
    currentTileWidth = tileWidth;
    currentTileHeight = tileHeight;
    currentMapRows = mapData.size();
    if (currentMapRows > 0)
    {
        currentMapCols = mapData[0].size();
    }
    else
    {
        currentMapCols = 0;
    }

    if (shootCooldownTimer > 0.0f)
    {
        shootCooldownTimer -= dt;
    }

    if (invulnerable)
    {
        invulnerableTimer -= dt;
        if (invulnerableTimer <= 0.0f)
        {
            invulnerable = false;
        }
    }

    if (currentState == PlayerState::DYING)
    {
        dyingTimer += dt;
        if (dyingTimer >= DYING_DURATION)
        {
            currentState = PlayerState::DEAD;
            isVisible = false;
        }
        else
        {
            isVisible = (static_cast<int>(floor(dyingTimer / BLINK_INTERVAL)) % 2 == 0);
        }
        velocity = {0.0f, 0.0f};
        return;
    }

    if (currentState == PlayerState::DEAD)
    {
        velocity = {0.0f, 0.0f};
        isVisible = false;
        return;
    }

    if (wantsToStandUp && isLyingDownState)
    {
        isLyingDownState = false;
        wantsToStandUp = false;
        SDL_Rect hbB = getWorldHitbox();
        hitbox = originalStandingHitboxDef;
        pos.y = static_cast<float>(hbB.y + hbB.h) - hitbox.y - hitbox.h;
        currentAnimFrameIndex = 0;
        animTimer = 0.0f;
    }
    else if (wantsToStopAimStraightUp && isAimingStraightUpState)
    {
        isAimingStraightUpState = false;
        wantsToStopAimStraightUp = false;
        currentAnimFrameIndex = 0;
        animTimer = 0.0f;
    }
    else if (wantsToLieDown && isOnGround && !isLyingDownState && !isInWaterState && !isAimingStraightUpState)
    {
        isLyingDownState = true;
        wantsToLieDown = false;
        SDL_Rect hbB = getWorldHitbox();
        hitbox.w = lyingFrameWidth - 18;
        hitbox.h = lyingFrameHeight - 10;
        hitbox.x = 9;
        hitbox.y = (lyingFrameHeight - hitbox.h);
        pos.y = static_cast<float>(hbB.y + hbB.h) - hitbox.y - hitbox.h;
        velocity.x = 0.0f;
        currentAnimFrameIndex = 0;
        animTimer = 0.0f;
    }
    else if (wantsToAimStraightUp && isOnGround && !isAimingStraightUpState && !isInWaterState && !isLyingDownState)
    {
        isAimingStraightUpState = true;
        wantsToAimStraightUp = false;
        velocity.x = 0.0f;
        currentAnimFrameIndex = 0;
        animTimer = 0.0f;
    }
    wantsToLieDown = wantsToStandUp = wantsToAimStraightUp = wantsToStopAimStraightUp = false;

    if ((!isOnGround || isInWaterState) && (isLyingDownState || isAimingStraightUpState))
    {
        if(isLyingDownState)
        {
            isLyingDownState = false;
            hitbox = originalStandingHitboxDef;
        }
        if(isAimingStraightUpState)
        {
            isAimingStraightUpState = false;
        }
        currentAnimFrameIndex = 0;
        animTimer = 0.0f;
    }

    applyGravity(dt);
    movePlayer(dt);
    checkMapCollision();
    if (currentState != PlayerState::DYING && currentState != PlayerState::DEAD)
    {
        updateCurrentState();
    }
    applyStateBasedMovementRestrictions();
    updatePlayerAnimation(dt);
    restoreDisabledTiles();
}

void Player::applyGravity(float dt) {
    if ((isLyingDownState && isOnGround) || currentState == PlayerState::DEAD || currentState == PlayerState::DYING) {
        if(isOnGround && currentState != PlayerState::DYING && currentState != PlayerState::DEAD)
        {
            velocity.y = 0.0f;
        }
        return;
    }
    if (!isOnGround || isInWaterState)
    {
        velocity.y += GRAVITY * dt;
    }
    if (isInWaterState)
    {
        velocity.y = utils::clamp(velocity.y, -MAX_FALL_SPEED, MAX_FALL_SPEED);
    }
    else
    {
        velocity.y = std::min(velocity.y, MAX_FALL_SPEED);
    }
}


void Player::movePlayer(float dt) {
    if (currentState == PlayerState::DEAD || currentState == PlayerState::DYING)
    {
        return;
    }
    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
}

void Player::applyStateBasedMovementRestrictions() {
    if (getIsDead())
    {
        velocity.x = 0.0f;
        return;
    }
    bool blockHorizontal = (isLyingDownState || isAimingStraightUpState);
    if (blockHorizontal)
    {
        velocity.x = 0.0f;
    }
}

void Player::checkMapCollision() {
    if (!currentMapData || currentTileWidth <= 0 || currentTileHeight <= 0 || currentMapRows == 0)
    {
        return;
    }
    if (currentState == PlayerState::DYING || currentState == PlayerState::DEAD)
    {
        return;
    }

    SDL_Rect playerHB = getWorldHitbox();
    bool deathTriggeredThisCollisionCheck = false;

    if (velocity.y < 0.0f && !isInWaterState) {
        float headY = static_cast<float>(playerHB.y);
        float midX = static_cast<float>(playerHB.x + playerHB.w / 2.0f);
        int tileRow = static_cast<int>(floor(headY / currentTileHeight));
        if (getTileAt(midX, headY) == TILE_UNKNOWN_SOLID_P) {
            pos.y = static_cast<float>((tileRow + 1) * currentTileHeight) - hitbox.y;
            velocity.y = 50.0f;
        }
    }

    bool landedOnSolidThisFrame = false;
    bool fellIntoWaterThisFrame = false;

    if (velocity.y >= 0.0f || isOnGround) {
        float feetY_check_offset = (isOnGround && !isLyingDownState && velocity.y == 0.0f) ? 0.1f : 1.0f;
        float feetY_center = static_cast<float>(playerHB.y + playerHB.h + feetY_check_offset);
        float midX = static_cast<float>(playerHB.x + playerHB.w / 2.0f);

        int tileRowBelow = static_cast<int>(floor(feetY_center / currentTileHeight));

        if (tileRowBelow < currentMapRows)
        {
            float checkPointsX[] = {
                static_cast<float>(playerHB.x + hitbox.w * 0.25f),
                midX,
                static_cast<float>(playerHB.x + hitbox.w * 0.75f)
            };
            int effectiveTileBelow = TILE_EMPTY_P;
            bool foundSolidOrWater = false;

            for (float checkX : checkPointsX)
            {
                int tileType = getTileAt(checkX, feetY_center);
                if (tileType == TILE_GRASS_P || tileType == TILE_UNKNOWN_SOLID_P)
                {
                    effectiveTileBelow = tileType;
                    foundSolidOrWater = true;
                    break;
                }
                if (tileType == TILE_WATER_SURFACE_P && !foundSolidOrWater)
                {
                    effectiveTileBelow = tileType;
                    foundSolidOrWater = true;
                }
                if (tileType == TILE_ABYSS_P && !foundSolidOrWater)
                {
                     effectiveTileBelow = TILE_ABYSS_P;
                }
            }
            if (!foundSolidOrWater && effectiveTileBelow != TILE_ABYSS_P)
            {
                effectiveTileBelow = getTileAt(midX, feetY_center);
            }


            bool isDroppingThroughThisTile = false;
            if (effectiveTileBelow == TILE_GRASS_P)
            {
                int checkCol = static_cast<int>(floor(midX / currentTileWidth));
                if (temporarilyDisabledTiles.count({tileRowBelow, checkCol}))
                {
                    isDroppingThroughThisTile = true;
                }
            }

            if (isDroppingThroughThisTile) {
                isOnGround = false;
            }
            else if (effectiveTileBelow == TILE_GRASS_P || effectiveTileBelow == TILE_UNKNOWN_SOLID_P)
            {
                pos.y = static_cast<float>(tileRowBelow * currentTileHeight) - hitbox.h - hitbox.y;
                if (velocity.y > 0.0f)
                {
                    velocity.y = 0.0f;
                }
                isOnGround = true;
                isInWaterState = false;
                landedOnSolidThisFrame = true;
            }
            else if (effectiveTileBelow == TILE_WATER_SURFACE_P)
            {
                if (!isInWaterState)
                {
                    fellIntoWaterThisFrame = true;
                    isInWaterState = true;
                    isOnGround = false;
                    waterSurfaceY = static_cast<float>(tileRowBelow * currentTileHeight);
                    pos.y = waterSurfaceY - hitbox.h * 0.7f;
                }

            }
            else
            {
                 isOnGround = false;
            }
        }
        else
        {
             isOnGround = false;
        }

        if (!landedOnSolidThisFrame && !isInWaterState && !fellIntoWaterThisFrame)
        {
             isOnGround = false;
        }
    }

    playerHB = getWorldHitbox();
    float checkY_mid_wall = static_cast<float>(playerHB.y + hitbox.h / 2.0f);
    float checkY_top_wall = static_cast<float>(playerHB.y + 1.0f);
    float checkY_bot_wall = static_cast<float>(playerHB.y + hitbox.h - 1.0f);

    if (velocity.x > 0.0f)
    {
        float rightEdge = static_cast<float>(playerHB.x + playerHB.w);
        if (getTileAt(rightEdge, checkY_mid_wall) == TILE_UNKNOWN_SOLID_P ||
            getTileAt(rightEdge, checkY_top_wall) == TILE_UNKNOWN_SOLID_P ||
            getTileAt(rightEdge, checkY_bot_wall) == TILE_UNKNOWN_SOLID_P) {
            int tc = static_cast<int>(floor(rightEdge / currentTileWidth));
            pos.x = static_cast<float>(tc * currentTileWidth) - hitbox.w - hitbox.x - 0.1f;
            velocity.x = 0.0f;
        }
    }
    else if (velocity.x < 0.0f)
    {
        float leftEdge = static_cast<float>(playerHB.x);
        if (getTileAt(leftEdge, checkY_mid_wall) == TILE_UNKNOWN_SOLID_P ||
            getTileAt(leftEdge, checkY_top_wall) == TILE_UNKNOWN_SOLID_P ||
            getTileAt(leftEdge, checkY_bot_wall) == TILE_UNKNOWN_SOLID_P) {
            int tc = static_cast<int>(floor(leftEdge / currentTileWidth));
            pos.x = static_cast<float>((tc + 1) * currentTileWidth) - hitbox.x + 0.1f;
            velocity.x = 0.0f;
        }
    }

    if (isInWaterState && !fellIntoWaterThisFrame && velocity.y < 0.0f)
    {
        playerHB = getWorldHitbox();
        if (static_cast<float>(playerHB.y) < waterSurfaceY)
        {
             float midXPlayer = static_cast<float>(playerHB.x + playerHB.w / 2.0f);
             float yAboveWaterSurface = waterSurfaceY - 1.0f;
             if (getTileAt(midXPlayer, yAboveWaterSurface) == TILE_EMPTY_P)
             {
                isInWaterState = false;
             }
        }
    }

    if (isInWaterState && currentMapRows > 0 && !deathTriggeredThisCollisionCheck &&
        (currentState != PlayerState::DYING && currentState != PlayerState::DEAD)) {

        playerHB = getWorldHitbox();
        int lastRowActualIndex = currentMapRows - 1;
        float playerFeetYForLastRowCheck = static_cast<float>(playerHB.y + playerHB.h + 1.0f);
        float playerMidX = static_cast<float>(playerHB.x + playerHB.w / 2.0f);

        if (static_cast<int>(floor(playerFeetYForLastRowCheck / currentTileHeight)) >= lastRowActualIndex) {
            int tileBelowPlayerAtLastRow = getTileAt(playerMidX, (static_cast<float>(lastRowActualIndex) + 0.5f) * currentTileHeight);

            if (tileBelowPlayerAtLastRow == TILE_GRASS_P) {

                pos.y = static_cast<float>(lastRowActualIndex * currentTileHeight) - hitbox.h - hitbox.y;
                velocity.y = 0.0f;
                isOnGround = true;
                isInWaterState = false;
                currentAnimFrameIndex = 0;
                animTimer = 0.0f;
            }
        }
    }


    if (!deathTriggeredThisCollisionCheck &&
        (currentState != PlayerState::DYING && currentState != PlayerState::DEAD))
    {

        playerHB = getWorldHitbox();
        float playerBottomEdgeY = static_cast<float>(playerHB.y + playerHB.h);
        float mapAbsoluteBottomYWithTolerance = static_cast<float>(currentMapRows * currentTileHeight) - 0.5f;

        if (playerBottomEdgeY >= mapAbsoluteBottomYWithTolerance)
        {
            if (currentMapRows > 0) {
                int lastRowActualIndex = currentMapRows - 1;
                float playerMidX = static_cast<float>(playerHB.x + playerHB.w / 2.0f);

                int tileInLastRow = getTileAt(playerMidX, (static_cast<float>(lastRowActualIndex) + 0.5f) * currentTileHeight);

                if (tileInLastRow == TILE_WATER_SURFACE_P)
                {
                    pos.y = static_cast<float>(currentMapRows * currentTileHeight) - static_cast<float>(hitbox.h) - hitbox.y - 0.1f;
                    velocity.y = 0.0f;
                    isOnGround = false;
                    if (!isInWaterState)
                    {
                        isInWaterState = true;
                        waterSurfaceY = static_cast<float>(lastRowActualIndex * currentTileHeight);
                        currentAnimFrameIndex = 0;
                        animTimer = 0.0f;
                    }
                }
                else if (tileInLastRow == TILE_GRASS_P)
                {
                    pos.y = static_cast<float>(currentMapRows * currentTileHeight) - static_cast<float>(hitbox.h) - hitbox.y;
                    velocity.y = 0.0f;
                    isOnGround = true;
                    isInWaterState = false;
                }
                else
                {
                    if (!invulnerable)
                    {
                        takeHit(true);
                        deathTriggeredThisCollisionCheck = true;
                    }
                    else
                    {
                        pos.y = static_cast<float>(currentMapRows * currentTileHeight) - static_cast<float>(hitbox.h) - hitbox.y - 0.1f;
                        velocity.y = 0.0f;
                        isOnGround = true;
                        isInWaterState = false;
                    }
                }
            }
            else
            {
                if (!invulnerable)
                {
                    takeHit(true);
                    deathTriggeredThisCollisionCheck = true;
                }
            }
        }
    }
}


void Player::restoreDisabledTiles(bool forceRestoreAll) {
    if (forceRestoreAll) {
        if (!temporarilyDisabledTiles.empty())
        {
            temporarilyDisabledTiles.clear();
        }
        return;
    }

    if (temporarilyDisabledTiles.empty() || !currentMapData)
    {
        return;
    }

    SDL_Rect playerHB = getWorldHitbox();
    float headY = static_cast<float>(playerHB.y);
    float feetY = static_cast<float>(playerHB.y + playerHB.h);

    for (auto it = temporarilyDisabledTiles.begin(); it != temporarilyDisabledTiles.end();) {
        float tileTopY = static_cast<float>(it->first * currentTileHeight);
        float tileBotY = static_cast<float>((it->first + 1) * currentTileHeight);
        if (headY >= tileBotY + 1.0f || feetY <= tileTopY - 1.0f)
        {
            it = temporarilyDisabledTiles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Player::updateCurrentState() {
    if (currentState == PlayerState::DYING || currentState == PlayerState::DEAD)
    {
        return;
    }
    PlayerState previousState = currentState;
    PlayerState nextState = currentState;

    if (isInWaterState) {
        if (previousState != PlayerState::ENTERING_WATER &&
            previousState != PlayerState::SWIMMING &&
            previousState != PlayerState::WATER_JUMP)
        {
            nextState = PlayerState::ENTERING_WATER;
        }
        else if (currentState == PlayerState::ENTERING_WATER && currentAnimFrameIndex < ENTER_WATER_FRAMES -1 && ENTER_WATER_FRAMES > 1)
        {
             nextState = PlayerState::ENTERING_WATER;
        }
        else
        {
            if (velocity.y < -10.0f && std::abs(velocity.y) > std::abs(velocity.x * 0.5f) )
            {
                nextState = PlayerState::WATER_JUMP;
            }
            else
            {
                nextState = PlayerState::SWIMMING;
            }
        }
    }
    else if (isLyingDownState)
    {
        nextState = (isShootingHeld || aimUpHeld || aimDownHeld) ? PlayerState::LYING_AIM_SHOOT : PlayerState::LYING_DOWN;
    }
    else if (isAimingStraightUpState)
    {
        nextState = PlayerState::STAND_AIM_UP;
    }
    else if (!isOnGround)
    {
        if (previousState == PlayerState::DROPPING && !temporarilyDisabledTiles.empty())
        {
            nextState = PlayerState::DROPPING;
        }
        else if (velocity.y < -0.1f)
        {
            nextState = PlayerState::JUMPING;
        }
        else
        {
            nextState = PlayerState::FALLING;
        }
    }
    else
    {
        if (previousState == PlayerState::JUMPING || previousState == PlayerState::FALLING || previousState == PlayerState::DROPPING)
        {
            const Uint8* keyStates = SDL_GetKeyboardState(NULL);
            bool movingIntent = keyStates[SDL_SCANCODE_LEFT] || keyStates[SDL_SCANCODE_RIGHT];
            nextState = movingIntent ? PlayerState::RUNNING : PlayerState::IDLE;
        }
        else
        {
            nextState = determineAimingOrShootingState();
        }
    }

    // Phần này vẫn giữ nguyên, nó áp dụng nextState đã được tính toán
    if (nextState != previousState)
    {
        currentState = nextState;
        animTimer = 0.0f;

        if (!(
               (previousState == PlayerState::IDLE &&
                (nextState == PlayerState::RUNNING ||
                 nextState == PlayerState::STAND_AIM_HORIZ ||
                 nextState == PlayerState::STAND_AIM_DIAG_UP ||
                 nextState == PlayerState::STAND_AIM_DIAG_DOWN ||
                 nextState == PlayerState::STAND_AIM_UP))
               ||
               (previousState == PlayerState::RUNNING &&
                (nextState == PlayerState::IDLE ||
                 nextState == PlayerState::RUN_AIM_HORIZ ||
                 nextState == PlayerState::RUN_AIM_DIAG_UP ||
                 nextState == PlayerState::RUN_AIM_DIAG_DOWN))
               ||
               (previousState == PlayerState::SWIMMING && nextState == PlayerState::WATER_JUMP) ||
               (previousState == PlayerState::WATER_JUMP && nextState == PlayerState::SWIMMING) ||
               (previousState == PlayerState::ENTERING_WATER && nextState == PlayerState::SWIMMING && ENTER_WATER_FRAMES == 1)
            ))
        {
            currentAnimFrameIndex = 0;
        }

        if (nextState == PlayerState::ENTERING_WATER && previousState != PlayerState::ENTERING_WATER)
        {
            currentAnimFrameIndex = 0;
        }
    }
}

void Player::updatePlayerAnimation(float dt) {
    if (currentState == PlayerState::DYING || currentState == PlayerState::DEAD)
    {
        return;
    }

    animTimer += dt;
    float currentLocalAnimSpeed = ANIM_SPEED;
    bool loopAnim = false;
    int numFramesForState = 1;
    int frameW_anim = standardFrameWidth;
    int frameH_anim = standardFrameHeight;

    switch(currentState) {
        case PlayerState::IDLE:
            numFramesForState = 1;
            currentAnimFrameIndex = 0;
            break;
        case PlayerState::RUNNING:
            numFramesForState = RUN_FRAMES;
            loopAnim = true;
            break;
        case PlayerState::JUMPING:
        case PlayerState::FALLING:
        case PlayerState::DROPPING:
            numFramesForState = JUMP_FRAMES;
            loopAnim = false;
            break;
        case PlayerState::ENTERING_WATER:
            numFramesForState = ENTER_WATER_FRAMES;
            loopAnim = (ENTER_WATER_FRAMES == 1);
            break;
        case PlayerState::SWIMMING:
        case PlayerState::WATER_JUMP:
            numFramesForState = SWIM_FRAMES;
            loopAnim = true;
            break;
        case PlayerState::STAND_AIM_HORIZ:
            numFramesForState = STAND_AIM_SHOOT_HORIZ_FRAMES;
            loopAnim = isShootingHeld;
            break;
        case PlayerState::RUN_AIM_HORIZ:
            numFramesForState = RUN_AIM_SHOOT_HORIZ_FRAMES;
            loopAnim = true;
            break;
        case PlayerState::STAND_AIM_UP:
            numFramesForState = STAND_AIM_SHOOT_UP_FRAMES;
            loopAnim = isShootingHeld;
            break;
        case PlayerState::STAND_AIM_DIAG_UP:
            numFramesForState = STAND_AIM_SHOOT_DIAG_UP_FRAMES;
            loopAnim = isShootingHeld;
            break;
        case PlayerState::RUN_AIM_DIAG_UP:
            numFramesForState = RUN_AIM_SHOOT_DIAG_UP_FRAMES;
            loopAnim = true;
            break;
        case PlayerState::STAND_AIM_DIAG_DOWN:
            numFramesForState = STAND_AIM_SHOOT_DIAG_DOWN_FRAMES;
            loopAnim = isShootingHeld;
            break;
        case PlayerState::RUN_AIM_DIAG_DOWN:
            numFramesForState = RUN_AIM_SHOOT_DIAG_DOWN_FRAMES;
            loopAnim = true;
            break;
        case PlayerState::LYING_DOWN:
            numFramesForState = LYING_DOWN_FRAMES;
            frameW_anim = lyingFrameWidth;
            frameH_anim = lyingFrameHeight;
            currentAnimFrameIndex = 0;
            break;
        case PlayerState::LYING_AIM_SHOOT:
            numFramesForState = LYING_AIM_SHOOT_FRAMES;
            loopAnim = isShootingHeld;
            frameW_anim = lyingFrameWidth;
            frameH_anim = lyingFrameHeight;
            break;
        case PlayerState::DYING:
            numFramesForState = 1;
            break;
        case PlayerState::DEAD:
            numFramesForState = 1;
            break;
        default:
            numFramesForState = 1;
            break;
    }

    if (numFramesForState > 0 && animTimer >= currentLocalAnimSpeed)
    {
        animTimer -= currentLocalAnimSpeed;
        if (loopAnim)
        {
            currentAnimFrameIndex = (currentAnimFrameIndex + 1) % numFramesForState;
        }
        else if (currentAnimFrameIndex < numFramesForState - 1)
        {
            currentAnimFrameIndex++;
        }
    }

    if ((currentState == PlayerState::IDLE) ||
        (currentState == PlayerState::LYING_DOWN) ||
        (numFramesForState == 1 && !loopAnim) )
    {
        currentAnimFrameIndex = 0;
    }
    if ((currentState == PlayerState::JUMPING || currentState == PlayerState::FALLING || currentState == PlayerState::DROPPING) && !loopAnim) {
        if (currentAnimFrameIndex >= numFramesForState -1)
        {
            currentAnimFrameIndex = numFramesForState -1;
        }
    }


    currentSourceRect.x = currentAnimFrameIndex * frameW_anim;
    currentSourceRect.y = 0;
    currentSourceRect.w = frameW_anim;
    currentSourceRect.h = frameH_anim;
}

void Player::render(RenderWindow& window, float cameraX, float cameraY) {
    if (currentState == PlayerState::DEAD && lives <= 0)
    {
        return;
    }
    if (currentState == PlayerState::DYING && !isVisible)
    {
        return;
    }
    if (currentState == PlayerState::DEAD && lives > 0 && !invulnerable)
    {
        return;
    }


    SDL_Texture* textureToUse = nullptr;
    switch(currentState) {
        case PlayerState::IDLE: textureToUse = runTexture; break;
        case PlayerState::RUNNING: textureToUse = runTexture; break;
        case PlayerState::JUMPING:
        case PlayerState::FALLING:
        case PlayerState::DROPPING:
            textureToUse = jumpTexture;
            break;
        case PlayerState::ENTERING_WATER: textureToUse = enterWaterTexture; break;
        case PlayerState::SWIMMING:
        case PlayerState::WATER_JUMP:
            textureToUse = swimTexture;
            break;
        case PlayerState::STAND_AIM_HORIZ: textureToUse = standAimShootHorizTexture; break;
        case PlayerState::RUN_AIM_HORIZ: textureToUse = runAimShootHorizTexture; break;
        case PlayerState::STAND_AIM_UP: textureToUse = standAimShootUpTexture; break;
        case PlayerState::STAND_AIM_DIAG_UP: textureToUse = standAimShootDiagUpTexture; break;
        case PlayerState::RUN_AIM_DIAG_UP: textureToUse = runAimShootDiagUpTexture; break;
        case PlayerState::STAND_AIM_DIAG_DOWN: textureToUse = standAimShootDiagDownTexture; break;
        case PlayerState::RUN_AIM_DIAG_DOWN: textureToUse = runAimShootDiagDownTexture; break;
        case PlayerState::LYING_DOWN: textureToUse = lyingDownTexture; break;
        case PlayerState::LYING_AIM_SHOOT: textureToUse = lyingAimShootTexture; break;
        case PlayerState::DYING: textureToUse = runTexture; break;
        case PlayerState::DEAD:
             if(invulnerable)
             {
                 textureToUse = jumpTexture;
             }
             else
             {
                 return;
             }
             break;
        default: textureToUse = runTexture; break;
    }

    if (!textureToUse) {
         if (!(currentState == PlayerState::DEAD && !invulnerable))
         {
            std::cerr << "!!!! [RENDER PLAYER] Error: Texture is NULL for state " << static_cast<int>(currentState) << "." << std::endl;
         }
         return;
    }

    SDL_Rect destRect = {
        static_cast<int>(round(pos.x - cameraX)),
        static_cast<int>(round(pos.y - cameraY)),
        currentSourceRect.w,
        currentSourceRect.h
    };
    SDL_RendererFlip flip = (facing == FacingDirection::LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (invulnerable && currentState != PlayerState::DYING)
    {
        bool showPlayer = static_cast<int>(floor(invulnerableTimer / 0.1f)) % 2 == 0;
        if (!showPlayer)
        {
            return;
        }
    }

    if(textureToUse)
    {
        SDL_RenderCopyEx(window.getRenderer(), textureToUse, &currentSourceRect, &destRect, 0.0, NULL, flip);
    }
}

void Player::takeHit(bool isFallDamage) {
    if (invulnerable || currentState == PlayerState::DYING || currentState == PlayerState::DEAD)
    {
        return;
    }
    lives--;
    std::cout << "Player hit! Lives remaining: " << lives << std::endl;
    currentState = PlayerState::DYING;
    dyingTimer = 0.0f;
    isVisible = true;
    setInvulnerable(false);
    velocity = {0.0f, 0.0f};
    if (gPlayerDeathSound)
    {
        Mix_PlayChannel(-1, gPlayerDeathSound, 0);
    }
}

void Player::respawn(float p_camX, float initialPlayerY_top, float playerStartXOffset) {
    pos.x = p_camX + playerStartXOffset;
    pos.y = initialPlayerY_top;
    velocity = {0.0f, 0.0f};
    currentState = PlayerState::FALLING;
    isOnGround = false;
    isInWaterState = false;
    currentAnimFrameIndex = 0;
    animTimer = 0.0f;
    setInvulnerable(true);
    facing = FacingDirection::RIGHT;
    isLyingDownState = false;
    isAimingStraightUpState = false;
    hitbox = originalStandingHitboxDef;
    currentSourceRect = {0, 0, standardFrameWidth, standardFrameHeight};
    isVisible = true;
    dyingTimer = 0.0f;
    std::cout << "Player respawned. Lives: " << lives << std::endl;
}