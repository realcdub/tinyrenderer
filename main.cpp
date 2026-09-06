#include <cmath>
#include <ctime>
#include <chrono>
#include <iostream>
#include <numbers>
#include <string>
#include "tgaimage.h"
#include "geometry.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

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

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    // Half the magnitude of the cross product of vectors AB and AC
    return ((bx - ax) * (cy - ay) - (cx - ax) * (by - ay)) * 0.5;
}

static void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &zbuffer, TGAImage &framebuffer, TGAColor color) {
    int bounding_box_min_x = std::min(std::min(ax, bx), cx);
    int bounding_box_min_y = std::min(std::min(ay, by), cy);
    int bounding_box_max_x = std::max(std::max(ax, bx), cx);
    int bounding_box_max_y = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);

    #pragma omp parallel for
    for (int x = bounding_box_min_x; x <= bounding_box_max_x; ++x) {
        for (int y = bounding_box_min_y; y <= bounding_box_max_y; ++y) {
            /*
             Cross product is not commutative, so the ordering must follow A -> B -> C, 
             which was defined by the ordering of the total_area calculation
            */
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;

            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            // Render triangle "wireframe"
            // if (alpha > 0.1 && beta > 0.1 && gamma > 0.1) continue;

            uint8_t z = alpha * az + beta * bz + gamma * cz;
            if (zbuffer.get(x, y).bgra[0] >= z) continue;
            zbuffer.set(x, y, {z});
            framebuffer.set(x, y, color);
        }
    }
}

static vec3 rotate_vector_y(const vec3& vec, float angle)
{
    return {
        vec.x * std::cos(angle) + vec.z * std::sin(angle),
        vec.y,
        -vec.x * std::sin(angle) + vec.z * std::cos(angle)
    };
}

static void render_object(std::string file_name, TGAImage &zbuffer, TGAImage &framebuffer) {
    std::ifstream obj_file(file_name);
    std::string file_line;
    std::vector<vec3> all_vertices{};

    while (std::getline(obj_file, file_line)) {
        if (file_line.size() == 0) continue;
        std::istringstream iss(file_line);

        std::string identifier;
        iss >> identifier;

        if (identifier == "v") {
            float x_normalized, y_normalized, z_normalized;
            iss >> x_normalized;
            iss >> y_normalized;
            iss >> z_normalized;

            vec3 new_vertex{x_normalized, y_normalized, z_normalized};
            new_vertex = rotate_vector_y(new_vertex, 3.14159265358979 / 6);

            new_vertex.x = (framebuffer.width() / 2.0)  * (new_vertex.x + 1);
            new_vertex.y = (framebuffer.height() / 2.0) * (new_vertex.y + 1);
            new_vertex.z = (new_vertex.z + 1) * (255 / 2.0);

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
            
            vec3 first_vertex = all_vertices.at(vertex_indices[0] - 1);
            vec3 second_vertex = all_vertices.at(vertex_indices[1] - 1);
            vec3 third_vertex = all_vertices.at(vertex_indices[2] - 1);

            // line(first_vertex.x, first_vertex.y, second_vertex.x, second_vertex.y, framebuffer, red);
            // line(first_vertex.x, first_vertex.y, third_vertex.x, third_vertex.y, framebuffer, red);
            // line(second_vertex.x, second_vertex.y, third_vertex.x, third_vertex.y, framebuffer, red);
            TGAColor random_color;
            for (int i = 0; i < 3; ++i) {
                random_color.bgra[i] = std::rand() % 255;
            }

            triangle(first_vertex.x, first_vertex.y, first_vertex.z, second_vertex.x, second_vertex.y, second_vertex.z, third_vertex.x, third_vertex.y, third_vertex.z, zbuffer, framebuffer, random_color);
        }
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 256;
    constexpr int height = 256;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    // triangle(7, 45, 35, 100, 45,  60, 255, 255, 255, framebuffer, red);
    // triangle(120, 35, 90, 5, 45, 110, 255, 255, 255, framebuffer, white);
    // triangle(115, 83, 80, 90, 85, 120, 255, 255, 255, framebuffer, green); 
    // line(115, 83, 80,  90, framebuffer, green); 

    render_object("..\\obj\\african_head\\african_head.obj", zbuffer, framebuffer);
    // render_object("..\\obj\\diablo3_pose\\diablo3_pose.obj", zbuffer, framebuffer);

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
    zbuffer.write_tga_file("zbuffer.tga");
    return 0;
}
