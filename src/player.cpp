#include "Player.hpp"
#include "RenderWindow.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <iostream> // Giữ lại để debug nếu cần

using namespace std;

const int TILE_EMPTY = 0;
const int TILE_GRASS = 1;
const int TILE_UNKNOWN_SOLID = 2;
const int TILE_WATER_SURFACE = 3;

Player::Player(vector2d p_pos,
               SDL_Texture* p_runTex, int p_runSheetCols,
               SDL_Texture* p_jumpTex, int p_jumpSheetCols,
               SDL_Texture* p_enterWaterTex, int p_enterWaterSheetCols,
               SDL_Texture* p_swimTex, int p_swimSheetCols,
               int p_frameW, int p_frameH)
    : entity(p_pos, p_runTex, p_frameW, p_frameH, p_runSheetCols),
      runTexture(p_runTex),
      jumpTexture(p_jumpTex),
      enterWaterTexture(p_enterWaterTex),
      swimTexture(p_swimTex),
      runSheetColumns(p_runSheetCols),
      jumpSheetColumns(p_jumpSheetCols),
      enterWaterSheetColumns(p_enterWaterSheetCols),
      swimSheetColumns(p_swimSheetCols),
      frameWidth(p_frameW),
      frameHeight(p_frameH),
      velocity({0.0, 0.0}), // Giả định khởi tạo vector2d với double
      currentState(PlayerState::FALLING),
      facing(FacingDirection::RIGHT),
      isOnGround(false),
      requestDrop(false),
      isInWaterState(false),
      waterSurfaceY(0.0),
      animTimer(0.0),
      currentAnimFrameIndex(0)
{
    hitbox.x = 10;
    hitbox.y = 4;
    hitbox.w = 13;
    hitbox.h = 78;

    currentSourceRect.x = 0;
    currentSourceRect.y = 0;
    currentSourceRect.w = frameWidth;
    currentSourceRect.h = frameHeight;
}

SDL_Rect Player::getWorldHitbox() {
    SDL_Rect worldHB;
    // Ép kiểu từ double (trong getPos) sang int cho SDL_Rect
    worldHB.x = static_cast<int>(round(getPos().x + hitbox.x));
    worldHB.y = static_cast<int>(round(getPos().y + hitbox.y));
    worldHB.w = hitbox.w;
    worldHB.h = hitbox.h;
    return worldHB;
}

void Player::handleInput(const Uint8* keyStates) {
    if (isInWaterState) {
        if (keyStates[SDL_SCANCODE_LEFT]) {
            velocity.x = -MOVE_SPEED * 0.7;
            facing = FacingDirection::LEFT;
        } else if (keyStates[SDL_SCANCODE_RIGHT]) {
            velocity.x = MOVE_SPEED * 0.7;
            facing = FacingDirection::RIGHT;
        } else {
             velocity.x *= WATER_DRAG_X;
             if (abs(velocity.x) < 1.0) velocity.x = 0.0;
        }
        if (keyStates[SDL_SCANCODE_SPACE]) {
             if (velocity.y > -WATER_JUMP_STRENGTH * 0.8) {
                 velocity.y = -WATER_JUMP_STRENGTH;
             }
        }
        if (keyStates[SDL_SCANCODE_DOWN]) {
             velocity.y += 50.0;
        }
    } else {
        if (keyStates[SDL_SCANCODE_LEFT]) {
            velocity.x = -MOVE_SPEED;
            facing = FacingDirection::LEFT;
        } else if (keyStates[SDL_SCANCODE_RIGHT]) {
            velocity.x = MOVE_SPEED;
            facing = FacingDirection::RIGHT;
        } else {
            velocity.x = 0.0; // Gán double
        }
        if (keyStates[SDL_SCANCODE_SPACE] && isOnGround) {
            velocity.y = -JUMP_STRENGTH;
            isOnGround = false;
        }
    }

    requestDrop = false;
    if (!isInWaterState && keyStates[SDL_SCANCODE_DOWN] && isOnGround) {
        requestDrop = true;
    }
}

void Player::update(double dt, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) {
    applyGravity(dt);
    move(dt);
    checkMapCollision(mapData, tileWidth, tileHeight);
    updateState();
    updateAnimation(dt);

    vector2d& posRef = getPos();
    posRef.x = max(0.0, posRef.x); // max hoạt động với double
}

void Player::applyGravity(double dt) {
    if (isInWaterState) {
        velocity.y += (GRAVITY * WATER_GRAVITY_MULTIPLIER) * dt;
        velocity.y = clamp(velocity.y, -MAX_FALL_SPEED * WATER_MAX_SPEED_MULTIPLIER, MAX_FALL_SPEED * WATER_MAX_SPEED_MULTIPLIER);
    } else if (!isOnGround) {
        velocity.y += GRAVITY * dt;
        velocity.y = min(velocity.y, MAX_FALL_SPEED);
    }
}

void Player::move(double dt) {
    vector2d& posRef = getPos();
    posRef.x += velocity.x * dt;
    posRef.y += velocity.y * dt;
}

