#pragma once
#include<iostream>
#include<cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct vector2d
{
    vector2d()
    :x(0.0f), y(0.0f)
    {}
    vector2d(float p_x, float p_y)
    :x(p_x), y(p_y)
    {}

    void print()
    {
        std::cout << x << " " << y << std::endl;
    }

    vector2d operator+(const vector2d& other) const
    {
        return vector2d(x + other.x, y + other.y);
    }
    vector2d& operator+=(const vector2d& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    vector2d operator-(const vector2d& other) const
    {
        return vector2d(x - other.x, y - other.y);
    }
    vector2d operator*(float scalar) const
    {
        return vector2d(x * scalar, y * scalar);
    }
    vector2d& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    vector2d operator/(float scalar) const
    {
        if (scalar != 0.0f)
        {
            return vector2d(x / scalar, y / scalar);
        }
        return vector2d(x,y);
    }


    float length() const
    {
        return std::sqrt(x * x + y * y);
    }

    vector2d normalized() const
    {
        float l = length();
        if (l > 0)
        {
            return vector2d(x / l, y / l);
        }
        return vector2d(0.0f, 0.0f);
    }

    float x, y;
};