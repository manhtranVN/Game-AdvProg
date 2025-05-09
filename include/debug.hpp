#pragma once

// Debug Flags
// Uncomment to enable debug features
//#define DEBUG_DRAW_GRID     
//#define DEBUG_DRAW_COLUMNS 
//#define DEBUG_DRAW_PLAYER_HITBOX 
//#define DEBUG_DRAW_HITBOXES    

namespace Debug {
    void drawGrid(SDL_Renderer* renderer, float cameraX, float cameraY, 
                 const std::vector<std::vector<int>>& mapData, 
                 int tileWidth, int tileHeight, int screenWidth, int screenHeight);
                 
    void drawTileNumbers(SDL_Renderer* renderer, TTF_Font* font, 
                        float cameraX, float cameraY,
                        const std::vector<std::vector<int>>& mapData,
                        int tileWidth, int tileHeight);
}