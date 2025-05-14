#include "Boss.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>

Boss::Boss(SDL_Texture* p_partTex, SDL_Texture* p_explosionTex, Mix_Chunk* p_explosionSnd,
           Mix_Chunk* p_partShootSnd, SDL_Texture* p_bulletTexForParts,
           std::list<Enemy>* game_enemies_list_ptr, SDL_Texture* p_enemyTexForSpawn,
           int p_tileW, int p_tileH)
    : currentHp(TOTAL_HP),
      bIsActive(false),
      bIsDefeated(false),
      bExplosionSoundPlayed(false),
      partTexture(p_partTex),
      explosionTextureForParts(p_explosionTex),
      explosionSound(p_explosionSnd),
      partShootSound(p_partShootSnd),
      bulletTextureForParts(p_bulletTexForParts),
      enemies_list_ref(game_enemies_list_ptr),
      enemyTextureToSpawn(p_enemyTexForSpawn),
      enemySpawnTimer(ENEMY_SPAWN_INTERVAL),
      individualPartExplosionTimer(0.0f),
      tileWidth(p_tileW),
      tileHeight(p_tileH)
{
    std::cout << "Boss created. Initial HP: " << currentHp << std::endl;
}

void Boss::addPart(vector2d pos)
{
    bossParts.emplace_back(std::make_unique<Turret>(
        pos,
        partTexture,
        explosionTextureForParts,
        nullptr,
        partShootSound,
        bulletTextureForParts,
        tileWidth,
        tileHeight
    ));
    if (!bossParts.empty())
    {
    }
    std::cout << "Boss part added at (" << pos.x << ", " << pos.y << "). Total parts: " << bossParts.size() << std::endl;
}

void Boss::activate()
{
    if (!bIsActive)
    {
        bIsActive = true;
        currentHp = TOTAL_HP;
        bIsDefeated = false;
        bExplosionSoundPlayed = false;
        enemySpawnTimer = ENEMY_SPAWN_INTERVAL / 2.0f;
        individualPartExplosionTimer = 0.0f;
        std::cout << "Boss activated!" << std::endl;
    }
}

void Boss::takeHit()
{
    if (!bIsActive || bIsDefeated)
    {
        return;
    }

    currentHp--;
    if (currentHp <= 0)
    {
        currentHp = 0;
        bIsDefeated = true;
        std::cout << "Boss defeated!" << std::endl;

        if (explosionSound && !bExplosionSoundPlayed)
        {
            Mix_PlayChannel(-1, explosionSound, 0);
            bExplosionSoundPlayed = true;
        }
    }
}

bool Boss::isDefeated() const
{
    if (!bIsDefeated)
    {
        return false;
    }

    return std::all_of(bossParts.begin(), bossParts.end(), [](const std::unique_ptr<Turret>& part)
    {
        return part->isFullyDestroyed();
    });
}


bool Boss::isActive() const
{
    return bIsActive;
}

float Boss::getHpPercentage() const
{
    return (TOTAL_HP > 0) ? (static_cast<float>(currentHp) / TOTAL_HP) : 0.0f;
}

void Boss::update(float dt, Player* player, std::list<Bullet>& enemyBullets, const std::vector<std::vector<int>>& mapData, int mapTileWidth, int mapTileHeight)
{
    if (!bIsActive)
    {
        return;
    }

    if (!bIsDefeated)
    {
        for (auto& part : bossParts)
        {
            if (part)
            {
                part->update(dt, player, enemyBullets);
            }
        }

        if (enemies_list_ref && enemyTextureToSpawn)
        {
            enemySpawnTimer -= dt;
            if (enemySpawnTimer <= 0.0f)
            {
                spawnEnemyAtRandomPart(mapData, mapTileWidth, mapTileHeight);
                enemySpawnTimer = ENEMY_SPAWN_INTERVAL + (static_cast<float>(rand() % 5) - 2.0f);
            }
        }
    }
    else
    {
        bool allPartsFullyDestroyed = true;
        individualPartExplosionTimer += dt;

        for (size_t i = 0; i < bossParts.size(); ++i)
        {
            if (bossParts[i] && !bossParts[i]->isFullyDestroyed())
            {
                if (bossParts[i]->getHp() > 0)
                {
                    if (individualPartExplosionTimer >= i * EXPLOSION_SOUND_DELAY)
                    {
                        bossParts[i]->forceExplode();
                    }
                }
                bossParts[i]->update(dt, nullptr, enemyBullets);
                if (!bossParts[i]->isFullyDestroyed())
                {
                    allPartsFullyDestroyed = false;
                }
            }
        }
    }
}


void Boss::spawnEnemyAtRandomPart(const std::vector<std::vector<int>>& mapData, int mapTileWidth, int mapTileHeight)
{
    if (bossParts.empty() || !enemies_list_ref || !enemyTextureToSpawn)
    {
        return;
    }

    int attempts = 0;
    while(attempts < 5)
    {
        int partIndex = rand() % bossParts.size();
        const auto& part = bossParts[partIndex];
        if (part)
        {
            vector2d spawnPos = {
                part->getWorldHitbox().x + part->getWorldHitbox().w/2.0f - 20.0f,
                part->getWorldHitbox().y
            };

            enemies_list_ref->emplace_back(spawnPos, enemyTextureToSpawn);
            std::cout << "Enemy spawned by boss at part index " << partIndex << std::endl;
            return;
        }
        attempts++;
    }
}


void Boss::render(RenderWindow& window, float cameraX, float cameraY)
{
    if (!bIsActive && !bIsDefeated)
    {
        return;
    }

    for (const auto& part : bossParts)
    {
        if (part)
        {
             part->render(window, cameraX, cameraY);
        }
    }
}