#ifndef BASE_H
#define BASE_H
#include <cmath>
#include <iostream>
#include <vector>
#include <string>

class Vector3 {
public:
    float x, y, z;

    // Constructor to initialize vector components
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(){}
    // Operator overloads
    Vector3 operator+(const Vector3& rhs) const {
        return {x + rhs.x, y + rhs.y, z + rhs.z};
    }

    Vector3 operator-(const Vector3& rhs) const {
        return {x - rhs.x, y - rhs.y, z - rhs.z};
    }

    Vector3 operator*(float scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }

    // Static member functions for operations that involve two Vector3 objects
    static float dot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    static Vector3 normalize(const Vector3& v) {
        float magnitude = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        return {v.x / magnitude, v.y / magnitude, v.z / magnitude};
    }
    static float lengthSquared(const Vector3& v) {
        return v.x * v.x + v.y * v.y + v.z * v.z;
    }
    static float projectAlongAxis(const Vector3& point, const Vector3& axis) {
        return Vector3::dot(point, Vector3::normalize(axis));
    }
};
class Matrix4x4 {
public:
    float m[4][4];

    Vector3 operator*(const Vector3& v) const;
};

class Shape {
public:
    virtual std::string getType() const = 0;
};

class Sphere : public Shape {
public:
    Vector3 center;
    float radius;
    
    std::string getType() const override {
        return "sphere";
    }
};

class Cylinder : public Shape {
public:
    Vector3 center;
    Vector3 axis;
    float radius, height;
    
    std::string getType() const override {
        return "cylinder";
    }
    Vector3 getTopCenter() const {
        Vector3 normalizedAxis = Vector3::normalize(axis);
        return center + normalizedAxis * (height);
    }

    Vector3 getBottomCenter() const {
        Vector3 normalizedAxis = Vector3::normalize(axis);
        return center - normalizedAxis * (height);
    }
};

class Triangle : public Shape {
public:
    Vector3 v0, v1, v2;
    
    std::string getType() const override {
        return "triangle";
    }
};
struct Color {
    unsigned char r, g, b;
};
#endif // BASE_H