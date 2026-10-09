#include <cmath>
#include <ctime>
#include <chrono>
#include <iostream>
#include <numbers>
#include <string>
#include "tgaimage.h"
#include "geometry.h"

constexpr TGAColor white   = {255, 255, 255, 255};
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

static void triangle(int ax, int ay, float az, int bx, int by, float bz, int cx, int cy, float cz, std::vector<float> &zbuffer, TGAImage &framebuffer, TGAColor color) {
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
            int index{x + y * framebuffer.width()};

            if (index < 0 || index > zbuffer.size() - 1) continue;

            float z = alpha * az + beta * bz + gamma * cz;
            if (zbuffer.at(index) >= z) continue;

            zbuffer[index] = z;
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

static mat4 central_projection(double c)
{
    return {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, -1 / c},
        {0, 0, 0, 1}
    };
}

static mat4 look_at(const vec3& eye, const vec3& center, const vec3& up)
{
    vec3 n{(eye - center).normalized()};
    vec3 l{up.cross(n).normalized()};
    vec3 m{n.cross(l).normalized()};

    mat4 ModelView =
        mat4( {l.x, m.x, n.x, 0}, {l.y, m.y, n.y, 0}, {l.z, m.z, n.z, 0}, {0, 0, 0, 1})
        * mat4( {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {-eye.x, -eye.y, -eye.z, 1});

    return ModelView;
}

static mat4 viewport(int width, int height)
{
    return {
        {width / 2.0, 0, 0, 0},
        {0, height / 2.0, 0, 0},
        {0, 0, 1, 0},
        {width / 2.0, height / 2.0, 0, 1}
    };
}

static void render_object(std::string file_name, std::vector<float> &zbuffer, TGAImage &framebuffer)
{
    std::ifstream obj_file(file_name);
    std::string file_line;
    std::vector<vec3> all_vertices{};

    const vec3 eye{0.35, 0, 0.6};
    const vec3 center{0, 0, 0};
    const vec3 up{0, 1, 0};
    const double focus{(eye - center).magnitude()};
    const int width{framebuffer.width()};
    const int height{framebuffer.height()};

    mat4 ModelView{look_at(eye, center, up)};
    mat4 Projection{central_projection(focus)};
    mat4 Viewport{viewport(width, height)};

    while (std::getline(obj_file, file_line)) {
        if (file_line.empty()) continue;
        std::istringstream iss(file_line);

        std::string identifier;
        iss >> identifier;

        if (identifier == "v") {
            float x_normalized, y_normalized, z_normalized;
            iss >> x_normalized;
            iss >> y_normalized;
            iss >> z_normalized;

            vec3 new_vertex{x_normalized, y_normalized, z_normalized};

            vec4 transformed_vertex{Projection * ModelView * vec4(new_vertex.x, new_vertex.y, new_vertex.z, 1)};
            vec4 ndc{transformed_vertex.x / transformed_vertex.w, transformed_vertex.y / transformed_vertex.w, transformed_vertex.z / transformed_vertex.w, 1};

            vec4 screen{Viewport * ndc};
            new_vertex = vec3(screen.x, screen.y, screen.z);

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

            TGAColor random_color;
            for (int i = 0; i < 3; ++i) {
                random_color.bgra[i] = std::rand() % 255;
            }

            triangle(first_vertex.x, first_vertex.y, first_vertex.z, second_vertex.x, second_vertex.y, second_vertex.z, third_vertex.x, third_vertex.y, third_vertex.z, zbuffer, framebuffer, random_color);
        }
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 800;
    constexpr int height = 800;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<float> zbuffer(width * height, -1000);

    render_object("..\\obj\\african_head\\african_head.obj", zbuffer, framebuffer);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
