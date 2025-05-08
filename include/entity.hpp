#pragma once

#include "math.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

class RenderWindow;

class entity {
public: // <--- THAY THÀNH PUBLIC
    entity(vector2d p_pos, SDL_Texture* p_tex = nullptr, int frame_w = 0, int frame_h = 0, int sheet_cols = 1);
    virtual ~entity() {}

    // Getters (giờ là public)
    vector2d& getPos() { return pos; }
    const vector2d& getPos() const { return pos; }
    SDL_Texture* getTex() { return tex; } // Giữ tên gốc từ file của bạn
    SDL_Rect getCurrentFrame() { return currentFrame; } // Giữ tên gốc từ file của bạn

    // Setters (giờ là public)
    void setPos(const vector2d& p_pos) { pos = p_pos; }
    void setTileFrame(int tileIndex);

    virtual void render(RenderWindow& window, float cameraX, float cameraY);

// Các thành viên dữ liệu có thể giữ protected hoặc private nếu muốn,
// vì đã có getter/setter public
protected: // Hoặc private
    vector2d pos;
    SDL_Texture* tex;
    SDL_Rect currentFrame;
    int frameWidth;
    int frameHeight;
    int tilesetColumns;
};