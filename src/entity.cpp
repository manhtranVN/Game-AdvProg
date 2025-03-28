#include "entity.hpp"
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>

entity::entity(vector2d p_pos, SDL_Texture *p_tex, int tileW, int tileH, int tilesetCols)
	:tex(p_tex), pos(p_pos), tileWidth(tileW), tileHeight(tileH), tilesetColumns(tilesetCols)
{
	currentFrame.x = 0;
	currentFrame.y = 0;
	currentFrame.w = tileWidth;
	currentFrame.h = tileHeight;
}

void entity::setTileFrame(int tileIndex)
{
	currentFrame.x = currentFrame.x = (tileIndex % tilesetColumns) * tileWidth;
    currentFrame.y = (tileIndex / tilesetColumns) * tileHeight;
    currentFrame.w = tileWidth; 
    currentFrame.h = tileHeight;
}

SDL_Texture* entity::getTex()
{
	return tex;
}

SDL_Rect entity::getCurrentFrame()
{
	return currentFrame;
}