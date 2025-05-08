#include "Turret.hpp"
#include "utils.hpp" // Cho utils::distance
#include <cmath>     // Cho sqrt, abs, copysignf
#include <iostream>
#include <algorithm> // Cho std::max

// <<< THÊM ĐỊNH NGHĨA CHO CÁC BIẾN STATIC CONST INT NGOÀI LỚP >>>
const int Turret::NUM_FRAMES_TURRET_IDLE = 1;
const int Turret::START_FRAME_TURRET_IDLE = 0;
const int Turret::NUM_FRAMES_TURRET_SHOOT = 3;
const int Turret::START_FRAME_TURRET_SHOOT = 0; // Giả sử anim bắn bắt đầu từ frame 0
const int Turret::NUM_FRAMES_EXPLOSION = 8; // Định nghĩa luôn ở đây


// --- Constructor ---
Turret::Turret(vector2d p_pos, SDL_Texture* p_turretTex,
               SDL_Texture* p_explosionTex, Mix_Chunk* p_explosionSnd, Mix_Chunk* p_shootSnd,
               SDL_Texture* p_bulletTex, int p_tileWidth, int p_tileHeight)
    : // Initializer list theo đúng thứ tự khai báo trong .hpp
      pos(p_pos),
      turretTexture(p_turretTex), explosionTexture(p_explosionTex), bulletTexture(p_bulletTex),
      explosionSound(p_explosionSnd), shootSound(p_shootSnd),
      // currentFrameTurret, currentFrameExplosion khởi tạo sau width/height
      // frameWidth/Height, sheetCols tính toán bên dưới
      currentAnimFrameIndexTurret(START_FRAME_TURRET_IDLE), // Sử dụng const static
      currentAnimFrameIndexExplosion(0),
      animTimerTurret(0.0f), animTimerExplosion(0.0f),
      currentState(TurretState::IDLE),
      // hitbox khởi tạo sau width/height
      hp(8), detectionRadius(8.0f * p_tileWidth), shootCooldown(1.7f),
      currentShootTimer(shootCooldown) // Khởi tạo timer sau const
{
    // Tính toán frameWidth/Height và khởi tạo currentFrame/hitbox...
    if (turretTexture) {
        int totalWidth; SDL_QueryTexture(turretTexture, NULL, NULL, &totalWidth, &frameHeightTurret);
        // Sử dụng các hằng số đã định nghĩa
        sheetColsTurretAnim = std::max(NUM_FRAMES_TURRET_IDLE, NUM_FRAMES_TURRET_SHOOT);
        if (sheetColsTurretAnim == 0) sheetColsTurretAnim = 1;
        if (totalWidth > 0 && sheetColsTurretAnim > 0) frameWidthTurret = totalWidth / sheetColsTurretAnim;
        else {frameWidthTurret = p_tileWidth; sheetColsTurretAnim = 1; std::cerr << "Warning: Turret texture width or sheetColsTurretAnim is invalid. Using tile size." << std::endl;}
        currentFrameTurret = {START_FRAME_TURRET_IDLE * frameWidthTurret, 0, frameWidthTurret, frameHeightTurret};
    } else { frameWidthTurret = p_tileWidth; frameHeightTurret = p_tileHeight; sheetColsTurretAnim = 1; currentFrameTurret = {0,0, frameWidthTurret, frameHeightTurret}; std::cerr << "Warning: Turret turretTexture is NULL!" << std::endl;}

    if (explosionTexture) {
        int totalWidthExpl; SDL_QueryTexture(explosionTexture, NULL, NULL, &totalWidthExpl, &frameHeightExplosion);
        sheetColsExplosion = NUM_FRAMES_EXPLOSION; // Giả sử NUM_FRAMES_EXPLOSION không gây lỗi
        if (sheetColsExplosion > 0) frameWidthExplosion = totalWidthExpl / sheetColsExplosion;
        else { frameWidthExplosion = totalWidthExpl; sheetColsExplosion = 1;}
        currentFrameExplosion = {0, 0, frameWidthExplosion, frameHeightExplosion};
    } else { frameWidthExplosion = p_tileWidth; frameHeightExplosion = p_tileHeight; sheetColsExplosion=1; std::cerr << "Warning: Turret explosionTexture is NULL!" << std::endl; }

    hitbox.w = frameWidthTurret - 4; hitbox.h = frameHeightTurret - 4;
    hitbox.x = (frameWidthTurret - hitbox.w) / 2; hitbox.y = (frameHeightTurret - hitbox.h) / 2;
}


