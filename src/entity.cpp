#include "entity.hpp"
#include "RenderWindow.hpp"

entity::entity(vector2d p_pos, SDL_Texture *p_tex, int p_frame_w, int p_frame_h, int p_sheet_cols)
    : // Khởi tạo theo thứ tự khai báo trong entity.hpp (đã giả định sửa đổi)
      pos(p_pos),
      tex(p_tex),
      currentFrame({0, 0, p_frame_w, p_frame_h}),
      frameWidth(p_frame_w),
      frameHeight(p_frame_h),
      tilesetColumns(p_sheet_cols > 0 ? p_sheet_cols : 1)
{}

void entity::setTileFrame(int tileIndex) {
    if (tilesetColumns > 0 && frameWidth > 0 && frameHeight > 0) {
        currentFrame.x = (tileIndex % tilesetColumns) * frameWidth;
        currentFrame.y = (tileIndex / tilesetColumns) * frameHeight;
        // w và h của currentFrame không đổi
    }
}

void entity::render(RenderWindow& window, float cameraX, float cameraY) {
    if (!tex) return;
    SDL_Rect dstRect = {
        static_cast<int>(round(pos.x - cameraX)),
        static_cast<int>(round(pos.y - cameraY)),
        frameWidth > 0 ? frameWidth : currentFrame.w, // Ưu tiên frameWidth nếu có
        frameHeight > 0 ? frameHeight : currentFrame.h
    };
    // Giả định RenderWindow có hàm getRenderer()
    SDL_RenderCopy(window.getRenderer(), tex, &currentFrame, &dstRect);
}