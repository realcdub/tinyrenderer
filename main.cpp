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

    for (float t = 0; t < 1; t += 0.01) {
        int x = ax + (bx - ax) * t;
        int y = ay + (by - ay) * t;
        framebuffer.set(x, y, color);
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

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
