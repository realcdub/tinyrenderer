#include <cmath>
#include <ctime>
#include <chrono>
#include <iostream>
#include <numbers>
#include <string>

#include "tgaimage.h"
#include "geometry.h"
#include "Object.h"

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

    for (int x = ax; x <= bx; ++x) {
        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }

        error += 2 * std::abs(by - ay);
        y += (by > ay ? 1 : -1) * (error > bx - ax);
        error -= (2 * (bx - ax)) * (error > bx - ax);
    }
}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    // Half the magnitude of the cross product of vectors AB and AC
    return ((bx - ax) * (cy - ay) - (cx - ax) * (by - ay)) * 0.5;
}

static void triangle(vec4 screenCoordinates[3], std::vector<float> &zbuffer, TGAImage &framebuffer, TGAColor color) {
    vec4 a{screenCoordinates[0]};
    vec4 b{screenCoordinates[1]};
    vec4 c{screenCoordinates[2]};

    int bounding_box_min_x = std::min(std::min(a.x, b.x), c.x);
    int bounding_box_min_y = std::min(std::min(a.y, b.y), c.y);
    int bounding_box_max_x = std::max(std::max(a.x, b.x), c.x);
    int bounding_box_max_y = std::max(std::max(a.y, b.y), c.y);
    double total_area = signed_triangle_area(a.x, a.y, b.x, b.y, c.x, c.y);

    #pragma omp parallel for
    for (int x = bounding_box_min_x; x <= bounding_box_max_x; ++x) {
        for (int y = bounding_box_min_y; y <= bounding_box_max_y; ++y) {
            /*
             Cross product is not commutative, so the ordering must follow A -> B -> C, 
             which was defined by the ordering of the total_area calculation
            */
            double alpha = signed_triangle_area(x, y, b.x, b.y, c.x, c.y) / total_area;
            double beta = signed_triangle_area(x, y, c.x, c.y, a.x, a.y) / total_area;
            double gamma = signed_triangle_area(x, y, a.x, a.y, b.x, b.y) / total_area;

            if (alpha < 0 || beta < 0 || gamma < 0) continue;
            int index{x + y * framebuffer.width()};

            if (index < 0 || index > zbuffer.size() - 1) continue;

            float z = alpha * a.z + beta * b.z + gamma * c.z;
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

static void render_object(Object& object, std::vector<float> &zbuffer, TGAImage &framebuffer)
{
    const vec3 eye{0, 0, 0.6};
    const vec3 center{0, 0, 0};
    const vec3 up{0, 1, 0};

    const double focus{(eye - center).magnitude()};
    const int width{framebuffer.width()};
    const int height{framebuffer.height()};

    mat4 ModelView{look_at(eye, center, up)};
    mat4 Projection{central_projection(focus)};
    mat4 Viewport{viewport(width, height)};

    for (int face = 0; face < object.GetNumberOfFaces(); ++face)
    {

        vec4 ndc[3];
        for (int vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
        {
            vec3 currentVertex{object.GetVertex(face, vertexIndex)};
            vec4 transformedVertex{Projection * ModelView * vec4(currentVertex.x, currentVertex.y, currentVertex.z, 1)};
            ndc[vertexIndex] = {transformedVertex.x / transformedVertex.w, transformedVertex.y / transformedVertex.w, transformedVertex.z / transformedVertex.w, 1};
        }

        vec4 screenCoordinates[3] = {Viewport * ndc[0], Viewport * ndc[1], Viewport * ndc[2]};

        TGAColor random_color;
        for (int i = 0; i < 3; ++i) {
            random_color.bgra[i] = std::rand() % 255;
        }

        triangle(screenCoordinates, zbuffer, framebuffer, random_color);
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 800;
    constexpr int height = 800;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<float> zbuffer(width * height, -1000);

    Object object{"..\\obj\\african_head\\african_head.obj"};
    render_object(object, zbuffer, framebuffer);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
