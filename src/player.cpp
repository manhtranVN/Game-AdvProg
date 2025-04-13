#include "Player.hpp"
#include "RenderWindow.hpp" 
#include <SDL2/SDL.h>
#include <algorithm> 
#include <cmath>    

using namespace std;

const int TILE_EMPTY = 0;
const int TILE_GRASS = 1; 

Player::Player(vector2d p_pos,
               SDL_Texture* p_runTex, int p_runSheetCols,
               SDL_Texture* p_jumpTex, int p_jumpSheetCols,
               int p_frameW, int p_frameH)
    : entity(p_pos, p_runTex, p_frameW, p_frameH, p_runSheetCols), 
      runTexture(p_runTex),
      jumpTexture(p_jumpTex),
      runSheetColumns(p_runSheetCols),
      jumpSheetColumns(p_jumpSheetCols),
      frameWidth(p_frameW),
      frameHeight(p_frameH),
      velocity({0.0f, 0.0f}),
      currentState(PlayerState::FALLING),
      facing(FacingDirection::RIGHT),
      isOnGround(false),
      requestDrop(false),
      animTimer(0.0f),
      currentAnimFrameIndex(0)
{

    hitbox.x = 10*3;  
    hitbox.y = 4*3;  
    hitbox.w = 13*3; 
    hitbox.h = 78*3; 

    currentSourceRect.x = 0;
    currentSourceRect.y = 0;
    currentSourceRect.w = frameWidth;
    currentSourceRect.h = frameHeight;

   // setTileFrame(0); 
}

SDL_Rect Player::getWorldHitbox() 
{
    SDL_Rect worldHB;
    worldHB.x = static_cast<int>(getPos().x + hitbox.x);
    worldHB.y = static_cast<int>(getPos().y + hitbox.y);
    worldHB.w = hitbox.w;
    worldHB.h = hitbox.h;
    return worldHB;
}

void Player::handleInput(const Uint8* keyStates)
{
    requestDrop = false; 
    if (keyStates[SDL_SCANCODE_LEFT]) 
    {
        velocity.x = -MOVE_SPEED;
        facing = FacingDirection::LEFT;
    } else if (keyStates[SDL_SCANCODE_RIGHT])
     {
        velocity.x = MOVE_SPEED;
        facing = FacingDirection::RIGHT;
    } else {
        velocity.x = 0; 
    }

    if (keyStates[SDL_SCANCODE_SPACE] && isOnGround) 
    { 
        velocity.y = -JUMP_STRENGTH;
        isOnGround = false;
    }

    if (keyStates[SDL_SCANCODE_DOWN] && isOnGround) 
    {
        requestDrop = true;
    }
}

void Player::update(double dt, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) 
{   
    cout << "[Player Update START] Received Tile Dimensions: W=" << tileWidth << " H=" << tileHeight << endl;
    applyGravity(dt);
    move(dt); 
    checkMapCollision(mapData, tileWidth, tileHeight); 
    updateState(); 
    updateAnimation(dt); 
}

void Player::applyGravity(double dt) 
{
    if (!isOnGround) 
    {
        velocity.y += GRAVITY * dt;
        velocity.y = min(velocity.y, MAX_FALL_SPEED);
    }
}

void Player::move(double dt) 
{
    vector2d& posRef = getPos(); 
    posRef.x += velocity.x * dt;
    posRef.y += velocity.y * dt;
    
}

