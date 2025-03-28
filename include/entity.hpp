#pragma once
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>

#include <math.hpp>
class entity
{
public:
	entity(vector2d p_pos, SDL_Texture* p_tex, int tileW, int tileH, int tilesetCols);
	vector2d& getPos()
	{
		return pos;
	}
	SDL_Texture* getTex();
	SDL_Rect getCurrentFrame();	 
	void setTileFrame(int tileIndex);
private:
	vector2d pos;
	SDL_Rect currentFrame;
	SDL_Texture* tex; 
	int tileWidth;
	int tileHeight;
	int tilesetColumns;
};