void Player::checkMapCollision(const vector<vector<int>>& mapData, int tileWidth, int tileHeight) {
    vector2d& posRef = getPos();
    SDL_Rect playerHB = getWorldHitbox(); // Hitbox có tọa độ int
    bool previouslyOnGround = isOnGround;

    isOnGround = false;

    // Sử dụng double cho tính toán vị trí thế giới
    double feetWorldY = static_cast<double>(playerHB.y) + playerHB.h;
    double headWorldY = static_cast<double>(playerHB.y);
    double midWorldX = static_cast<double>(playerHB.x) + static_cast<double>(playerHB.w) / 2.0;
    double leftWorldX = static_cast<double>(playerHB.x);
    double rightWorldX = static_cast<double>(playerHB.x) + playerHB.w;
    double midWorldY = static_cast<double>(playerHB.y) + static_cast<double>(playerHB.h) / 2.0;

    int tileBelowRow = static_cast<int>(floor(feetWorldY / tileHeight));
    int tileBelowCol = static_cast<int>(floor(midWorldX / tileWidth));
    int tileBelowType = getTileAt(midWorldX, feetWorldY, mapData, tileWidth, tileHeight);

    bool canDropTile = (getTileAt(midWorldX, feetWorldY - 1.0, mapData, tileWidth, tileHeight) == TILE_GRASS);
    bool tryingToDrop = requestDrop && previouslyOnGround && canDropTile && !isInWaterState;

    if (tryingToDrop) {
        posRef.y += 2.0; // Dùng double literal
        currentState = PlayerState::DROPPING;
    }
    else if (velocity.y >= 0 && currentState != PlayerState::DROPPING) {
        if (tileBelowType == TILE_GRASS || tileBelowType == TILE_UNKNOWN_SOLID) {
            isOnGround = true;
            if (isInWaterState) { isInWaterState = false; }
            velocity.y = 0.0;
            double newY = static_cast<double>(tileBelowRow * tileHeight) - static_cast<double>(hitbox.h) - static_cast<double>(hitbox.y);
            posRef.y = newY;
        }
        else if (tileBelowType == TILE_WATER_SURFACE && !isInWaterState) {
            isOnGround = false;
            isInWaterState = true;
            currentState = PlayerState::ENTERING_WATER;
            velocity.y *= 0.3;
            waterSurfaceY = static_cast<double>(tileBelowRow * tileHeight);
            posRef.y = waterSurfaceY - static_cast<double>(hitbox.h) - static_cast<double>(hitbox.y) + 1.0;
            currentAnimFrameIndex = 0;
        }
    }

    if (isInWaterState && velocity.y < 0) {
        if (headWorldY < waterSurfaceY) {
             int tileAboveType = getTileAt(midWorldX, headWorldY - 1.0 , mapData, tileWidth, tileHeight);
             if (tileAboveType == TILE_EMPTY || tileAboveType == TILE_WATER_SURFACE) {
                  isInWaterState = false;
             }
        }
    }

    if (!isInWaterState) {
        int tileAboveType = getTileAt(midWorldX, headWorldY, mapData, tileWidth, tileHeight);
        if (velocity.y < 0 && (tileAboveType == TILE_GRASS || tileAboveType == TILE_UNKNOWN_SOLID)) {
            velocity.y = 0.0;
            int tileRow = static_cast<int>(floor(headWorldY / tileHeight));
            double newY = static_cast<double>((tileRow + 1) * tileHeight) - static_cast<double>(hitbox.y);
            posRef.y = newY;
        }
    }

     if (!isInWaterState) {
        int tileRightType = getTileAt(rightWorldX, midWorldY, mapData, tileWidth, tileHeight);
        if (velocity.x > 0 && (tileRightType == TILE_GRASS || tileRightType == TILE_UNKNOWN_SOLID)) {
            velocity.x = 0.0;
            int tileCol = static_cast<int>(floor(rightWorldX / tileWidth));
            // Chuyển thành double hết
            double newX = static_cast<double>(tileCol * tileWidth) - static_cast<double>(hitbox.w) - static_cast<double>(hitbox.x);
            posRef.x = newX;
        }
        else {
             int tileLeftType = getTileAt(leftWorldX, midWorldY, mapData, tileWidth, tileHeight);
             if (velocity.x < 0 && (tileLeftType == TILE_GRASS || tileLeftType == TILE_UNKNOWN_SOLID)) {
                 velocity.x = 0.0;
                 int tileCol = static_cast<int>(floor(leftWorldX / tileWidth));
                 double newX = static_cast<double>((tileCol + 1) * tileWidth) - static_cast<double>(hitbox.x);
                 posRef.x = newX;
             }
        }
     }
}

int Player::getTileAt(double worldX, double worldY, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) {
    if (worldX < 0 || worldY < 0 || tileWidth <= 0 || tileHeight <= 0) return TILE_EMPTY;
    int col = static_cast<int>(floor(worldX / tileWidth));
    int row = static_cast<int>(floor(worldY / tileHeight));
    if (row >= 0 && row < mapData.size()) {
        if (col >= 0 && col < mapData[row].size()) {
            return mapData[row][col];
        }
    }
    return TILE_EMPTY;
}

