#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <list>
#include <memory>
#include "math.hpp"
#include "Turret.hpp"
#include "Enemy.hpp"
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

    void takeHit();
    bool isDefeated() const;
    bool isActive() const;
    void activate();

    float getHpPercentage() const;
    const std::vector<std::unique_ptr<Turret>>& getParts() const { return bossParts; }

private:
    static const int TOTAL_HP = 50;
    static constexpr float ENEMY_SPAWN_INTERVAL = 8.0f;
    static constexpr float EXPLOSION_SOUND_DELAY = 0.1f;

    int currentHp;
    bool bIsActive;
    bool bIsDefeated;
    bool bExplosionSoundPlayed;


    std::vector<std::unique_ptr<Turret>> bossParts;
    SDL_Texture* partTexture;
    SDL_Texture* explosionTextureForParts;
    Mix_Chunk* explosionSound;
    Mix_Chunk* partShootSound;
    SDL_Texture* bulletTextureForParts;


    std::list<Enemy>* enemies_list_ref;
    SDL_Texture* enemyTextureToSpawn;
    float enemySpawnTimer;
    float individualPartExplosionTimer;


    int tileWidth;
    int tileHeight;

    void spawnEnemyAtRandomPart(const std::vector<std::vector<int>>& mapData, int mapTileWidth, int mapTileHeight);
};