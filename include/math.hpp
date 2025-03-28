#pragma once
#include<iostream>
struct vector2d
{
	vector2d()
	:x(0.0d), y(0.0d)
	{}
	vector2d(double p_x, double p_y)
	:x(p_x), y(p_y)
	{}
	void print()
	{
		std::cout << x << " " << y << std::endl;
	}
	double x, y;
};
