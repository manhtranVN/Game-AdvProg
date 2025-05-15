#pragma once

#include <SDL2/SDL.h>
#include "math.hpp" 
#include <vector>  

class RenderWindow; 

class Bullet {
public:
    Bullet(vector2d p_pos, vector2d p_vel, SDL_Texture* p_tex, int p_renderW, int p_renderH);

    void update(double dt);
    void render(RenderWindow& window, double cameraX, double cameraY);
    bool isActive() const;
    void setActive(bool active);
    SDL_Rect getWorldHitbox() const; 

    void checkMapCollision(const std::vector<std::vector<int>>& mapData, int tileWidth, int tileHeight);

private:
    vector2d pos;
    vector2d velocity;
    SDL_Texture* tex;
    SDL_Rect currentFrame;      
    bool active;
    double lifeTime; 
    const double MAX_LIFETIME = 2.0; 

    int renderWidth;
    int renderHeight;

};

