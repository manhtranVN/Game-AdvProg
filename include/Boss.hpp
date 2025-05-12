#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <list>
#include <memory>
#include "math.hpp"
#include "Turret.hpp" // Boss sẽ chứa các Turret làm bộ phận
#include "Enemy.hpp"  // Boss có thể spawn Enemy
#include "Player.hpp"
#include "RenderWindow.hpp"

class Boss {
public:
    Boss(SDL_Texture* partTex, SDL_Texture* explosionTex, Mix_Chunk* p_explosionSound,
         Mix_Chunk* partShootSnd, SDL_Texture* bulletTexForParts,
         std::list<Enemy>* game_enemies_list_ptr, SDL_Texture* enemyTexForSpawn,
         int p_tileWidth, int p_tileHeight);

    void addPart(vector2d pos);
    void update(float dt, Player* player, std::list<Bullet>& enemyBullets, const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight);
    void render(RenderWindow& window, float cameraX, float cameraY);

    void takeHit(); // Gọi khi một bộ phận của boss bị bắn trúng
    bool isDefeated() const;
    bool isActive() const;
    void activate();

    float getHpPercentage() const;
    const std::vector<std::unique_ptr<Turret>>& getParts() const { return bossParts; } // Để kiểm tra va chạm

private:
    static const int TOTAL_HP = 50;
    static constexpr float ENEMY_SPAWN_INTERVAL = 8.0f; // Giây
    static constexpr float EXPLOSION_SOUND_DELAY = 0.1f; // Để âm thanh nổ của boss không bị chồng chéo quá nhiều

    int currentHp;
    bool bIsActive;
    bool bIsDefeated;
    bool bExplosionSoundPlayed;


    std::vector<std::unique_ptr<Turret>> bossParts;
    SDL_Texture* partTexture; // Dùng cho các Turret part
    SDL_Texture* explosionTextureForParts;
    Mix_Chunk* explosionSound; // Âm thanh nổ chính của Boss
    Mix_Chunk* partShootSound;
    SDL_Texture* bulletTextureForParts;


    // Để spawn enemy
    std::list<Enemy>* enemies_list_ref; // Tham chiếu đến danh sách enemy của game
    SDL_Texture* enemyTextureToSpawn;
    float enemySpawnTimer;
    float individualPartExplosionTimer;


    int tileWidth;
    int tileHeight;

    void spawnEnemyAtRandomPart(const std::vector<std::vector<int>>& mapData, int mapTileWidth, int mapTileHeight);
};