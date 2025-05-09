#include "Turret.hpp"
#include "utils.hpp" 
#include <cmath>     
#include <iostream>
#include <algorithm> 

// --- Static const int definitions ---
const int Turret::NUM_FRAMES_TURRET_IDLE = 1;
const int Turret::START_FRAME_TURRET_IDLE = 0;
const int Turret::NUM_FRAMES_TURRET_SHOOT = 3;
const int Turret::START_FRAME_TURRET_SHOOT = 0;
const int Turret::NUM_FRAMES_EXPLOSION = 7;


// --- Constructor ---
Turret::Turret(vector2d p_pos, SDL_Texture* p_turretTex,
               SDL_Texture* p_explosionTex, Mix_Chunk* p_explosionSnd, Mix_Chunk* p_shootSnd,
               SDL_Texture* p_bulletTex, int p_tileWidth, int p_tileHeight)
    : pos(p_pos),
      turretTexture(p_turretTex), explosionTexture(p_explosionTex), bulletTexture(p_bulletTex),
      explosionSound(p_explosionSnd), shootSound(p_shootSnd),
      currentAnimFrameIndexTurret(START_FRAME_TURRET_IDLE),
      currentAnimFrameIndexExplosion(0),
      animTimerTurret(0.0f), animTimerExplosion(0.0f),
      currentState(TurretState::IDLE),
      hp(8), detectionRadius(8.0f * p_tileWidth), shootCooldown(1.7f),
      currentShootTimer(shootCooldown)
{
    renderWidthTurret = p_tileWidth;
    renderHeightTurret = p_tileHeight;

    if (turretTexture) {
        int totalSheetWidth;
        SDL_QueryTexture(turretTexture, NULL, NULL, &totalSheetWidth, &sheetFrameHeightTurret);
        // Sửa: sheetColsTurretAnim nên dựa trên tổng số frame có thể có trong animation dài nhất
        // Ví dụ, nếu idle có 1 frame (index 0), shoot có 3 frame (index 0,1,2)
        // thì tổng số frame là 3 (từ 0 đến 2) nếu chúng nằm liên tiếp và bắt đầu từ 0.
        // Hoặc nếu chúng là các dãy riêng biệt, cần biết tổng số cột của spritesheet.
        // Giả sử các animation nằm trong cùng một hàng và tổng số cột được cung cấp hoặc tính toán
        // Dựa trên code cũ, có vẻ sheetColsTurretAnim là số frame của animation dài nhất.
        sheetColsTurretAnim = std::max(NUM_FRAMES_TURRET_IDLE, NUM_FRAMES_TURRET_SHOOT);
        if (sheetColsTurretAnim == 0) sheetColsTurretAnim = 1;


        if (totalSheetWidth > 0 && sheetColsTurretAnim > 0) {
            // Giả sử totalSheetWidth là chiều rộng của TOÀN BỘ spritesheet chứa TẤT CẢ các frame
            // Và sheetColsTurretAnim là TỔNG SỐ CỘT frame trên spritesheet đó.
            // Nếu START_FRAME_TURRET_SHOOT không phải là 0 và các frame không liên tục, cách tính này cần xem lại.
            // Với logic hiện tại (START_FRAME_TURRET_IDLE = 0, START_FRAME_TURRET_SHOOT = 0),
            // sheetFrameWidthTurret = totalSheetWidth / (tổng số frame thực tế trên sheet);
            // Nếu sheet chỉ chứa các frame của một anim, ví dụ shoot (3 frame), thì là totalSheetWidth / 3
            // Tạm thời giữ nguyên logic cũ dựa trên NUM_FRAMES...
            // Nếu spritesheet của bạn có N cột frame, thì sheetFrameWidthTurret = totalSheetWidth / N;
            // Và sheetColsTurretAnim lúc đó sẽ là N.
            // Để đơn giản, nếu spritesheet được thiết kế sao cho mỗi anim có số frame = số cột, thì cách tính này ổn.
            sheetFrameWidthTurret = totalSheetWidth / sheetColsTurretAnim;

        } else {
            sheetFrameWidthTurret = renderWidthTurret; // Fallback
            // sheetFrameHeightTurret đã được query ở trên
            sheetColsTurretAnim = 1; // Fallback
            std::cerr << "Warning: Turret texture width or sheetColsTurretAnim is invalid. Using tile size for sheet frame." << std::endl;
        }
        currentFrameSrcTurret = {START_FRAME_TURRET_IDLE * sheetFrameWidthTurret, 0, sheetFrameWidthTurret, sheetFrameHeightTurret};
    } else {
        sheetFrameWidthTurret = renderWidthTurret;
        sheetFrameHeightTurret = renderHeightTurret;
        sheetColsTurretAnim = 1;
        currentFrameSrcTurret = {0,0, sheetFrameWidthTurret, sheetFrameHeightTurret};
        std::cerr << "Warning: Turret turretTexture is NULL!" << std::endl;
    }

    if (explosionTexture) {
        int totalWidthExpl;
        SDL_QueryTexture(explosionTexture, NULL, NULL, &totalWidthExpl, &sheetFrameHeightExplosion);
        sheetColsExplosion = NUM_FRAMES_EXPLOSION;
        if (sheetColsExplosion > 0) {
            sheetFrameWidthExplosion = totalWidthExpl / sheetColsExplosion;
        } else {
            sheetFrameWidthExplosion = totalWidthExpl; // Nếu NUM_FRAMES_EXPLOSION là 0 hoặc 1
            sheetColsExplosion = 1;
        }
        currentFrameSrcExplosion = {0, 0, sheetFrameWidthExplosion, sheetFrameHeightExplosion};
    } else {
        sheetFrameWidthExplosion = renderWidthTurret; // Fallback
        sheetFrameHeightExplosion = renderHeightTurret; // Fallback
        sheetColsExplosion = 1; // Fallback
        currentFrameSrcExplosion = {0, 0, sheetFrameWidthExplosion, sheetFrameHeightExplosion};
        std::cerr << "Warning: Turret explosionTexture is NULL!" << std::endl;
    }

    // Hitbox được đặt ở (0,0) tương đối so với pos của Turret
    // và có kích thước bằng kích thước render của Turret
    hitbox.w = renderWidthTurret;
    hitbox.h = renderHeightTurret;
    hitbox.x = 0; 
    hitbox.y = 0; 
}

