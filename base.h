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

    Vector3 operator-() const {
        return {-x, -y, -z};
    }
    Vector3 normalize() {
        float magnitude = sqrt(x * x + y * y + z * z);
        Vector3 ret;
        // Check for divide by zero
        if (magnitude > 0) {
            ret.x = x / magnitude;
            ret.y = y / magnitude;
            ret.z = z / magnitude;
        }
        return ret;
    }
    float dot(const Vector3 v) {
        return x * v.x + y * v.y + z * v.z;
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
    static float length(const Vector3& v) {
        return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }
};
class Matrix4x4 {
public:
    float m[4][4];

    Vector3 operator*(const Vector3& v) const;
};
class Color {
public:
    float r, g, b;

    // Default constructor for black color
    Color() : r(0.0f), g(0.0f), b(0.0f) {}

    // Constructor with floats (range between 0.0 and 1.0)
    Color(float red, float green, float blue) : r(red), g(green), b(blue) {}

    // Get color as 8-bit integers
    void getAsIntegers(unsigned char &red, unsigned char &green, unsigned char &blue) const {
        red = static_cast<unsigned char>(r * 255);
        green = static_cast<unsigned char>(g * 255);
        blue = static_cast<unsigned char>(b * 255);
    }

    // Overload the assignment operator to allow initialization with integer values
    Color& operator=(const Color& other) {
        if (this != &other) {
            r = other.r;
            g = other.g;
            b = other.b;
        }
        return *this;
    }
    // Addition of two Colors (component-wise)
    Color operator+(const Color& other) const {
        return Color(r + other.r, g + other.g, b + other.b);
    }

    // Subtraction of two Colors (component-wise)
    Color operator-(const Color& other) const {
        return Color(r - other.r, g - other.g, b - other.b);
    }

    // Multiplication with a scalar (each component is scaled by the scalar)
    Color operator*(float scalar) const {
        return Color(r * scalar, g * scalar, b * scalar);
    }

    // Multiplication (component-wise) of two Colors
    Color operator*(const Color& other) const {
        return Color(r * other.r, g * other.g, b * other.b);
    }

    // Compound assignment operators for efficiency
    Color& operator+=(const Color& other) {
        r += other.r;
        g += other.g;
        b += other.b;
        return *this;
    }

    Color& operator-=(const Color& other) {
        r -= other.r;
        g -= other.g;
        b -= other.b;
        return *this;
    }

    Color& operator*=(float scalar) {
        r *= scalar;
        g *= scalar;
        b *= scalar;
        return *this;
    }

    Color& operator*=(const Color& other) {
        r *= other.r;
        g *= other.g;
        b *= other.b;
        return *this;
    }
    // Clamps color values to the range [0, 1]
    void clamp() {
        r = std::max(0.0f, std::min(1.0f, r));
        g = std::max(0.0f, std::min(1.0f, g));
        b = std::max(0.0f, std::min(1.0f, b));
    }
};


class Material {
public:
    float ks;  // Specular coefficient
    float kd;  // Diffuse coefficient
    int specularExponent;
    Color diffuseColor;
    Color specularColor;
    Color ambientColor;
    bool isReflective;
    float reflectivity;
    bool isRefractive;
    float refractiveIndex;

    Material() : ks(0), kd(0), specularExponent(0), isReflective(false), reflectivity(0), isRefractive(false), refractiveIndex(1.0f) {}
};

class Shape {
public:
    Material material;
    virtual std::string getType() const = 0;
    virtual Vector3 getNormal(const Vector3& point) const = 0;
};

class Sphere : public Shape {
public:
    Vector3 center;
    float radius;
    
    std::string getType() const override {
        return "sphere";
    }
    Vector3 getNormal(const Vector3& p) const {
        Vector3 outwardNormal = p - center; // Vector from the center of the sphere to the point p
        return Vector3::normalize(outwardNormal); // Normalize this vector to get the normal
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
        return center + normalizedAxis * (height * 0.5f);
    }

    Vector3 getBottomCenter() const {
        Vector3 normalizedAxis = Vector3::normalize(axis);
        return center - normalizedAxis * (height * 0.5f);
    }
    Vector3 getNormal(const Vector3& p) const {
        Vector3 normalizedAxis = Vector3::normalize(axis);
        float halfHeight = height * 0.5f;
        float projection = Vector3::dot((p - center), normalizedAxis);
        if (std::abs(projection) > halfHeight) {
            // The point is on the top or bottom cap
            return (projection < 0 ? -normalizedAxis : normalizedAxis);
        } else {
            // The point is on the side, calculate the normal for the curved surface
            Vector3 onAxis = center + normalizedAxis * projection; // The closest point on the axis to p
            return Vector3::normalize(p - onAxis);
        }
    }

};

class Triangle : public Shape {
public:
    Vector3 v0, v1, v2;
    
    std::string getType() const override {
        return "triangle";
    }
    Vector3 getNormal(const Vector3& point) const override {
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        return Vector3::normalize(Vector3::cross(edge1, edge2)); // The normal is the cross product of two edges of the triangle
    }
};


class LightSource {
public:
    Vector3 position;
    Color intensity;

    // Default constructor
    LightSource() 
        : position(Vector3{0, 0, 0}), intensity(Color{1, 1, 1}) {}

    // Parameterized constructor
    LightSource(const Vector3& pos, const Color& inten) 
        : position(pos), intensity(inten) {}

    // You can add methods for light behavior here, if necessary.
};


#endif // BASE_H