#include "entity.hpp"
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>

entity::entity(double p_x, double p_y, SDL_Texture *p_tex)
{
	x = p_x;
	y = p_y;
	tex = p_tex;
	currentFrame.x = 0;
	currentFrame.y = 0;
	currentFrame.w = 32;
	currentFrame.h = 32;

}

double entity::getX()
{
	return x;
}

double entity::getY()
{
	return y;
}

SDL_Texture* entity::getTex()
{
	return tex;
}

SDL_Rect entity::getCurrentFrame()
{
	return currentFrame;
}