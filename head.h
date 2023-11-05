#include <iostream>
#include <vector>
#include <string>
#include "base.h"

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

class Ray {
public:
    Vector3 origin;
    Vector3 direction;

    Ray(const Vector3& origin, const Vector3& direction)
        : origin(origin), direction(direction) {}
    
    // Function to compute a point along the ray
    Vector3 at(float t) const {
        return origin + direction * t;
    }
};


class Camera {
public:
    std::string type;
    int width, height;
    Vector3 position;
    Vector3 lookAt;
    Vector3 upVector;
    float fov;
    float exposure;
};

class Scene {
public:
    Color backgroundColor;
    std::vector<Shape*> shapes;
};

class Renderer {
public:
    std::string renderMode;
    Camera camera;
    Scene scene;
    void loadFromJSON(const std::string& filename);
    std::vector<std::vector<bool>> renderBinary();


    std::vector<std::vector<Color>> renderColor();
    void writeBinaryImageToPPM(const std::vector<std::vector<bool>>& image, const std::string& filename);
    void writeColorImageToPPM(const std::vector<std::vector<Color>>& image, const std::string& filename);
private:
    Ray computeRay(int x, int y);

    bool intersect(const Ray& ray, Shape* shape);
};
/*
int main() {
    // Create a renderer, camera, and scene, and populate them with data
    Renderer renderer;
    renderer.renderMode = "binary";
    
    renderer.camera.type = "pinhole";
    renderer.camera.width = 1200;
    renderer.camera.height = 800;
    // ... (and so on for other camera and scene properties)
    
    Sphere sphere;
    sphere.center = { -0.3f, 0.19f, 1.0f };
    sphere.radius = 0.2f;
    
    renderer.scene.shapes.push_back(&sphere);
    // ... (and similarly for other shapes)
    
    return 0;
}*/