// --- Update Method ---
void Turret::update(float dt, Player* player, std::list<Bullet>& enemyBullets) {
    if (currentState == TurretState::FULLY_DESTROYED) return;

    if (currentState == TurretState::DESTROYED_ANIM) {
        animTimerExplosion += dt;
        if (animTimerExplosion >= ANIM_SPEED_EXPLOSION) {
            animTimerExplosion -= ANIM_SPEED_EXPLOSION;
            currentAnimFrameIndexExplosion++;
            if (currentAnimFrameIndexExplosion >= NUM_FRAMES_EXPLOSION) {
                currentState = TurretState::FULLY_DESTROYED;
                currentAnimFrameIndexExplosion = NUM_FRAMES_EXPLOSION - 1; // Giữ ở frame cuối
            }
        }
        if(explosionTexture) { // Chỉ cập nhật source rect nếu có texture
            currentFrameSrcExplosion.x = currentAnimFrameIndexExplosion * sheetFrameWidthExplosion;
        }
        return;
    }

    currentShootTimer -= dt;
    bool playerInRangeAndVisible = false;

    if (player && !player->getIsDead() && !player->isInvulnerable()) {
        SDL_Rect playerHb = player->getWorldHitbox(); 
        vector2d playerCenter = { static_cast<float>(playerHb.x + playerHb.w / 2.0f), static_cast<float>(playerHb.y + playerHb.h / 2.0f) };
        vector2d turretCenter = {pos.x + renderWidthTurret / 2.0f, pos.y + renderHeightTurret / 2.0f};
        if (utils::distance(playerCenter, turretCenter) <= detectionRadius) {
            playerInRangeAndVisible = true;
        }
    }

    if (currentState == TurretState::SHOOTING) {
        animTimerTurret += dt;
        if (animTimerTurret >= ANIM_SPEED_TURRET_SHOOT) {
            animTimerTurret -= ANIM_SPEED_TURRET_SHOOT;
            currentAnimFrameIndexTurret++;
            // Kiểm tra nếu animation bắn đã hoàn thành (vượt qua số frame của shoot anim)
            if (currentAnimFrameIndexTurret >= START_FRAME_TURRET_SHOOT + NUM_FRAMES_TURRET_SHOOT) {
                currentAnimFrameIndexTurret = START_FRAME_TURRET_IDLE; // Quay lại frame idle đầu tiên
                currentState = TurretState::IDLE;
            }
        }
    } else { // IDLE state
        if (NUM_FRAMES_TURRET_IDLE > 1) { // Chỉ animate idle nếu có nhiều hơn 1 frame
             animTimerTurret += dt;
            if (animTimerTurret >= ANIM_SPEED_TURRET_IDLE) {
                animTimerTurret -= ANIM_SPEED_TURRET_IDLE;
                int relativeFrame = (currentAnimFrameIndexTurret - START_FRAME_TURRET_IDLE + 1) % NUM_FRAMES_TURRET_IDLE;
                currentAnimFrameIndexTurret = START_FRAME_TURRET_IDLE + relativeFrame;
            }
        } else { // Nếu idle chỉ có 1 frame
            currentAnimFrameIndexTurret = START_FRAME_TURRET_IDLE;
        }

        if (playerInRangeAndVisible && currentShootTimer <= 0.0f) {
            shootAtPlayer(player, enemyBullets);
            currentShootTimer = shootCooldown; // Reset cooldown
            if (NUM_FRAMES_TURRET_SHOOT > 0) { // Chỉ chuyển sang SHOOTING nếu có animation bắn
                 currentState = TurretState::SHOOTING;
                 currentAnimFrameIndexTurret = START_FRAME_TURRET_SHOOT; // Bắt đầu từ frame đầu của shoot anim
                 animTimerTurret = 0.0f; // Reset timer cho shoot anim
            }
        }
    }
    if (turretTexture) { // Chỉ cập nhật source rect nếu có texture
        currentFrameSrcTurret.x = currentAnimFrameIndexTurret * sheetFrameWidthTurret;
    }
}

