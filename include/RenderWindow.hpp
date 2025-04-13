#pragma once
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>

#include "entity.hpp"
#include "math.hpp"

class RenderWindow
{
public:
    RenderWindow(const char* p_title, int p_w, int p_h); 
    void render(SDL_Texture* p_tex, const SDL_Rect& p_src, const SDL_Rect& p_dst);
    SDL_Texture* loadTexture(const char* p_filePath);   

    int getRefreshRate();

    void cleanUp();                                     
    void clear();                                       
    void render(entity& p_entity);                
    void display();   
    SDL_Renderer* getRenderer();                                   

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
};