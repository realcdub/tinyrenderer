#include <cmath>
#include "tgaimage.h"
#include <ctime>
#include <chrono>
#include <iostream>
#include <string>

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

typedef struct {
   int x; 
   int y; 
} vertex;

static void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
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

        error += 2 * std::abs(by - ay);
        y += (by > ay ? 1 : -1) * (error > bx - ax);
        error -= (2 * (bx - ax)) * (error > bx - ax);
        // y += slope;
    }
}

static void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    if (ay > by) { std::swap(ax, bx); std::swap(ay, by); }
    if (ay > cy) { std::swap(ax, cx); std::swap(ay, cy); }
    if (by > cy) { std::swap(bx, cx); std::swap(by, cy); }

    for (int y = ay; y < by; ++y) {
        int x1 = ax + (cx - ax) * (y - ay) / (cy - ay);
        int x2 = ax + (bx - ax) * (y - ay) / (by - ay);

        line(x1, y, x2, y, framebuffer, color);
    }

    for (int y = by; y < cy; ++y) {
        int x1 = bx + (cx - bx) * (y - by) / (cy - by);
        int x2 = ax + (cx - ax) * (y - ay) / (cy - ay);
        
        line(x1, y, x2, y, framebuffer, color);
    }

    // line(ax, ay, bx, by, framebuffer, color);
    // line(bx, by, cx, cy, framebuffer, color);
    // line(cx, cy, ax, ay, framebuffer, color);
}

static void render_object(std::string file_name, TGAImage &framebuffer) {
    std::ifstream obj_file(file_name);
    std::string file_line;
    std::vector<vertex> all_vertices{};

    while (std::getline(obj_file, file_line)) {
        if (file_line.size() == 0) continue;
        std::istringstream iss(file_line);

        std::string identifier;
        iss >> identifier;

        if (identifier == "v") {
            vertex new_vertex{};

            float x_normalized, y_normalized;
            iss >> x_normalized;
            iss >> y_normalized;

            new_vertex.x = (framebuffer.width() / 2) + (framebuffer.width() / 2) * x_normalized;
            new_vertex.y = (framebuffer.height() / 2) + (framebuffer.height() / 2) * y_normalized;

            all_vertices.push_back(new_vertex);
        } else if (identifier == "f") {
            std::string face[3];
            int vertex_indices[3] = {};

            iss >> face[0] >> face[1] >> face[2];

            for (int i = 0; i < 3; ++i) {
                std::size_t found_slash_index = face[i].find("/");
                if (found_slash_index != std::string::npos) {
                    vertex_indices[i] = std::stoi(face[i].substr(0, found_slash_index));
                }
            }
            
            vertex first_vertex = all_vertices.at(vertex_indices[0] - 1); 
            vertex second_vertex = all_vertices.at(vertex_indices[1] - 1); 
            vertex third_vertex = all_vertices.at(vertex_indices[2] - 1); 

            line(first_vertex.x, first_vertex.y, second_vertex.x, second_vertex.y, framebuffer, red);
            line(first_vertex.x, first_vertex.y, third_vertex.x, third_vertex.y, framebuffer, red);
            line(second_vertex.x, second_vertex.y, third_vertex.x, third_vertex.y, framebuffer, red);
        }
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 128;
    constexpr int height = 128;

    TGAImage framebuffer(width, height, TGAImage::RGB);

    triangle(  7, 45, 35, 100, 45,  60, framebuffer, red);
    triangle(120, 35, 90,   5, 45, 110, framebuffer, white);
    triangle(115, 83, 80,  90, 85, 120, framebuffer, green); 

    // render_object("..\\obj\\african_head\\african_head.obj", framebuffer);

    // std::srand(std::time({}));
    // auto start = std::chrono::steady_clock::now();
    // for (int i=0; i<(1<<24); i++) {
    //     int ax = std::rand()%width, ay = std::rand()%height;
    //     int bx = std::rand()%width, by = std::rand()%height;
    //     line(ax, ay, bx, by, framebuffer, { std::rand()%255, std::rand()%255, std::rand()%255, std::rand()%255 });
    // }
    // auto end = std::chrono::steady_clock::now();
    // auto elapsedMicroseconds  = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    // auto elapsedMilliseconds  = elapsedMicroseconds / 1000.0;
    // auto elapsedSeconds  = elapsedMilliseconds / 1000.0;

    // std::cout << "Elapsed time:" << std::endl;
    // std::cout << "Microseconds: " << elapsedMicroseconds << std::endl;
    // std::cout << "Milliseconds: " << elapsedMilliseconds << std::endl;
    // std::cout << "Seconds: " << elapsedSeconds << std::endl;

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
