#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<iostream>

#include "RenderWindow.hpp"
#include "entity.hpp"

using namespace std;

int main(int argc, char* args[])
{
    if(SDL_Init(SDL_INIT_VIDEO) > 0)
    {
        cout << "SDL_Init failed. SDL_Error: " << SDL_GetError() << endl;
    }
    if(!IMG_Init(IMG_INIT_PNG))
    {
        cout << "IMG_Init failed. SDL_Error: " << SDL_GetError() << endl;
    }

    RenderWindow window("game", 1280, 720);

    SDL_Texture* grassTexture = window.loadTexture("res/gfx/groundGrass_1.png");

    entity entities[3] = {  entity(0, 0, grassTexture),
                            entity(30, 0, grassTexture),
                            entity(30, 30, grassTexture)};

    bool gameRunning = true;

    SDL_Event event;

    while(gameRunning)
    {
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT) gameRunning = false;
        }
        window.clear();
        for(int i = 0; i < 3; i++) window.render(entities[i]);
        window.display();
    }

    window.cleanUp();
    SDL_Quit();

    return 0;
}