#include "Turret.hpp"
#include "utils.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

const int Turret::NUM_FRAMES_TURRET_IDLE = 1;
const int Turret::START_FRAME_TURRET_IDLE = 0;
const int Turret::NUM_FRAMES_TURRET_SHOOT = 3;
const int Turret::START_FRAME_TURRET_SHOOT = 0;
const int Turret::NUM_FRAMES_EXPLOSION = 7;


Turret::Turret(vector2d p_pos, SDL_Texture* p_turretTex,
               SDL_Texture* p_explosionTex, Mix_Chunk* p_explosionSnd, Mix_Chunk* p_shootSnd,
               SDL_Texture* p_bulletTex, int p_tileWidth, int p_tileHeight)
    : pos(p_pos),
      turretTexture(p_turretTex),
      explosionTexture(p_explosionTex),
      bulletTexture(p_bulletTex),
      explosionSound(p_explosionSnd),
      shootSound(p_shootSnd),
      currentAnimFrameIndexTurret(START_FRAME_TURRET_IDLE),
      currentAnimFrameIndexExplosion(0),
      animTimerTurret(0.0f),
      animTimerExplosion(0.0f),
      currentState(TurretState::IDLE),
      hp(8),
      detectionRadius(8.0f * p_tileWidth),
      shootCooldown(1.7f),
      currentShootTimer(shootCooldown * 0.75f)
{
    renderWidthTurret = p_tileWidth;
    renderHeightTurret = p_tileHeight;

    if (this->turretTexture)
    {
        int totalSheetWidthQuery = 0;
        SDL_QueryTexture(this->turretTexture, NULL, NULL, &totalSheetWidthQuery, &sheetFrameHeightTurret);

        sheetColsTurretAnim = std::max(NUM_FRAMES_TURRET_IDLE, NUM_FRAMES_TURRET_SHOOT);
        if (sheetColsTurretAnim == 0)
        {
            sheetColsTurretAnim = 1;
        }

        if (totalSheetWidthQuery > 0 && sheetColsTurretAnim > 0)
        {
            sheetFrameWidthTurret = totalSheetWidthQuery / sheetColsTurretAnim;
        }
        else
        {
            sheetFrameWidthTurret = renderWidthTurret;
            if (sheetFrameHeightTurret <= 0)
            {
                sheetFrameHeightTurret = renderHeightTurret;
            }
        }
        currentFrameSrcTurret = {START_FRAME_TURRET_IDLE * sheetFrameWidthTurret, 0, sheetFrameWidthTurret, sheetFrameHeightTurret};
    }
    else
    {
        sheetFrameWidthTurret = renderWidthTurret;
        sheetFrameHeightTurret = renderHeightTurret;
        sheetColsTurretAnim = 1;
        currentFrameSrcTurret = {0, 0, 0, 0};
    }

    if (this->explosionTexture)
    {
        int totalWidthExplQuery = 0;
        SDL_QueryTexture(this->explosionTexture, NULL, NULL, &totalWidthExplQuery, &sheetFrameHeightExplosion);
        sheetColsExplosion = NUM_FRAMES_EXPLOSION;
        if (sheetColsExplosion > 0 && totalWidthExplQuery > 0)
        {
            sheetFrameWidthExplosion = totalWidthExplQuery / sheetColsExplosion;
        }
        else
        {
            sheetFrameWidthExplosion = renderWidthTurret;
            if (sheetFrameHeightExplosion <= 0)
            {
                sheetFrameHeightExplosion = renderHeightTurret;
            }
            sheetColsExplosion = 1;
        }
        currentFrameSrcExplosion = {0, 0, sheetFrameWidthExplosion, sheetFrameHeightExplosion};
    }
    else
    {
        sheetFrameWidthExplosion = renderWidthTurret;
        sheetFrameHeightExplosion = renderHeightTurret;
        sheetColsExplosion = 1;
        currentFrameSrcExplosion = {0, 0, 0, 0};
    }

    hitbox.x = 0;
    hitbox.y = 0;
    hitbox.w = renderWidthTurret;
    hitbox.h = renderHeightTurret;
}