// --- ShootAtPlayer Method ---
void Turret::shootAtPlayer(Player* player, std::list<Bullet>& enemyBullets) {
    if (!this->bulletTexture || !player) return;

    vector2d turretCenter = {
        pos.x + renderWidthTurret / 2.0f,
        pos.y + renderHeightTurret / 2.0f
    };

    // ĐỊNH NGHĨA KÍCH THƯỚC RENDER CHO ĐẠN CỦA TURRET
    const int turretBulletRenderW = 16; // Ví dụ: kích thước đạn turret
    const int turretBulletRenderH = 16; // Ví dụ: kích thước đạn turret

    // Tính toán vị trí góc trên trái của viên đạn để tâm của nó ở turretCenter
    vector2d bulletTopLeftSpawnPos = {
        turretCenter.x - static_cast<float>(turretBulletRenderW) / 2.0f,
        turretCenter.y - static_cast<float>(turretBulletRenderH) / 2.0f
    };
    
    SDL_Rect playerHb = player->getWorldHitbox(); 
    vector2d playerCenter = { static_cast<float>(playerHb.x + playerHb.w / 2.0f), static_cast<float>(playerHb.y + playerHb.h / 2.0f) };
    
    float dx = playerCenter.x - turretCenter.x;
    float dy = playerCenter.y - turretCenter.y;
    vector2d bulletVel = {0.0f, 0.0f};
    const float epsilon = 0.1f; 

    if (std::abs(dx) < epsilon && std::abs(dy) < epsilon) {
        bulletVel.x = TURRET_BULLET_SPEED;
        bulletVel.y = 0.0f;
    } else {
        float absDx = std::abs(dx); 
        float absDy = std::abs(dy);
        const float diagonalThresholdRatio = 0.414f; 

        if (absDy < absDx * diagonalThresholdRatio) { 
            bulletVel.x = std::copysign(TURRET_BULLET_SPEED, dx);
            bulletVel.y = 0.0f;
        } else if (absDx < absDy * diagonalThresholdRatio) { 
            bulletVel.x = std::copysign(TURRET_DIAGONAL_SPEED_COMPONENT, dx);
            bulletVel.y = std::copysign(TURRET_DIAGONAL_SPEED_COMPONENT, dy);
        }
         else { 
            bulletVel.x = std::copysign(TURRET_DIAGONAL_SPEED_COMPONENT, dx);
            bulletVel.y = std::copysign(TURRET_DIAGONAL_SPEED_COMPONENT, dy);
        }
    }

    // TRUYỀN KÍCH THƯỚC RENDER VÀO CONSTRUCTOR CỦA BULLET
    enemyBullets.emplace_back(bulletTopLeftSpawnPos, bulletVel, this->bulletTexture, 
                              turretBulletRenderW, turretBulletRenderH);
    if (shootSound) Mix_PlayChannel(-1, shootSound, 0);
}