void Player::checkMapCollision(const vector<vector<int>>& mapData, int tileWidth, int tileHeight) 
{
    vector2d& posRef = getPos();
    SDL_Rect playerHB = getWorldHitbox();

    bool previouslyOnGround = isOnGround;
    isOnGround = false;

    int feetY = playerHB.y + playerHB.h; 
    int headY = playerHB.y;           
    int midX = playerHB.x + playerHB.w / 2; 

    
    int tileBelowRow = static_cast<int>(floor(static_cast<double>(feetY) / tileHeight));
    int tileBelowCol = static_cast<int>(floor(static_cast<double>(midX) / tileWidth));

    int tileBelowType = getTileAt(midX, feetY, mapData, tileWidth, tileHeight);

    
    bool tryingToDrop = requestDrop && (tileBelowType == TILE_GRASS); 
    if (tryingToDrop) 
    {
         posRef.y += 1.0f;
         currentState = PlayerState::DROPPING;
    }
    else if (velocity.y >= 0 && tileBelowType == TILE_GRASS) 
    {
        isOnGround = true;
        velocity.y = 0;
        posRef.y = static_cast<double>(tileBelowRow * tileHeight - hitbox.h - hitbox.y);
        if (!previouslyOnGround) 
        {
             // Có thể thêm hiệu ứng tiếp đất ở đây
        }
    }

    int tileAboveRow = static_cast<int>(floor(static_cast<double>(headY) / tileHeight));
    int tileAboveCol = static_cast<int>(floor(static_cast<double>(midX) / tileWidth));
    int tileAboveType = getTileAt(midX, headY, mapData, tileWidth, tileHeight);

    if (velocity.y < 0 && tileAboveType == TILE_GRASS) 
    { 
        velocity.y = 0; 
        posRef.y = static_cast<double>((tileAboveRow + 1) * tileHeight - hitbox.y);
    }

    playerHB = getWorldHitbox();
    int checkYMid = playerHB.y + playerHB.h / 2; 

    if (velocity.x > 0) 
    { 
        int rightX = playerHB.x + playerHB.w;
        int tileRightType = getTileAt(rightX, checkYMid, mapData, tileWidth, tileHeight);
        if (tileRightType == TILE_GRASS) 
        { 
            velocity.x = 0;
            int tileCol = static_cast<int>(floor(static_cast<double>(rightX) / tileWidth));
            posRef.x = static_cast<double>(tileCol * tileWidth - hitbox.w - hitbox.x);
        }
    } 
    else 
    if (velocity.x < 0) //Di chuyển sang trái
    { 
        int leftX = playerHB.x; // Tọa độ X cạnh trái hitbox
        int checkYMid = playerHB.y + playerHB.h / 2; // Điểm kiểm tra giữa người theo chiều Y

        // Lấy chỉ số cột của tile mà cạnh trái hitbox đang ở hoặc sắp vào
        int tileCol = static_cast<int>(floor(static_cast<float>(leftX) / tileWidth));

        int tileLeftType = getTileAt(leftX, checkYMid, mapData, tileWidth, tileHeight);

        // --- THÊM LOG DEBUG CHI TIẾT ---
        cout << "[DEBUG] Checking Left Collision:\n";
        cout << "  Current pos.x: " << getPos().x << ", velocity.x: " << velocity.x << "\n";
        cout << "  Hitbox leftX: " << leftX << ", checkYMid: " << checkYMid << "\n";
        cout << "  Checking Tile Column Index: " << tileCol << "\n";
        cout << "  Tile Type Found: " << tileLeftType << "\n";
        // --------------------------------

        if (tileLeftType == TILE_GRASS /* || Các loại tile rắn khác */) 
        {
            cout << "  --> Left Wall Collision Detected!\n";
            velocity.x = 0; // Dừng di chuyển ngang

            // Tính toán tọa độ X của cạnh PHẢI của ô tile vừa va chạm
            float tileRightEdgeX = static_cast<float>((tileCol + 1) * tileWidth);

            // Tính toán vị trí mới cho player (pos.x) sao cho cạnh TRÁI của hitbox
            // (pos.x + hitbox.x) nằm ngay sát cạnh PHẢI của ô tile (tileRightEdgeX)
            // => pos.x = tileRightEdgeX - hitbox.x
            float newX = tileRightEdgeX - static_cast<float>(hitbox.x);

            // --- THÊM LOG DEBUG CHI TIẾT ---
            cout << "      Collided Tile Column: " << tileCol << "\n";
            cout << "      Tile Right Edge X: " << tileRightEdgeX << "\n";
            cout << "      Player Hitbox Offset X (hitbox.x): " << hitbox.x << "\n";
            cout << "      Calculated newX: " << newX << "\n";
            // ---------------------------------

            // Áp dụng vị trí mới
            vector2d& posRef = getPos();
            posRef.x = newX;
            cout << "      Applied new pos.x: " << posRef.x << "\n"; // Xem giá trị cuối cùng

        } 
        else 
        {
            cout << "  --> No Left Wall Collision.\n";
        }
         // --- Reset trạng thái DROPPING nếu không còn yêu cầu hoặc đã rơi qua ---
        if (currentState == PlayerState::DROPPING && !requestDrop) 
        {
             // Kiểm tra nếu đã hoàn toàn rơi qua khỏi nền cũ thì chuyển sang FALLING
             // (Logic kiểm tra cần chính xác hơn)
             // Tạm thời: Nếu không còn giữ nút xuống thì ngừng DROPPING
             currentState = PlayerState::FALLING; // Sẽ tự cập nhật trong updateState nếu cần
        }
    }

}

