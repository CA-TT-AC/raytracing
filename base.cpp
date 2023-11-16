#include "base.h"
#include <cmath>
// Function to compute a point along the ray using operator overloading
Vector3 Matrix4x4::operator*(const Vector3 &v) const
{
    // Homogenize the 3D vector to a 4D vector
    float x = v.x, y = v.y, z = v.z, w = 1.0f;

    // Perform the multiplication
    float resultX = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3] * w;
    float resultY = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3] * w;
    float resultZ = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3] * w;
    float resultW = m[3][0] * x + m[3][1] * y + m[3][2] * z + m[3][3] * w;

    // De-homogenize the result (if resultW is not 0)
    if (resultW != 0.0f)
    {
        resultX /= resultW;
        resultY /= resultW;
        resultZ /= resultW;
    }

    return {resultX, resultY, resultZ};
}

Vector3 normalize(const Vector3 &v)
{
    float magnitude = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return {v.x / magnitude, v.y / magnitude, v.z / magnitude};
}
void Cylinder::getTextureCoordinates(const Vector3 &point, int &u, int &v) const
{
    Vector3 p = (point - center).normalize();
    float phi = atan2(p.z, p.x);

    u = static_cast<int>((phi + M_PI) / (2 * M_PI) * texture->width);

    // For the vertical position (v), we map the height of the cylinder to the texture's height
    float heightPosition = Vector3::dot(point - center, axis);
    v = static_cast<int>((heightPosition / height) * texture->height);
}
void Sphere::getTextureCoordinates(const Vector3 &point, int &u, int &v) const
{
    Vector3 p = (point - center).normalize();
    float phi = atan2(p.z, p.x);
    float theta = acos(p.y);

    u = static_cast<int>((phi + M_PI) / (2 * M_PI) * texture->width);
    v = static_cast<int>(theta / M_PI * texture->height);
}