// --- Định nghĩa các hàm thành viên khác ---

void Turret::update(float dt, Player* player, std::list<Bullet>& enemyBullets) {
    if (currentState == TurretState::FULLY_DESTROYED) return;

    if (currentState == TurretState::DESTROYED_ANIM) {
        animTimerExplosion += dt;
        if (animTimerExplosion >= ANIM_SPEED_EXPLOSION) {
            animTimerExplosion -= ANIM_SPEED_EXPLOSION;
            currentAnimFrameIndexExplosion++;
            if (currentAnimFrameIndexExplosion >= NUM_FRAMES_EXPLOSION) {
                currentState = TurretState::FULLY_DESTROYED;
                currentAnimFrameIndexExplosion = NUM_FRAMES_EXPLOSION - 1;
            }
        }
        currentFrameExplosion.x = currentAnimFrameIndexExplosion * frameWidthExplosion;
        return;
    }

    currentShootTimer -= dt;
    bool playerInRangeAndVisible = false;

    if (player && !player->getIsDead() && !player->isInvulnerable()) {
        SDL_Rect playerHb = player->getWorldHitbox();
        vector2d playerCenter = { static_cast<float>(playerHb.x + playerHb.w / 2.0f), static_cast<float>(playerHb.y + playerHb.h / 2.0f) };
        vector2d turretCenter = {pos.x + frameWidthTurret / 2.0f, pos.y + frameHeightTurret / 2.0f};
        if (utils::distance(playerCenter, turretCenter) <= detectionRadius) {
            playerInRangeAndVisible = true;
        }
    }

    if (currentState == TurretState::SHOOTING) {
        animTimerTurret += dt;
        if (animTimerTurret >= ANIM_SPEED_TURRET_SHOOT) {
            animTimerTurret -= ANIM_SPEED_TURRET_SHOOT;
            currentAnimFrameIndexTurret++;
            if (currentAnimFrameIndexTurret >= START_FRAME_TURRET_SHOOT + NUM_FRAMES_TURRET_SHOOT) {
                currentAnimFrameIndexTurret = START_FRAME_TURRET_IDLE;
                currentState = TurretState::IDLE;
            }
        }
    } else { // IDLE state
        if (NUM_FRAMES_TURRET_IDLE > 1) { // Nếu có animation idle
             animTimerTurret += dt;
            if (animTimerTurret >= ANIM_SPEED_TURRET_IDLE) {
                animTimerTurret -= ANIM_SPEED_TURRET_IDLE;
                int relativeFrame = (currentAnimFrameIndexTurret - START_FRAME_TURRET_IDLE + 1) % NUM_FRAMES_TURRET_IDLE;
                currentAnimFrameIndexTurret = START_FRAME_TURRET_IDLE + relativeFrame;
            }
        } else {
            currentAnimFrameIndexTurret = START_FRAME_TURRET_IDLE; // Giữ frame idle
        }

        if (playerInRangeAndVisible && currentShootTimer <= 0.0f) {
            shootAtPlayer(player, enemyBullets);
            currentShootTimer = shootCooldown;
            if (NUM_FRAMES_TURRET_SHOOT > 0) { // Chỉ chuyển state nếu có animation bắn
                 currentState = TurretState::SHOOTING;
                 currentAnimFrameIndexTurret = START_FRAME_TURRET_SHOOT; // Bắt đầu anim bắn
                 animTimerTurret = 0.0f; // Reset timer cho anim bắn
            }
        }
    }
    // Cập nhật source rect cho turret dựa trên frame animation hiện tại
    currentFrameTurret.x = currentAnimFrameIndexTurret * frameWidthTurret;
}

