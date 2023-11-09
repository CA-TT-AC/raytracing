#include <iostream>
#include <vector>
#include <string>
#include "base.h"


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
    std::vector<LightSource*> lights; // Container for light sources

    ~Scene() {
        // Destructor to clean up allocated Shapes
        for (auto* shape : shapes) {
            delete shape;
        }
        // Destructor to clean up allocated LightSources
        for (auto* light : lights) {
            delete light;
        }
    }

    // Add a method to add lights to the scene
    void addLight(LightSource* light) {
        lights.push_back(light);
    }
};


class Renderer {
public:
    std::string renderMode;
    Camera camera;
    Scene scene;
    void loadFromJSON(const std::string& filename);

    // render part
    std::vector<std::vector<Color>> render();
    std::vector<std::vector<Color>> renderBinary();
    std::vector<std::vector<Color>> renderPhong();
    void writeColorImageToPPM(const std::vector<std::vector<Color>>& image, const std::string& filename);
private:
    Ray computeRay(int x, int y);

    bool intersect(const Ray& ray, Shape* shape);
};