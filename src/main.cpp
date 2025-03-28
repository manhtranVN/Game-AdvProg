#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<iostream>
#include<vector>

#include "RenderWindow.hpp"
#include "entity.hpp"
#include "math.hpp"
#include "utils.hpp"

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
    //int windowRefreshRate = window.getRefreshRate();
    cout << window.getRefreshRate() << endl;
    SDL_Texture* grassTexture = window.loadTexture("res/gfx/Tileset.png");

    vector <entity> entities = {    entity(vector2d(0, 0), grassTexture, 32, 32, 12),
                                    entity(vector2d(30, 60), grassTexture, 32, 32, 12),
                                    entity(vector2d(60, 90), grassTexture, 32, 32, 12)};

    bool gameRunning = true;

    SDL_Event event;

    const double timeStep = 0.01d;
    double accumulator = 0.0d;
    double currentTime = utils::hireTimeInSeconds();

    while(gameRunning)
    {
        int startTicks = SDL_GetTicks();
        double newTime = utils::hireTimeInSeconds();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        accumulator += frameTime;

        while(accumulator >= timeStep)
        {
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_QUIT) gameRunning = false;
            }
            accumulator -= timeStep;
        }

        const double alpha = accumulator / timeStep;

        
        window.clear();
        for(int i = 0; i < 3; i++) window.render(entities[i]);
        entities[0].setTileFrame(0);
        entities[1].setTileFrame(3);
        entities[2].setTileFrame(9);
        cout << utils::hireTimeInSeconds() << endl;
        window.display();

        int frameTicks = SDL_GetTicks() - startTicks;
        if(frameTicks < 1000 / window.getRefreshRate())
        {
            SDL_Delay(1000/window.getRefreshRate() - frameTicks);
        }

    }

    window.cleanUp();
    SDL_Quit();

    return 0;
}