#pragma once

#include<SDL2/SDL.h>

namespace utils
{
	inline double hireTimeInSeconds()
	{
		double t = SDL_GetTicks();
		t *= 0.001d;
		return t;
	}
}