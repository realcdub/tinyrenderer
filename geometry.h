#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>
#include <iostream>

struct mat4;
// ============================================================================
// 2D Vector
// ============================================================================
struct vec2 {
    double x{0.0};
    double y{0.0};

    vec2() = default;
    vec2(double x, double y) : x(x), y(y) {}

    // Vector arithmetic
    vec2 operator+(const vec2& v) const { return {x + v.x, y + v.y}; }
    vec2 operator-(const vec2& v) const { return {x - v.x, y - v.y}; }
    vec2 operator*(double s) const      { return {x * s, y * s}; }
    vec2 operator/(double s) const      { return {x / s, y / s}; }

    vec2& operator+=(const vec2& v) { x += v.x; y += v.y; return *this; }
    vec2& operator-=(const vec2& v) { x -= v.x; y -= v.y; return *this; }
    vec2& operator*=(double s)      { x *= s;   y *= s;   return *this; }
    vec2& operator/=(double s)      { x /= s;   y /= s;   return *this; }

    // Scalar on left-hand side: 2.0 * v
    friend vec2 operator*(double s, const vec2& v) { return v * s; }

    // Products & Metrics
    double dot(const vec2& v) const { return x * v.x + y * v.y; }
    double norm() const             { return std::sqrt(dot(*this)); }
    vec2 normalized() const {
        double n = norm();
        return n > 0.0 ? (*this) / n : *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const vec2& v) {
        return os << "(" << v.x << ", " << v.y << ")";
    }
};

// ============================================================================
// 3D Vector
// ============================================================================
struct vec3 {
    double x{0.0};
    double y{0.0};
    double z{0.0};

    vec3() = default;
    vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    // Vector arithmetic
    vec3 operator+(const vec3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    vec3 operator-(const vec3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    vec3 operator*(double s) const      { return {x * s, y * s, z * s}; }
    vec3 operator/(double s) const      { return {x / s, y / s, z / s}; }

    vec3& operator+=(const vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    vec3& operator-=(const vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    vec3& operator*=(double s)      { x *= s;   y *= s;   z *= s;   return *this; }
    vec3& operator/=(double s)      { x /= s;   y /= s;   z /= s;   return *this; }

    friend vec3 operator*(double s, const vec3& v) { return v * s; }

    // Products & Metrics
    double dot(const vec3& v) const { return x * v.x + y * v.y + z * v.z; }

    // Used heavily for triangle face normals and barycentric weights
    vec3 cross(const vec3& v) const {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }

    double norm() const { return std::sqrt(dot(*this)); }
    double magnitude() const { return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2)); }
    vec3 normalized() const {
        double n = norm();
        return n > 0.0 ? (*this) / n : *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const vec3& v) {
        return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    }
};

// ============================================================================
// 4D Vector (Homogeneous coordinates for projection/transformations)
// ============================================================================
struct vec4 {
    double x{0.0};
    double y{0.0};
    double z{0.0};
    double w{1.0}; // Default homogeneous coordinate

    vec4() = default;
    vec4(double x, double y, double z, double w = 1.0) : x(x), y(y), z(z), w(w) {}
    vec4(const vec3& v, double w = 1.0) : x(v.x), y(v.y), z(v.z), w(w) {}

    vec4 operator+(const vec4& v) const { return {x + v.x, y + v.y, z + v.z, w + v.w}; }
    vec4 operator-(const vec4& v) const { return {x - v.x, y - v.y, z - v.z, w - v.w}; }
    vec4 operator*(double s) const      { return {x * s, y * s, z * s, w * s}; }
    vec4 operator/(double s) const      { return {x / s, y / s, z / s, w / s}; }

    vec4& operator+=(const vec4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    vec4& operator-=(const vec4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    vec4& operator*=(double s)      { x *= s;   y *= s;   z *= s;   w *= s;   return *this; }
    vec4& operator/=(double s)      { x /= s;   y /= s;   z /= s;   w /= s;   return *this; }

    friend vec4 operator*(double s, const vec4& v) { return v * s; }

    double dot(const vec4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

    // Perspective divide: project back to 3D Cartesian space
    vec3 to_cartesian() const {
        return (w != 0.0) ? vec3(x / w, y / w, z / w) : vec3(x, y, z);
    }

    friend std::ostream& operator<<(std::ostream& os, const vec4& v) {
        return os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    }
};

// 4D Matrix
struct mat4
{
    vec4 cols[4];

    mat4()
    {
        cols[0] = {1, 0, 0, 0};
        cols[1] = {0, 1, 0, 0};
        cols[2] = {0, 0, 1, 0};
        cols[3] = {0, 0, 0, 1};
    }

    mat4(const vec4& first, const vec4& second, const vec4& third, const vec4& fourth)
    {
        cols[0] = first;
        cols[1] = second;
        cols[2] = third;
        cols[3] = fourth;
    }

    mat4 operator*(mat4 m) const
    {
        mat4 mat;
        mat.cols[0] = {m.cols[0].x * cols[0] + m.cols[0].y * cols[1] + m.cols[0].z * cols[2] + m.cols[0].w * cols[3]};
        mat.cols[1] = {m.cols[1].x * cols[0] + m.cols[1].y * cols[1] + m.cols[1].z * cols[2] + m.cols[1].w * cols[3]};
        mat.cols[2] = {m.cols[2].x * cols[0] + m.cols[2].y * cols[1] + m.cols[2].z * cols[2] + m.cols[2].w * cols[3]};
        mat.cols[3] = {m.cols[3].x * cols[0] + m.cols[3].y * cols[1] + m.cols[3].z * cols[2] + m.cols[3].w * cols[3]};
        return mat;
    }

    vec4 operator*(const vec4& v) const
    {
        return v.x * cols[0] + v.y * cols[1] + v.z * cols[2] + v.w * cols[3];
    }

    // friend mat4 operator*(vec4& v, mat4& m) { return (m * v); };
};

#endif // GEOMETRY_H