#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "math.hpp"
#include "RenderWindow.hpp"
#include "Player.hpp" // Đảm bảo Player.hpp đã được include đầy đủ
#include "Bullet.hpp"
#include <list>
#include <vector>
#include <string>
#include <algorithm> // Cho std::max
#include <cmath>     // Cho std::sqrt

enum class TurretState {
    IDLE, SHOOTING, DESTROYED_ANIM, FULLY_DESTROYED
};

class Turret {
public:
    // Constructor
    Turret(vector2d p_pos, SDL_Texture* p_turretTex,
           SDL_Texture* p_explosionTex, Mix_Chunk* p_explosionSnd, Mix_Chunk* p_shootSnd,
           SDL_Texture* p_bulletTex, int p_tileWidth, int p_tileHeight); // Bỏ desiredWidth, desiredHeight

    // Public methods
    void update(float dt, Player* player, std::list<Bullet>& enemyBullets);
    void render(RenderWindow& window, float cameraX, float cameraY);
    void takeDamage();
    SDL_Rect getWorldHitbox() const;
    bool isFullyDestroyed() const;
    int getHp() const { return hp; }

private:
    // --- Static Constants ---
    static constexpr float ANIM_SPEED_TURRET_IDLE = 0.2f;
    static constexpr float ANIM_SPEED_TURRET_SHOOT = 0.1f;
    static constexpr float ANIM_SPEED_EXPLOSION = 0.1f;
    static constexpr float TURRET_BULLET_SPEED = 350.0f;
    static constexpr float TURRET_DIAGONAL_SPEED_COMPONENT = TURRET_BULLET_SPEED / 1.41421356237f;

    static const int NUM_FRAMES_TURRET_IDLE;
    static const int START_FRAME_TURRET_IDLE;
    static const int NUM_FRAMES_TURRET_SHOOT;
    static const int START_FRAME_TURRET_SHOOT;
    static const int NUM_FRAMES_EXPLOSION;

    // --- Member variables ---
    vector2d pos;
    SDL_Texture* turretTexture;
    SDL_Texture* explosionTexture;
    SDL_Texture* bulletTexture;
    Mix_Chunk* explosionSound;
    Mix_Chunk* shootSound;

    SDL_Rect currentFrameSrcTurret;    // Đổi tên để rõ ràng đây là source rect từ spritesheet
    SDL_Rect currentFrameSrcExplosion; // Đổi tên để rõ ràng đây là source rect từ spritesheet
    int renderWidthTurret, renderHeightTurret; // Kích thước render (sẽ là tileWidth, tileHeight)
    int sheetFrameWidthTurret, sheetFrameHeightTurret; // Kích thước 1 frame trên spritesheet turret
    int sheetFrameWidthExplosion, sheetFrameHeightExplosion; // Kích thước 1 frame trên spritesheet explosion

    int sheetColsTurretAnim;
    int sheetColsExplosion;

    int currentAnimFrameIndexTurret;
    int currentAnimFrameIndexExplosion;
    float animTimerTurret;
    float animTimerExplosion;

    TurretState currentState;
    SDL_Rect hitbox; // Hitbox sẽ dựa trên renderWidthTurret, renderHeightTurret
    int hp;
    float detectionRadius;
    float shootCooldown;
    float currentShootTimer;

    // Private methods
    void shootAtPlayer(Player* player, std::list<Bullet>& enemyBullets);
};
