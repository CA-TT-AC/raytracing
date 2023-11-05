#ifndef BASE_H
#define BASE_H

#include <iostream>
#include <vector>
#include <string>

struct Vector3 {
    float x, y, z;
};
Vector3 operator+(const Vector3& lhs, const Vector3& rhs);
Vector3 operator*(const Vector3& v, float scalar);
Vector3 operator-(const Vector3& lhs, const Vector3& rhs);
float dot(const Vector3& a, const Vector3& b);
Vector3 cross(const Vector3& a, const Vector3& b);

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