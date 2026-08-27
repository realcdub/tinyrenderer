#include <cmath>
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAColor color, TGAImage &framebuffer) {
    // x(t) = ax + (bx - ax) * t
    // y(t) = ay + (by - ay) * t

    bool steep = std::abs(by - ay) > std::abs(ax - bx);

    if (steep) {
        std::swap(ax, ay);
        std::swap(bx, by);
    }

    if (bx < ax) {
        std::swap(ax, bx);
        std::swap(ay, by);
    }

    int y = ay;
    int error = 0;
    // float y = ay;
    // float slope = (by - ay) / static_cast<float>(bx - ax);
    for (int x = ax; x <= bx; ++x) {
        // float t = (x - ax) / static_cast<float>(bx - ax);
        // int y = std::round(ay + (by - ay) * t);

        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }

        error += 2 * (by - ay);
        if (error > bx - ax) {
            y += by > ay ? 1 : -1;
            error -= 2 * (bx - ax);
        }
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    // framebuffer.set(ax, ay, white);
    // framebuffer.set(bx, by, white);
    // framebuffer.set(cx, cy, white);

    line(ax, ay, bx, by, green, framebuffer);
    line(bx, by, cx, cy, blue, framebuffer);
    line(ax, ay, cx, cy, red, framebuffer);
    // line(cx, cy, ax, ay, yellow, framebuffer);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
