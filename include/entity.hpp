#pragma once
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>

class entity
{
public:
	entity(double p_x, double p_y, SDL_Texture* p_tex);
	double getX();
	double getY();
	SDL_Texture* getTex();
	SDL_Rect getCurrentFrame();	 
private:
	double x, y;
	SDL_Rect currentFrame;
	SDL_Texture* tex; 
};