#include "Enemy.hpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <SDL2/SDL_mixer.h>
#include <algorithm>

const int TILE_EMPTY_E = 0;
const int TILE_GRASS_E = 1;
const int TILE_UNKNOWN_SOLID_E = 2;

Enemy::Enemy(vector2d p_pos, SDL_Texture* p_tex)
    : // Khởi tạo theo đúng thứ tự khai báo trong Enemy.hpp
      pos(p_pos),
      tex(p_tex),
      // currentFrame sẽ được khởi tạo sau khi frameWidth/Height có giá trị
      // frameWidth, frameHeight, sheetColumns được tính toán bên dưới
      currentAnimFrameIndex(0),
      animTimer(0.0f),
      currentState(EnemyState::ALIVE),
      // hitbox sẽ được khởi tạo sau khi frameWidth/Height có giá trị
      velocityY(0.0f),
      isOnGround(false),
      dyingTimer(0.0f),
      isVisible(true),
      movingRight(false) // Enemy bắt đầu đi sang trái
{
    if (tex) {
        int totalTextureWidth, totalTextureHeight;
        SDL_QueryTexture(tex, NULL, NULL, &totalTextureWidth, &totalTextureHeight);
        sheetColumns = NUM_FRAMES_WALK;
        if (sheetColumns > 0) {
            frameWidth = totalTextureWidth / sheetColumns;
            frameHeight = totalTextureHeight;
        } else {
            frameWidth = 40; frameHeight = 72; sheetColumns = 1;
            std::cerr << "Warning: Enemy texture has 0 sheet columns? Using default values." << std::endl;
        }
    } else {
        frameWidth = 40; frameHeight = 72; sheetColumns = NUM_FRAMES_WALK;
        std::cerr << "Error: Enemy created with NULL texture! Using default values." << std::endl;
    }

    // Khởi tạo currentFrame và hitbox sau khi có frameWidth/Height
    currentFrame.x = 0; currentFrame.y = 0;
    currentFrame.w = frameWidth; currentFrame.h = frameHeight;

    hitbox.w = frameWidth - 10;
    hitbox.h = frameHeight - 5;
    hitbox.x = (frameWidth - hitbox.w) / 2;
    hitbox.y = frameHeight - hitbox.h;
}

// ... (Các hàm update, render, getTileAt, takeHit, etc. giữ nguyên như trước) ...

int Enemy::getTileAt(float worldX, float worldY, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight) const {
    if (worldX < 0.0f || worldY < 0.0f || tileWidth <= 0 || tileHeight <= 0 || mapData.empty()) return TILE_EMPTY_E;
    int col = static_cast<int>(floor(worldX / tileWidth));
    int row = static_cast<int>(floor(worldY / tileHeight));

    // Sửa lỗi sign-compare
    if (row >= 0 && static_cast<size_t>(row) < mapData.size()) {
        const auto& rowData = mapData[row];
        if (col >= 0 && static_cast<size_t>(col) < rowData.size()) {
            return rowData[col];
        }
    }
    return TILE_EMPTY_E;
}