void Turret::update(float dt, Player* player, std::list<Bullet>& enemyBullets) {
    if (currentState == TurretState::FULLY_DESTROYED)
    {
        return;
    }

    if (currentState == TurretState::DESTROYED_ANIM)
    {
        if (this->explosionTexture)
        {
            animTimerExplosion += dt;
            if (animTimerExplosion >= ANIM_SPEED_EXPLOSION)
            {
                animTimerExplosion -= ANIM_SPEED_EXPLOSION;
                currentAnimFrameIndexExplosion++;
                if (currentAnimFrameIndexExplosion >= NUM_FRAMES_EXPLOSION)
                {
                    currentState = TurretState::FULLY_DESTROYED;
                    currentAnimFrameIndexExplosion = NUM_FRAMES_EXPLOSION - 1;
                }
            }
            currentFrameSrcExplosion.x = currentAnimFrameIndexExplosion * sheetFrameWidthExplosion;
            currentFrameSrcExplosion.y = 0;
            currentFrameSrcExplosion.w = sheetFrameWidthExplosion;
            currentFrameSrcExplosion.h = sheetFrameHeightExplosion;
        }
        else
        {
            currentState = TurretState::FULLY_DESTROYED;
        }
        if (currentState == TurretState::FULLY_DESTROYED) {
            return;
        }
        return;
    }

    bool playerInRangeAndVisible = false;

    currentShootTimer -= dt;

    if (player && !player->getIsDead() && !player->isInvulnerable())
    {
        SDL_Rect playerHb = player->getWorldHitbox();
        vector2d playerCenter = {static_cast<float>(playerHb.x + playerHb.w / 2.0f), static_cast<float>(playerHb.y + playerHb.h / 2.0f)};
        vector2d turretCenter = {pos.x + static_cast<float>(renderWidthTurret) / 2.0f, pos.y + static_cast<float>(renderHeightTurret) / 2.0f};
        if (utils::distance(playerCenter, turretCenter) <= detectionRadius)
        {
            playerInRangeAndVisible = true;
        }
    }

    if (!turretTexture && currentState == TurretState::IDLE)
    {
        if (playerInRangeAndVisible && currentShootTimer <= 0.0f && this->bulletTexture)
        {
            shootAtPlayer(player, enemyBullets);
            currentShootTimer = shootCooldown;
        }
    }

    if (currentState == TurretState::SHOOTING)
    {
        if (this->turretTexture)
        {
            animTimerTurret += dt;
            if (animTimerTurret >= ANIM_SPEED_TURRET_SHOOT)
            {
                animTimerTurret -= ANIM_SPEED_TURRET_SHOOT;
                currentAnimFrameIndexTurret++;
                if (currentAnimFrameIndexTurret >= START_FRAME_TURRET_SHOOT + NUM_FRAMES_TURRET_SHOOT)
                {
                    currentAnimFrameIndexTurret = START_FRAME_TURRET_IDLE;
                    currentState = TurretState::IDLE;
                }
            }
        }
        else
        {
            currentState = TurretState::IDLE;
        }
    }
    else
    {
        if (this->turretTexture)
        {
            if (NUM_FRAMES_TURRET_IDLE > 1)
            {
                animTimerTurret += dt;
                if (animTimerTurret >= ANIM_SPEED_TURRET_IDLE)
                {
                    animTimerTurret -= ANIM_SPEED_TURRET_IDLE;
                    int relativeFrame = (currentAnimFrameIndexTurret - START_FRAME_TURRET_IDLE + 1) % NUM_FRAMES_TURRET_IDLE;
                    currentAnimFrameIndexTurret = START_FRAME_TURRET_IDLE + relativeFrame;
                }
            }
            else
            {
                currentAnimFrameIndexTurret = START_FRAME_TURRET_IDLE;
            }
        }

        if (playerInRangeAndVisible && currentShootTimer <= 0.0f && this->bulletTexture)
        {
            shootAtPlayer(player, enemyBullets);
            currentShootTimer = shootCooldown;
            if (NUM_FRAMES_TURRET_SHOOT > 0 && this->turretTexture)
            {
                currentState = TurretState::SHOOTING;
                currentAnimFrameIndexTurret = START_FRAME_TURRET_SHOOT;
                animTimerTurret = 0.0f;
            }
        }
    }

    if (this->turretTexture)
    {
        currentFrameSrcTurret.x = currentAnimFrameIndexTurret * sheetFrameWidthTurret;
        currentFrameSrcTurret.y = 0;
        currentFrameSrcTurret.w = sheetFrameWidthTurret;
        currentFrameSrcTurret.h = sheetFrameHeightTurret;
    }
}