void Player::updateState() {
    if (currentState == PlayerState::ENTERING_WATER) {
         if (velocity.y >= -1.0) {
             currentState = PlayerState::SWIMMING;
         }
    }

    if (isInWaterState) {
         if (currentState != PlayerState::ENTERING_WATER) {
             if (velocity.y < -10.0) {
                 currentState = PlayerState::WATER_JUMP;
             } else {
                 currentState = PlayerState::SWIMMING;
             }
         }
    } else if (isOnGround) {
        if (requestDrop && currentState != PlayerState::DROPPING) {
             // State DROPPING được set trong checkMapCollision
        }
        else if (currentState != PlayerState::DROPPING) {
             currentState = (velocity.x != 0) ? PlayerState::RUNNING : PlayerState::IDLE;
        }
    } else {
        if (currentState == PlayerState::DROPPING) {
             if (!requestDrop) {
                 currentState = PlayerState::FALLING;
             }
        }
        else {
            currentState = (velocity.y < 0) ? PlayerState::JUMPING : PlayerState::FALLING;
        }
    }
}


void Player::updateAnimation(double dt) {
    animTimer += dt;
    if (animTimer >= ANIM_SPEED) {
        animTimer -= ANIM_SPEED;

        int startFrame = 0;
        int numFrames = 1;
        int sheetCols = runSheetColumns;
        bool loopAnim = true;
        int endClampFrame = -1;

        switch (currentState) {
            case PlayerState::IDLE:
                startFrame = 0; numFrames = 1; currentAnimFrameIndex = 0; sheetCols = runSheetColumns;
                break;
            case PlayerState::RUNNING:
                startFrame = 0; numFrames = RUN_FRAMES; sheetCols = runSheetColumns;
                break;
            case PlayerState::JUMPING:
                startFrame = 0; numFrames = JUMP_FRAMES; sheetCols = jumpSheetColumns;
                loopAnim = false; endClampFrame = numFrames - 1;
                break;
            case PlayerState::FALLING:
            case PlayerState::DROPPING:
                startFrame = 0; numFrames = JUMP_FRAMES; sheetCols = jumpSheetColumns;
                loopAnim = false; endClampFrame = numFrames - 1;
                break;
            case PlayerState::ENTERING_WATER:
                startFrame = 0; numFrames = ENTER_WATER_FRAMES; sheetCols = enterWaterSheetColumns;
                loopAnim = false; endClampFrame = numFrames -1;
                break;
            case PlayerState::SWIMMING:
            case PlayerState::WATER_JUMP:
                startFrame = 0; numFrames = SWIM_FRAMES; sheetCols = swimSheetColumns;
                loopAnim = true;
                break;
        }

        if (loopAnim) {
            currentAnimFrameIndex = (currentAnimFrameIndex + 1) % numFrames;
        } else {
            // Chỉ tăng frame nếu chưa ở frame cuối cùng cần giữ lại
            if (endClampFrame == -1 || currentAnimFrameIndex < endClampFrame) {
                currentAnimFrameIndex++;
                currentAnimFrameIndex = min(currentAnimFrameIndex, numFrames - 1);
            } else {
                 currentAnimFrameIndex = endClampFrame; // Giữ ở frame cuối
            }
        }

        int frameIndexInLogic = startFrame + currentAnimFrameIndex;
        currentSourceRect.x = (frameIndexInLogic % sheetCols) * frameWidth;
        currentSourceRect.y = (frameIndexInLogic / sheetCols) * frameHeight;
        currentSourceRect.w = frameWidth;
        currentSourceRect.h = frameHeight;
    }
}

void Player::render(RenderWindow& window, double cameraX, double cameraY) {
    SDL_Texture* textureToRender = runTexture;

    switch(currentState) {
        case PlayerState::JUMPING: case PlayerState::FALLING: case PlayerState::DROPPING:
            textureToRender = jumpTexture; break;
        case PlayerState::ENTERING_WATER:
            textureToRender = enterWaterTexture; break;
        case PlayerState::SWIMMING: case PlayerState::WATER_JUMP:
            textureToRender = swimTexture; break;
        default: textureToRender = runTexture; break;
    }

    if (!textureToRender) return;

    SDL_Rect destRect;
    destRect.x = static_cast<int>(round(getPos().x - cameraX));
    destRect.y = static_cast<int>(round(getPos().y - cameraY));
    destRect.w = currentSourceRect.w;
    destRect.h = currentSourceRect.h;

    SDL_RendererFlip flip = (facing == FacingDirection::LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    SDL_RenderCopyEx(window.getRenderer(), textureToRender, &currentSourceRect, &destRect, 0.0, NULL, flip);

    #ifdef DEBUG_DRAW_HITBOX
        SDL_Renderer* renderer = window.getRenderer();
        if (renderer) {
            SDL_Rect worldHB = getWorldHitbox(); // Lấy hitbox với tọa độ int đã làm tròn
            SDL_Rect screenHB = { worldHB.x - static_cast<int>(cameraX), worldHB.y - static_cast<int>(cameraY), worldHB.w, worldHB.h};
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
            SDL_RenderFillRect(renderer, &screenHB);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }
    #endif
}