int Player::getTileAt(double worldX, double worldY, const vector<vector<int>>& mapData, int tileWidth, int tileHeight) 
{
    if (worldX < 0 || worldY < 0) return TILE_EMPTY;
    int col = static_cast<int>(floor(worldX / tileWidth));
    int row = static_cast<int>(floor(worldY / tileHeight));
    if (row >= 0 && row < mapData.size()) 
    {
        if (col >= 0 && col < mapData[row].size()) 
        {
            return mapData[row][col];
        }
    }
    return TILE_EMPTY;
}


void Player::updateState() 
{
    if (currentState == PlayerState::DROPPING) 
    {
         if (isOnGround) 
         {
              currentState = (velocity.x != 0) ? PlayerState::RUNNING : PlayerState::IDLE;
         }
         return;
    }


    if (isOnGround) {
        if (velocity.x != 0) {
            currentState = PlayerState::RUNNING;
        } else {
            currentState = PlayerState::IDLE;
        }
    } else { // Đang ở trên không
        if (velocity.y < 0) {
            currentState = PlayerState::JUMPING;
        } else {
            currentState = PlayerState::FALLING;
        }
    }
}

void Player::updateAnimation(double dt) 
{
    animTimer += dt;
    if (animTimer >= ANIM_SPEED) 
    {
        animTimer -= ANIM_SPEED;

        int startFrame = 0; 
        int numFrames = 1;
        int sheetCols = runSheetColumns; 

        bool useJumpSheet = false; 

        switch (currentState) 
        {
            case PlayerState::IDLE:
                startFrame = 0; 
                numFrames = 1;
                currentAnimFrameIndex = 0;
                sheetCols = runSheetColumns;
                useJumpSheet = false;
                break;
            case PlayerState::RUNNING:
                startFrame = 0; 
                numFrames = RUN_FRAMES;
                currentAnimFrameIndex = (currentAnimFrameIndex + 1) % numFrames;
                sheetCols = runSheetColumns;
                useJumpSheet = false;
                break;
            case PlayerState::JUMPING:
            case PlayerState::FALLING:
            case PlayerState::DROPPING:
                startFrame = 0; 
                numFrames = JUMP_FRAMES;
                currentAnimFrameIndex++;
                if (currentAnimFrameIndex >= numFrames) 
                {
                    currentAnimFrameIndex = numFrames - 1;
                }
                sheetCols = jumpSheetColumns;
                useJumpSheet = true;
                break;
        }
        int frameIndexInLogic = startFrame + currentAnimFrameIndex; 
        int tileX = (frameIndexInLogic % sheetCols) * frameWidth;
        int tileY = (frameIndexInLogic / sheetCols) * frameHeight;
        currentSourceRect.x = tileX;
        currentSourceRect.y = tileY;
        currentSourceRect.w = frameWidth;
        currentSourceRect.h = frameHeight;
    }
}

void Player::render(RenderWindow& window, double cameraX, double cameraY) 
{
    SDL_Texture* textureToRender = nullptr;
    if (currentState == PlayerState::JUMPING || currentState == PlayerState::FALLING || currentState == PlayerState::DROPPING) 
    {
        textureToRender = jumpTexture;
    } 
    else 
    {
        textureToRender = runTexture; 
    }

    if (!textureToRender) return; 

    SDL_Rect destRect;
    destRect.x = static_cast<int>(getPos().x - cameraX);
    destRect.y = static_cast<int>(getPos().y - cameraY);
    destRect.w = currentSourceRect.w; 
    destRect.h = currentSourceRect.h;

    SDL_RendererFlip flip = (facing == FacingDirection::LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    //cout << "Rendering Player at: (" << destRect.x << ", " << destRect.y << ")" << endl;
    SDL_RenderCopyEx(window.getRenderer(), textureToRender, &currentSourceRect, &destRect, 0.0, NULL, flip);
}