void Turret::shootAtPlayer(Player* player, std::list<Bullet>& enemyBullets) {
    if (!this->bulletTexture || !player)
    {
        return;
    }

    vector2d turretCenter = {
        pos.x + static_cast<float>(renderWidthTurret) / 2.0f,
        pos.y + static_cast<float>(renderHeightTurret) / 2.0f
    };

    const int turretBulletRenderW = 16;
    const int turretBulletRenderH = 16;

    vector2d bulletTopLeftSpawnPos = {
        turretCenter.x - static_cast<float>(turretBulletRenderW) / 2.0f,
        turretCenter.y - static_cast<float>(turretBulletRenderH) / 2.0f
    };

    SDL_Rect playerHb = player->getWorldHitbox();
    vector2d playerCenter = {
        static_cast<float>(playerHb.x + playerHb.w / 2.0f),
        static_cast<float>(playerHb.y + playerHb.h / 2.0f)
    };

    float dx = playerCenter.x - turretCenter.x;
    float dy = playerCenter.y - turretCenter.y;
    vector2d bulletVel = {0.0f, 0.0f};

    if (std::abs(dx) < 5.0f && std::abs(dy) < 5.0f)
    {
        bulletVel.x = (dx >= 0) ? TURRET_BULLET_SPEED : -TURRET_BULLET_SPEED;
        if (std::abs(dx) < 1.0f && dx < 0)
        {
            bulletVel.x = -TURRET_BULLET_SPEED;
        }
        else if (std::abs(dx) < 1.0f && dx >= 0)
        {
            bulletVel.x = TURRET_BULLET_SPEED;
        }
        bulletVel.y = 0;
    }
    else
    {
        float angle = atan2(dy, dx);
        const float PI = 3.1415926535f;
        const float PI_8 = PI / 8.0f;

        if (angle >= -PI_8 && angle < PI_8)
        {
            bulletVel.x = TURRET_BULLET_SPEED;
            bulletVel.y = 0;
        }
        else if (angle >= PI_8 && angle < 3 * PI_8)
        {
            bulletVel.x = TURRET_DIAGONAL_SPEED_COMPONENT;
            bulletVel.y = TURRET_DIAGONAL_SPEED_COMPONENT;
        }
        else if (angle >= 3 * PI_8 && angle < 5 * PI_8)
        {
            bulletVel.x = 0;
            bulletVel.y = TURRET_BULLET_SPEED;
        }
        else if (angle >= 5 * PI_8 && angle < 7 * PI_8)
        {
            bulletVel.x = -TURRET_DIAGONAL_SPEED_COMPONENT;
            bulletVel.y = TURRET_DIAGONAL_SPEED_COMPONENT;
        }
        else if (angle >= 7 * PI_8 || angle < -7 * PI_8)
        {
            bulletVel.x = -TURRET_BULLET_SPEED;
            bulletVel.y = 0;
        }
        else if (angle >= -7 * PI_8 && angle < -5 * PI_8)
        {
            bulletVel.x = -TURRET_DIAGONAL_SPEED_COMPONENT;
            bulletVel.y = -TURRET_DIAGONAL_SPEED_COMPONENT;
        }
        else if (angle >= -5 * PI_8 && angle < -3 * PI_8)
        {
            bulletVel.x = 0;
            bulletVel.y = -TURRET_BULLET_SPEED;
        }
        else
        {
            bulletVel.x = TURRET_DIAGONAL_SPEED_COMPONENT;
            bulletVel.y = -TURRET_DIAGONAL_SPEED_COMPONENT;
        }
    }

    enemyBullets.emplace_back(bulletTopLeftSpawnPos, bulletVel, this->bulletTexture,
                              turretBulletRenderW, turretBulletRenderH);
    if (this->shootSound)
    {
        Mix_PlayChannel(-1, this->shootSound, 0);
    }
}

