#include "base.h"
// Function to compute a point along the ray using operator overloading
Vector3 operator+(const Vector3& lhs, const Vector3& rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

Vector3 operator*(const Vector3& v, float scalar) {
    return { v.x * scalar, v.y * scalar, v.z * scalar };
}

Vector3 operator-(const Vector3& lhs, const Vector3& rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}
float dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}