void Turret::shootAtPlayer(Player* player, std::list<Bullet>& enemyBullets) {
    if (!this->bulletTexture || !player) return;

    vector2d turretBarrelTip = {pos.x + frameWidthTurret / 2.0f, pos.y + frameHeightTurret * 0.25f};
    SDL_Rect playerHb = player->getWorldHitbox();
    vector2d playerCenter = { static_cast<float>(playerHb.x + playerHb.w / 2.0f), static_cast<float>(playerHb.y + playerHb.h / 2.0f) };
    float dx = playerCenter.x - turretBarrelTip.x;
    float dy = playerCenter.y - turretBarrelTip.y;
    vector2d bulletVel = {0.0f, 0.0f};
    const float epsilon = 0.1f;

    if (std::abs(dx) < epsilon && std::abs(dy) < epsilon) {
        bulletVel.x = TURRET_BULLET_SPEED; // Sử dụng const static
        bulletVel.y = 0.0f;
    } else {
        float absDx = std::abs(dx); float absDy = std::abs(dy);
        const float diagonalThreshold = 0.414f; // tan(22.5 deg)

        if (absDy < absDx * diagonalThreshold) { // Gần ngang hơn
            bulletVel.x = std::copysign(TURRET_BULLET_SPEED, dx); // Sử dụng const static
            bulletVel.y = 0.0f;
        } else { // Gần chéo 45 độ hơn
            bulletVel.x = std::copysign(TURRET_DIAGONAL_SPEED_COMPONENT, dx); // Sử dụng const static
            bulletVel.y = std::copysign(TURRET_DIAGONAL_SPEED_COMPONENT, dy); // Sử dụng const static
        }
    }

    enemyBullets.emplace_back(turretBarrelTip, bulletVel, this->bulletTexture);
    if (shootSound) Mix_PlayChannel(-1, shootSound, 0);
}

void Turret::takeDamage() {
    if (currentState == TurretState::DESTROYED_ANIM || currentState == TurretState::FULLY_DESTROYED) return;
    hp--;
    if (hp <= 0) {
        currentState = TurretState::DESTROYED_ANIM;
        animTimerExplosion = 0.0f;
        currentAnimFrameIndexExplosion = 0;
        if (explosionSound) Mix_PlayChannel(-1, explosionSound, 0);
    }
}

SDL_Rect Turret::getWorldHitbox() const {
    SDL_Rect worldHB = { static_cast<int>(round(pos.x + hitbox.x)), static_cast<int>(round(pos.y + hitbox.y)), hitbox.w, hitbox.h };
    return worldHB;
}

bool Turret::isFullyDestroyed() const { return currentState == TurretState::FULLY_DESTROYED; }

void Turret::render(RenderWindow& window, float cameraX, float cameraY) {
    if (currentState == TurretState::FULLY_DESTROYED && currentState != TurretState::DESTROYED_ANIM) return;
    SDL_Rect destRect;
    if (currentState == TurretState::DESTROYED_ANIM) {
        if (!explosionTexture) return;
        // currentFrameExplosion.x đã được cập nhật trong update()
        destRect = { static_cast<int>(round(pos.x-cameraX+(frameWidthTurret-frameWidthExplosion)/2.0f)), static_cast<int>(round(pos.y-cameraY+(frameHeightTurret-frameHeightExplosion)/2.0f)), frameWidthExplosion, frameHeightExplosion };
        SDL_RenderCopy(window.getRenderer(), explosionTexture, &currentFrameExplosion, &destRect);
    } else { // Vẽ Turret (IDLE hoặc SHOOTING)
        if (turretTexture) {
            // currentFrameTurret.x đã được cập nhật trong update()
            destRect = { static_cast<int>(round(pos.x - cameraX)), static_cast<int>(round(pos.y - cameraY)), frameWidthTurret, frameHeightTurret };
            SDL_RenderCopy(window.getRenderer(), turretTexture, &currentFrameTurret, &destRect);
        }
    }

    #ifdef DEBUG_DRAW_HITBOXES // Cần #define DEBUG_DRAW_HITBOXES ở đầu main.cpp hoặc file config
    if (currentState != TurretState::DESTROYED_ANIM && currentState != TurretState::FULLY_DESTROYED) {
        SDL_SetRenderDrawColor(window.getRenderer(), 255, 0, 255, 100); // Màu tím cho hitbox turret
        SDL_Rect debugHitbox = getWorldHitbox();
        debugHitbox.x = static_cast<int>(round(debugHitbox.x - cameraX));
        debugHitbox.y = static_cast<int>(round(debugHitbox.y - cameraY));
        SDL_RenderDrawRect(window.getRenderer(), &debugHitbox);
    }
    #endif
}