// --- takeDamage Method ---
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

// --- getWorldHitbox Method ---
SDL_Rect Turret::getWorldHitbox() const {
    SDL_Rect worldHB = {
        static_cast<int>(round(pos.x + hitbox.x)),
        static_cast<int>(round(pos.y + hitbox.y)),
        hitbox.w,
        hitbox.h
    };
    return worldHB;
}

// --- isFullyDestroyed Method ---
bool Turret::isFullyDestroyed() const {
    return currentState == TurretState::FULLY_DESTROYED;
}

// --- Render Method ---
void Turret::render(RenderWindow& window, float cameraX, float cameraY) {
    // Điều kiện thoát render nếu đã hoàn toàn bị phá hủy và animation nổ đã kết thúc
    if (currentState == TurretState::FULLY_DESTROYED && currentAnimFrameIndexExplosion >= (NUM_FRAMES_EXPLOSION -1) ) {
        // Có thể thêm một khoảng thời gian nhỏ để frame cuối của explosion được hiển thị
        // if (animTimerExplosion < ANIM_SPEED_EXPLOSION) { /* render explosion */ } else return;
        // Hiện tại, nếu đã là FULLY_DESTROYED và ở frame cuối, sẽ không render nữa (do return ở dưới)
        // Hoặc đơn giản là không làm gì cả, để logic dưới xử lý
    }

    SDL_Rect destRect;

    if (currentState == TurretState::DESTROYED_ANIM || 
        (currentState == TurretState::FULLY_DESTROYED && currentAnimFrameIndexExplosion < NUM_FRAMES_EXPLOSION) ) { 
        // Luôn render explosion nếu đang trong state DESTROYED_ANIM
        // hoặc nếu là FULLY_DESTROYED nhưng animation chưa chạy hết frame cuối cùng.
        if (!explosionTexture) return;

        // Kích thước render của explosion (có thể scale theo kích thước tile hoặc kích thước gốc của frame explosion)
        float explosionRenderWidth = static_cast<float>(sheetFrameWidthExplosion);
        float explosionRenderHeight = static_cast<float>(sheetFrameHeightExplosion);
        
        // Nếu muốn explosion cũng scale ra bằng kích thước tile của Turret:
        // explosionRenderWidth = static_cast<float>(renderWidthTurret);
        // explosionRenderHeight = static_cast<float>(renderHeightTurret);

        destRect = {
            static_cast<int>(round(pos.x - cameraX + (renderWidthTurret - explosionRenderWidth) / 2.0f)), // Căn giữa explosion với Turret
            static_cast<int>(round(pos.y - cameraY + (renderHeightTurret - explosionRenderHeight) / 2.0f)),
            static_cast<int>(round(explosionRenderWidth)),
            static_cast<int>(round(explosionRenderHeight))
        };
        // currentFrameSrcExplosion đã được cập nhật trong update()
        SDL_RenderCopy(window.getRenderer(), explosionTexture, &currentFrameSrcExplosion, &destRect);
    } else if (currentState != TurretState::FULLY_DESTROYED) { // Chỉ vẽ turret nếu chưa bị phá hủy hoàn toàn
        if (turretTexture) {
            destRect = {
                static_cast<int>(round(pos.x - cameraX)),
                static_cast<int>(round(pos.y - cameraY)),
                renderWidthTurret,
                renderHeightTurret
            };
            // currentFrameSrcTurret đã được cập nhật trong update()
            SDL_RenderCopy(window.getRenderer(), turretTexture, &currentFrameSrcTurret, &destRect);
        }
    }

    #ifdef DEBUG_DRAW_HITBOXES
    if (currentState != TurretState::DESTROYED_ANIM && currentState != TurretState::FULLY_DESTROYED) {
        SDL_SetRenderDrawColor(window.getRenderer(), 255, 0, 255, 100); // Màu tím cho hitbox
        SDL_Rect debugHitbox = getWorldHitbox();
        debugHitbox.x = static_cast<int>(round(debugHitbox.x - cameraX));
        debugHitbox.y = static_cast<int>(round(debugHitbox.y - cameraY));
        SDL_RenderDrawRect(window.getRenderer(), &debugHitbox);
    }
    #endif
}