void Turret::takeDamage() {
    if (currentState == TurretState::DESTROYED_ANIM || currentState == TurretState::FULLY_DESTROYED)
    {
        return;
    }
    hp--;
    if (hp <= 0)
    {
        hp = 0;
        if (this->explosionTexture)
        {
            currentState = TurretState::DESTROYED_ANIM;
            animTimerExplosion = 0.0f;
            currentAnimFrameIndexExplosion = 0;
            if (this->explosionSound)
            {
                Mix_PlayChannel(-1, this->explosionSound, 0);
            }
        }
        else
        {
            currentState = TurretState::FULLY_DESTROYED;
        }
    }
}

SDL_Rect Turret::getWorldHitbox() const {
    SDL_Rect worldHB = {
        static_cast<int>(round(pos.x + hitbox.x)),
        static_cast<int>(round(pos.y + hitbox.y)),
        hitbox.w,
        hitbox.h
    };
    return worldHB;
}

bool Turret::isFullyDestroyed() const {
    return currentState == TurretState::FULLY_DESTROYED;
}

void Turret::render(RenderWindow& window, float cameraX, float cameraY) {
    SDL_Rect destRect;

    // Sửa đổi điều kiện render animation nổ:
    // Chỉ render animation nổ nếu đang trong DESTROYED_ANIM HOẶC nếu là FULLY_DESTROYED nhưng frame nổ chưa phải là frame cuối cùng
    // (để đảm bảo frame cuối của vụ nổ được render một lần trước khi isFullyDestroyed() trả về true hoàn toàn).
    if (currentState == TurretState::DESTROYED_ANIM ||
        (currentState == TurretState::FULLY_DESTROYED && this->explosionTexture && currentAnimFrameIndexExplosion < NUM_FRAMES_EXPLOSION -1 )) { // Chú ý: < NUM_FRAMES_EXPLOSION -1
        if (this->explosionTexture)
        {
            float explosionRenderWidth = static_cast<float>(currentFrameSrcExplosion.w);
            float explosionRenderHeight = static_cast<float>(currentFrameSrcExplosion.h);
            destRect = {
                static_cast<int>(round(pos.x - cameraX + (renderWidthTurret - explosionRenderWidth) / 2.0f)),
                static_cast<int>(round(pos.y - cameraY + (renderHeightTurret - explosionRenderHeight) / 2.0f)),
                static_cast<int>(round(explosionRenderWidth)),
                static_cast<int>(round(explosionRenderHeight))
            };
            SDL_RenderCopy(window.getRenderer(), this->explosionTexture, &currentFrameSrcExplosion, &destRect);
        }
    }
    else if (currentState != TurretState::FULLY_DESTROYED && this->turretTexture)
    {
        destRect = {
            static_cast<int>(round(pos.x - cameraX)),
            static_cast<int>(round(pos.y - cameraY)),
            renderWidthTurret,
            renderHeightTurret
        };
        SDL_RenderCopy(window.getRenderer(), this->turretTexture, &currentFrameSrcTurret, &destRect);
    }
}

void Turret::setHp(int newHp) {
    hp = newHp;
}

void Turret::forceExplode() {
    if (currentState != TurretState::FULLY_DESTROYED && currentState != TurretState::DESTROYED_ANIM)
    {
        hp = 0; // Đặt HP về 0 để logic takeDamage (nếu có) không can thiệp
        if (this->explosionTexture)
        {
            currentState = TurretState::DESTROYED_ANIM; // Chuyển sang trạng thái nổ
            animTimerExplosion = 0.0f;               // Reset timer animation nổ
            currentAnimFrameIndexExplosion = 0;      // Bắt đầu từ frame đầu tiên của animation nổ
            // Âm thanh nổ của boss part thường sẽ do Boss quản lý, không phát ở đây trừ khi có yêu cầu riêng
        }
        else
        {
            currentState = TurretState::FULLY_DESTROYED; // Nếu không có animation nổ, coi như bị phá hủy ngay
        }
    }
}