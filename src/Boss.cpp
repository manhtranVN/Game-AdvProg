#include "Boss.hpp"
#include "utils.hpp" // For random numbers if needed
#include <algorithm> // For std::all_of
#include <iostream>

Boss::Boss(SDL_Texture* p_partTex, SDL_Texture* p_explosionTex, Mix_Chunk* p_explosionSnd,
           Mix_Chunk* p_partShootSnd, SDL_Texture* p_bulletTexForParts,
           std::list<Enemy>* game_enemies_list_ptr, SDL_Texture* p_enemyTexForSpawn,
           int p_tileW, int p_tileH)
    : currentHp(TOTAL_HP), bIsActive(false), bIsDefeated(false), bExplosionSoundPlayed(false),
      partTexture(p_partTex), explosionTextureForParts(p_explosionTex), explosionSound(p_explosionSnd),
      partShootSound(p_partShootSnd), bulletTextureForParts(p_bulletTexForParts),
      enemies_list_ref(game_enemies_list_ptr), enemyTextureToSpawn(p_enemyTexForSpawn),
      enemySpawnTimer(ENEMY_SPAWN_INTERVAL), individualPartExplosionTimer(0.0f),
      tileWidth(p_tileW), tileHeight(p_tileH) {
    std::cout << "Boss created. Initial HP: " << currentHp << std::endl;
}

void Boss::addPart(vector2d pos) {
    // Các Turret này sẽ không tự "chết" theo HP của chúng, mà theo HP của Boss
    // HP của Turret ở đây chỉ dùng để kích hoạt animation nổ của nó khi Boss chết.
    // Chúng ta sẽ không gọi takeDamage() của Turret khi Boss bị bắn trúng,
    // mà sẽ gọi forceExplode() khi Boss bị đánh bại.
    bossParts.emplace_back(std::make_unique<Turret>(
        pos,
        partTexture,
        explosionTextureForParts, // Turret sẽ dùng texture nổ này
        nullptr, // Không để Turret tự phát âm thanh nổ, Boss sẽ quản lý
        partShootSound,
        bulletTextureForParts,
        tileWidth, tileHeight
    ));
    // Giảm detectionRadius của các part để chúng không bắn quá sớm/xa
    if (!bossParts.empty()) {
         // Chỉnh sửa detectionRadius của Turret (nếu Turret có setter hoặc bạn có thể truy cập trực tiếp nếu là friend class)
         // Hoặc là điều chỉnh trong constructor của Turret nếu nó là một boss part.
         // Hiện tại, Turret không có setter cho detectionRadius, ta để mặc định.
    }
    std::cout << "Boss part added at (" << pos.x << ", " << pos.y << "). Total parts: " << bossParts.size() << std::endl;
}

void Boss::activate() {
    if (!bIsActive) {
        bIsActive = true;
        currentHp = TOTAL_HP;
        bIsDefeated = false;
        bExplosionSoundPlayed = false;
        enemySpawnTimer = ENEMY_SPAWN_INTERVAL / 2.0f; // Spawn lính sớm hơn một chút khi bắt đầu
        individualPartExplosionTimer = 0.0f;
        std::cout << "Boss activated!" << std::endl;
    }
}

void Boss::takeHit() {
    if (!bIsActive || bIsDefeated) return;

    currentHp--;
    // std::cout << "Boss took hit. HP: " << currentHp << "/" << TOTAL_HP << std::endl;
    if (currentHp <= 0) {
        currentHp = 0;
        bIsDefeated = true;
        std::cout << "Boss defeated!" << std::endl;
        
        // Kích hoạt nổ cho tất cả các bộ phận
        // Sẽ phát một âm thanh nổ lớn cho boss, sau đó các phần sẽ nổ tuần tự hoặc đồng thời
        if (explosionSound && !bExplosionSoundPlayed) {
            Mix_PlayChannel(-1, explosionSound, 0);
            bExplosionSoundPlayed = true; // Chỉ phát một lần
        }
        // Các phần sẽ bắt đầu nổ trong hàm update
    }
}

bool Boss::isDefeated() const {
    if (!bIsDefeated) return false;
    // Boss được coi là hoàn toàn bị đánh bại khi tất cả các phần của nó đã hoàn thành animation nổ
    return std::all_of(bossParts.begin(), bossParts.end(), [](const std::unique_ptr<Turret>& part){
        return part->isFullyDestroyed();
    });
}


bool Boss::isActive() const {
    return bIsActive;
}

float Boss::getHpPercentage() const {
    return (TOTAL_HP > 0) ? (static_cast<float>(currentHp) / TOTAL_HP) : 0.0f;
}

