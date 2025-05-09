#include "debug.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <cmath>

void Debug::drawGrid(SDL_Renderer* renderer, float cameraX, float cameraY, 
                    const std::vector<std::vector<int>>& mapData, 
                    int tileWidth, int tileHeight, int screenWidth, int screenHeight) {
    if (!renderer) return;
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 70);
    
    int startCol = static_cast<int>(floor(cameraX/tileWidth));
    int endCol = startCol + static_cast<int>(ceil((float)screenWidth/tileWidth)) + 1;
    
    for(int c = startCol; c < endCol; ++c) {
        int sx = static_cast<int>(round(c*tileWidth-cameraX));
        SDL_RenderDrawLine(renderer, sx, 0, sx, screenHeight);
    }
    
    for(int r = 0; r < mapData.size()+1; ++r) {
        int sy = static_cast<int>(round(r*tileHeight-cameraY));
        SDL_RenderDrawLine(renderer, 0, sy, screenWidth, sy);
    }
}

void Debug::drawTileNumbers(SDL_Renderer* renderer, TTF_Font* font, 
                          float cameraX, float cameraY,
                          const std::vector<std::vector<int>>& mapData,
                          int tileWidth, int tileHeight) {
    if (!renderer || !font) return;
    
    SDL_Color textColor = {255, 255, 0, 255};
    int startCol = static_cast<int>(floor(cameraX / tileWidth));
    int endCol = startCol + static_cast<int>(ceil(static_cast<float>(SCREEN_WIDTH) / tileWidth)) + 1;
    endCol = std::min(endCol, static_cast<int>(mapData[0].size()));

    for (int r = 0; r < mapData.size(); ++r) {
        for (int c = startCol; c < endCol; ++c) {
            if (c < 0 || c >= mapData[0].size()) continue;
            if (r < 0 || r >= mapData.size()) continue;

            int screenX = static_cast<int>(round(c * tileWidth - cameraX));
            int screenY = static_cast<int>(round(r * tileHeight - cameraY));

            if (screenX + tileWidth < 0 || screenX > SCREEN_WIDTH ||
                screenY + tileHeight < 0 || screenY > SCREEN_HEIGHT) {
                continue;
            }
            
            string tileText = std::to_string(mapData[r][c]);
            SDL_Surface* surface = TTF_RenderText_Solid(font, tileText.c_str(), textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    int textX = screenX + (tileWidth - surface->w) / 2;
                    int textY = screenY + (tileHeight - surface->h) / 2;
                    SDL_Rect dstRect = {textX, textY, surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
    }
}