#include <cmath>
#include "tgaimage.h"
#include <ctime>
#include <chrono>
#include <iostream>

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
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
        y += (by > ay ? 1 : -1) * (error > bx - ax);
        error -= (2 * (bx - ax)) * (error > bx - ax);
        // y += slope;
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    std::srand(std::time({}));
    auto start = std::chrono::steady_clock::now();
    for (int i=0; i<(1<<24); i++) {
        int ax = rand()%width, ay = rand()%height;
        int bx = rand()%width, by = rand()%height;
        line(ax, ay, bx, by, framebuffer, { rand()%255, rand()%255, rand()%255, rand()%255 });
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsedMicroseconds  = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    auto elapsedMilliseconds  = elapsedMicroseconds / 1000.0;
    auto elapsedSeconds  = elapsedMilliseconds / 1000.0;

    std::cout << "Elapsed time:" << std::endl;
    std::cout << "Microseconds: " << elapsedMicroseconds << std::endl;
    std::cout << "Milliseconds: " << elapsedMilliseconds << std::endl;
    std::cout << "Seconds: " << elapsedSeconds << std::endl;

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
