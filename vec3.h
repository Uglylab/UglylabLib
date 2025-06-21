#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

struct Vec3 {
    float x = 0, y = 0, z = 0;

    // Constructor
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Vector addition
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    // Vector subtraction
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    // Scalar multiplication
    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    // Scalar division
    Vec3 operator/(float scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    // Unary minus
    Vec3 operator-() const {
        return Vec3(-x, -y, -z);
    }

    // Compound assignment
    Vec3& operator+=(const Vec3& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }

    Vec3& operator-=(const Vec3& other) {
        x -= other.x; y -= other.y; z -= other.z;
        return *this;
    }

    Vec3& operator*=(float scalar) {
        x *= scalar; y *= scalar; z *= scalar;
        return *this;
    }

    Vec3& operator/=(float scalar) {
        x /= scalar; y /= scalar; z /= scalar;
        return *this;
    }

    // Length (magnitude)
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Squared length (faster)
    float lengthSquared() const {
        return x * x + y * y + z * z;
    }

    // Normalize (returns a copy)
    Vec3 normalized() const {
        float len = length();
        return (len > 0) ? (*this / len) : Vec3();
    }

    // Dot product
    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // Cross product
    Vec3 cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
            );
    }

    // Output stream
    friend std::ostream& operator<<(std::ostream& os, const Vec3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
};

#endif // VEC3_H