void Enemy::update(float dt, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight) {
    switch (currentState) {
        case EnemyState::ALIVE: {
            if (!isOnGround) {
                velocityY += GRAVITY * dt;
                velocityY = std::min(velocityY, MAX_FALL_SPEED);
            }
            float tentativeY = pos.y + velocityY * dt;
            isOnGround = false;
            SDL_Rect nextWorldHitbox = getWorldHitbox();
            nextWorldHitbox.y = static_cast<int>(round(tentativeY + hitbox.y));
            float feetX_left = static_cast<float>(nextWorldHitbox.x + 1.0f);
            float feetX_mid = static_cast<float>(nextWorldHitbox.x + nextWorldHitbox.w / 2.0f);
            float feetX_right = static_cast<float>(nextWorldHitbox.x + nextWorldHitbox.w - 1.0f);
            float feetY_check = static_cast<float>(nextWorldHitbox.y + nextWorldHitbox.h + 0.1f);
            int tileBelowLeft = getTileAt(feetX_left, feetY_check, mapData, tileWidth, tileHeight);
            int tileBelowMid = getTileAt(feetX_mid, feetY_check, mapData, tileWidth, tileHeight);
            int tileBelowRight = getTileAt(feetX_right, feetY_check, mapData, tileWidth, tileHeight);
            int standingOnTileType = TILE_EMPTY_E;
            if (tileBelowMid == TILE_GRASS_E || tileBelowMid == TILE_UNKNOWN_SOLID_E) standingOnTileType = tileBelowMid;
            else if (tileBelowLeft == TILE_GRASS_E || tileBelowLeft == TILE_UNKNOWN_SOLID_E) standingOnTileType = tileBelowLeft;
            else if (tileBelowRight == TILE_GRASS_E || tileBelowRight == TILE_UNKNOWN_SOLID_E) standingOnTileType = tileBelowRight;
            if (standingOnTileType != TILE_EMPTY_E) {
                int tileRowBelow = static_cast<int>(floor(feetY_check / tileHeight));
                float groundSurfaceY = static_cast<float>(tileRowBelow * tileHeight);
                if (static_cast<float>(nextWorldHitbox.y + nextWorldHitbox.h) >= groundSurfaceY - 0.1f) {
                    pos.y = groundSurfaceY - static_cast<float>(frameHeight);
                    velocityY = 0.0f;
                    isOnGround = true;
                } else { pos.y = tentativeY; }
            } else { pos.y = tentativeY; }

            if (isOnGround) {
                float checkX_ahead;
                float checkY_wall = pos.y + frameHeight / 2.0f;
                float checkY_ground_ahead = pos.y + frameHeight + 1.0f;
                if (movingRight) checkX_ahead = pos.x + frameWidth + 1.0f;
                else checkX_ahead = pos.x - 1.0f;
                int tileInFrontWall = getTileAt(checkX_ahead, checkY_wall, mapData, tileWidth, tileHeight);
                int tileBelowFront = getTileAt(checkX_ahead, checkY_ground_ahead, mapData, tileWidth, tileHeight);
                bool shouldTurn = false;
                if (tileInFrontWall == TILE_UNKNOWN_SOLID_E || tileInFrontWall == TILE_GRASS_E) shouldTurn = true;
                else if (tileBelowFront != TILE_GRASS_E && tileBelowFront != TILE_UNKNOWN_SOLID_E) shouldTurn = true;
                if (!mapData.empty() && !mapData[0].empty()) {
                     float mapEdgeRight = static_cast<float>(mapData[0].size() * tileWidth);
                     if (movingRight && (pos.x + frameWidth + MOVE_SPEED * dt > mapEdgeRight)) shouldTurn = true;
                     else if (!movingRight && (pos.x - MOVE_SPEED * dt < 0)) shouldTurn = true;
                }
                if (shouldTurn) movingRight = !movingRight;
                float moveAmount = MOVE_SPEED * dt;
                if (movingRight) pos.x += moveAmount;
                else pos.x -= moveAmount;
            }

            if (isOnGround && std::abs(MOVE_SPEED) > 0.1f) {
                animTimer += dt;
                if (animTimer >= ANIM_SPEED) {
                    animTimer -= ANIM_SPEED;
                    currentAnimFrameIndex = (currentAnimFrameIndex + 1) % NUM_FRAMES_WALK;
                }
            } else if (isOnGround) { currentAnimFrameIndex = 0; }
              else { currentAnimFrameIndex = 0; }
            break;
        }
        case EnemyState::DYING:
            velocityY = 0.0f;
            dyingTimer += dt;
            if (dyingTimer >= DYING_DURATION) {
                currentState = EnemyState::DEAD;
                isVisible = false;
            } else { isVisible = (static_cast<int>(floor(dyingTimer / BLINK_INTERVAL)) % 2 == 0); }
            break;
        case EnemyState::DEAD: break;
    }
}

void Enemy::render(RenderWindow& window, float cameraX, float cameraY) {
    if (currentState == EnemyState::DEAD || (currentState == EnemyState::DYING && !isVisible)) return;
    if (!tex) return;
    currentFrame.x = currentAnimFrameIndex * frameWidth;
    currentFrame.y = 0;
    SDL_Rect destRect = { static_cast<int>(round(pos.x - cameraX)), static_cast<int>(round(pos.y - cameraY)), frameWidth, frameHeight };
    SDL_RendererFlip flip = (!movingRight) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(window.getRenderer(), tex, &currentFrame, &destRect, 0.0, NULL, flip);
}

SDL_Rect Enemy::getWorldHitbox() const {
    SDL_Rect worldHB = { static_cast<int>(round(pos.x + hitbox.x)), static_cast<int>(round(pos.y + hitbox.y)), hitbox.w, hitbox.h };
    return worldHB;
}

void Enemy::takeHit() {
    if (currentState == EnemyState::ALIVE) {
        currentState = EnemyState::DYING;
        dyingTimer = 0.0f;
        isVisible = true;
        if (gEnemyDeathSound != nullptr) { Mix_PlayChannel(-1, gEnemyDeathSound, 0); }
        else { std::cerr << "Warning: gEnemyDeathSound is NULL in Enemy::takeHit!" << std::endl; }
    }
}

bool Enemy::isAlive() const { return currentState == EnemyState::ALIVE; }
bool Enemy::isDead() const { return currentState == EnemyState::DEAD; }
EnemyState Enemy::getState() const { return currentState; }
