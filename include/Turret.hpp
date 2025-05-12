#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "math.hpp"
#include "RenderWindow.hpp"
#include "Player.hpp" 
#include "Bullet.hpp"
#include <list>
#include <vector>
#include <string>
#include <algorithm> 
#include <cmath>     

enum class TurretState {
    IDLE, SHOOTING, DESTROYED_ANIM, FULLY_DESTROYED
};

class Turret {
public:
    Turret(vector2d p_pos, SDL_Texture* p_turretTex,
           SDL_Texture* p_explosionTex, Mix_Chunk* p_explosionSnd, Mix_Chunk* p_shootSnd,
           SDL_Texture* p_bulletTex, int p_tileWidth, int p_tileHeight); 

    void update(float dt, Player* player, std::list<Bullet>& enemyBullets);
    void render(RenderWindow& window, float cameraX, float cameraY);
    void takeDamage();
    SDL_Rect getWorldHitbox() const;
    bool isFullyDestroyed() const;
    int getHp() const { return hp; }
    void forceExplode();
    void setHp(int newHp);

private:
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

    vector2d pos;
    SDL_Texture* turretTexture;
    SDL_Texture* explosionTexture;
    SDL_Texture* bulletTexture;
    Mix_Chunk* explosionSound;
    Mix_Chunk* shootSound;

    SDL_Rect currentFrameSrcTurret;    
    SDL_Rect currentFrameSrcExplosion; 
    int renderWidthTurret, renderHeightTurret;
    int sheetFrameWidthTurret, sheetFrameHeightTurret; 
    int sheetFrameWidthExplosion, sheetFrameHeightExplosion; 

    int sheetColsTurretAnim;
    int sheetColsExplosion;

    int currentAnimFrameIndexTurret;
    int currentAnimFrameIndexExplosion;
    float animTimerTurret;
    float animTimerExplosion;

    TurretState currentState;
    SDL_Rect hitbox; 
    int hp;
    float detectionRadius;
    float shootCooldown;
    float currentShootTimer;

    void shootAtPlayer(Player* player, std::list<Bullet>& enemyBullets);
};