void Boss::update(float dt, Player* player, std::list<Bullet>& enemyBullets, const std::vector<std::vector<int>>& mapData, int mapTileWidth, int mapTileHeight) {
    if (!bIsActive) return;

    if (!bIsDefeated) {
        // Cập nhật các bộ phận của boss (để chúng bắn)
        for (auto& part : bossParts) {
            if (part) { // Đảm bảo con trỏ hợp lệ
                part->update(dt, player, enemyBullets);
            }
        }

        // Xử lý spawn enemy
        if (enemies_list_ref && enemyTextureToSpawn) {
            enemySpawnTimer -= dt;
            if (enemySpawnTimer <= 0.0f) {
                spawnEnemyAtRandomPart(mapData, mapTileWidth, mapTileHeight);
                enemySpawnTimer = ENEMY_SPAWN_INTERVAL + (static_cast<float>(rand() % 5) - 2.0f); // Thêm chút ngẫu nhiên
            }
        }
    } else { // Boss đã bị đánh bại, xử lý animation nổ
        bool allPartsFullyDestroyed = true;
        individualPartExplosionTimer += dt;

        // Kích hoạt nổ cho từng phần một cách tuần tự hoặc gần đồng thời
        for (size_t i = 0; i < bossParts.size(); ++i) {
            if (bossParts[i] && !bossParts[i]->isFullyDestroyed()) {
                // Nếu phần này chưa ở trạng thái nổ và chưa bị phá hủy hoàn toàn
                if (bossParts[i]->getHp() > 0) { // Kiểm tra xem nó đã được forceExplode chưa
                     // Kích hoạt nổ cho phần này nếu đến lượt (ví dụ, mỗi 0.2s một phần)
                    if (individualPartExplosionTimer >= i * EXPLOSION_SOUND_DELAY) { // Hiệu ứng nổ lan tỏa
                        bossParts[i]->forceExplode();
                    }
                }
                bossParts[i]->update(dt, nullptr, enemyBullets); // Cập nhật animation nổ
                if (!bossParts[i]->isFullyDestroyed()) {
                    allPartsFullyDestroyed = false;
                }
            }
        }
        // if (allPartsFullyDestroyed) {
        //     // Logic này đã được chuyển vào isDefeated()
        // }
    }
}


void Boss::spawnEnemyAtRandomPart(const std::vector<std::vector<int>>& mapData, int mapTileWidth, int mapTileHeight) {
    if (bossParts.empty() || !enemies_list_ref || !enemyTextureToSpawn) return;

    int attempts = 0;
    while(attempts < 5) { // Thử tìm vị trí spawn vài lần
        int partIndex = rand() % bossParts.size();
        const auto& part = bossParts[partIndex];
        if (part) { // Đảm bảo con trỏ hợp lệ
            // Lấy vị trí của boss part
            //SDL_Rect partHitbox = part->getWorldHitbox(); // Lấy hitbox để biết vị trí
            //vector2d spawnPos = {static_cast<float>(partHitbox.x), static_cast<float>(partHitbox.y - 72)}; // Spawn phía trên một chút

            // Để đơn giản, lấy vị trí gốc của Turret part
            // Turret::pos là góc trên trái. Enemy cũng có pos là góc trên trái.
            // Enemy cao 72. Turret part (tile) cao 96.
            // Spawn enemy ở y = part.pos.y + (part.height / 2) - enemy.height
            // Hoặc đơn giản là part.pos.y (nếu enemy có cơ chế rơi)
            vector2d spawnPos = { part->getWorldHitbox().x + part->getWorldHitbox().w/2.0f - 20.0f, // -20 để enemy không bị kẹt nếu part ở rìa
                                   part->getWorldHitbox().y }; // Enemy sẽ rơi xuống

            // Kiểm tra xem vị trí spawn có hợp lệ không (ví dụ, không nằm trong tường)
            // Cần kiểm tra tile tại spawnPos.x, spawnPos.y + enemyHeight
            // Hiện tại, Enemy tự xử lý rơi và va chạm, nên cứ spawn.
            
            enemies_list_ref->emplace_back(spawnPos, enemyTextureToSpawn);
            std::cout << "Enemy spawned by boss at part index " << partIndex << std::endl;
            return; // Spawn thành công
        }
        attempts++;
    }
}


void Boss::render(RenderWindow& window, float cameraX, float cameraY) {
    if (!bIsActive && !bIsDefeated) return; // Không render nếu chưa active và chưa bị đánh bại (tránh render lúc đầu)
                                        // Nếu đã bị đánh bại thì vẫn render animation nổ

    for (const auto& part : bossParts) {
        if (part) { // Đảm bảo con trỏ hợp lệ
             part->render(window, cameraX, cameraY);
        }
    }
}