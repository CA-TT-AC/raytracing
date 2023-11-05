#include <vector>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "head.h"
#include "json.hpp"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Renderer::loadFromJSON(const std::string& filename) {
    printf("Start loadFromJSON....\n");
    // Open the file and parse the JSON
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    nlohmann::json json;
    file >> json;

    // Load render mode
    if (json.contains("rendermode")) {
        renderMode = json["rendermode"];
    }

    // Load camera data
    if (json.contains("camera")) {
        nlohmann::json cameraJson = json["camera"];
        camera.type = cameraJson["type"];
        camera.width = cameraJson["width"];
        camera.height = cameraJson["height"];
        camera.position = { 
            cameraJson["position"][0], 
            cameraJson["position"][1], 
            cameraJson["position"][2] 
        };
        camera.lookAt = { 
            cameraJson["lookAt"][0], 
            cameraJson["lookAt"][1], 
            cameraJson["lookAt"][2] 
        };
        camera.upVector = { 
            cameraJson["upVector"][0], 
            cameraJson["upVector"][1], 
            cameraJson["upVector"][2] 
        };
        camera.fov = cameraJson["fov"];
        camera.exposure = cameraJson["exposure"];
    }

    // Load scene data
    if (json.contains("scene")) {
        nlohmann::json sceneJson = json["scene"];
        scene.backgroundColor = {
            sceneJson["backgroundcolor"][0],
            sceneJson["backgroundcolor"][1],
            sceneJson["backgroundcolor"][2]
        };
        // Load shapes
        for (const auto& shapeJson : sceneJson["shapes"]) {
            std::string type = shapeJson["type"];
            if (type == "sphere") {
                Sphere* sphere = new Sphere();
                sphere->center = { shapeJson["center"][0], shapeJson["center"][1], shapeJson["center"][2] };
                sphere->radius = shapeJson["radius"];
                scene.shapes.push_back(sphere);
            } else if (type == "cylinder") {
                Cylinder* cylinder = new Cylinder();
                cylinder->center = { shapeJson["center"][0], shapeJson["center"][1], shapeJson["center"][2] };
                cylinder->axis = { shapeJson["axis"][0], shapeJson["axis"][1], shapeJson["axis"][2] };
                cylinder->radius = shapeJson["radius"];
                cylinder->height = shapeJson["height"];
                scene.shapes.push_back(cylinder);
            } else if (type == "triangle") {
                Triangle* triangle = new Triangle();
                triangle->v0 = { shapeJson["v0"][0], shapeJson["v0"][1], shapeJson["v0"][2] };
                triangle->v1 = { shapeJson["v1"][0], shapeJson["v1"][1], shapeJson["v1"][2] };
                triangle->v2 = { shapeJson["v2"][0], shapeJson["v2"][1], shapeJson["v2"][2] };
                scene.shapes.push_back(triangle);
            }
        }
    }
    printf("Success loadFromJSON!\n");
}

Ray Renderer::computeRay(int x, int y) {
    // Normalize the pixel coordinates to [-1, 1]
    float normalizedX = (2.0f * x / camera.width - 1.0f);
    float normalizedY = (1.0f - 2.0f * y / camera.height);

    // Compute the direction vector based on the FOV
    float tanFov = tan(camera.fov * M_PI / 360.0f);  // Half FOV in radians
    Vector3 direction = {
        normalizedX * tanFov * camera.width / camera.height,
        normalizedY * tanFov,
        1.0f  // Assuming the image plane is at z = 1 in a left-handed system
    };

    // TODO: Apply any camera transformations (rotation, translation) to the direction vector

    return Ray(camera.position, direction);
}

bool Renderer::intersect(const Ray& ray, Shape* shape) {
    if (shape->getType() == "sphere") {
        Sphere* sphere = static_cast<Sphere*>(shape);
        Vector3 oc = ray.origin - sphere->center;
        float a = dot(ray.direction, ray.direction);
        float b = 2.0f * dot(oc, ray.direction);
        float c = dot(oc, oc) - sphere->radius * sphere->radius;
        float discriminant = b * b - 4 * a * c;
        return (discriminant > 0);
    }
    else if (shape->getType() == "cylinder") {
        Cylinder* cylinder = static_cast<Cylinder*>(shape);
        Vector3 oc = ray.origin - cylinder->center;
        float a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z;
        float b = 2.0f * (oc.x * ray.direction.x + oc.z * ray.direction.z);
        float c = oc.x * oc.x + oc.z * oc.z - cylinder->radius * cylinder->radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0) {
            return false;
        } else {
            float t = (-b - sqrt(discriminant)) / (2.0f * a);
            float y = ray.origin.y + t * ray.direction.y;
            float yMin = cylinder->center.y - cylinder->height / 2.0f;
            float yMax = cylinder->center.y + cylinder->height / 2.0f;
            if (y >= yMin && y <= yMax) {
                return true;
            }
        }
    }
    else if (shape->getType() == "triangle") {
        Triangle* triangle = static_cast<Triangle*>(shape);
        Vector3 edge1 = triangle->v1 - triangle->v0;
        Vector3 edge2 = triangle->v2 - triangle->v0;
        Vector3 pvec = cross(ray.direction, edge2);
        float det = dot(edge1, pvec);
        if (fabs(det) < 1e-8) {
            return false;  // Ray is parallel to the triangle
        }
        float invDet = 1.0f / det;
        Vector3 tvec = ray.origin - triangle->v0;
        float u = dot(tvec, pvec) * invDet;
        if (u < 0.0f || u > 1.0f) {
            return false;
        }
        Vector3 qvec = cross(tvec, edge1);
        float v = dot(ray.direction, qvec) * invDet;
        if (v < 0.0f || u + v > 1.0f) {
            return false;
        }
        return true;  // Ray intersects the triangle
    }
    // TODO: Implement intersection tests for other shape types (cylinder, triangle, etc.)
    return false;
}


std::vector<std::vector<bool>> Renderer::renderBinary() {
        std::vector<std::vector<bool>> image(camera.height, std::vector<bool>(camera.width, false));
        
        // Loop over each pixel in the image
        for (int y = 0; y < camera.height; ++y) {
            for (int x = 0; x < camera.width; ++x) {
                // Compute ray for this pixel
                Ray ray = computeRay(x, y);
                
                // Check for intersections with any shape in the scene
                for (Shape* shape : scene.shapes) {
                    
                    if (intersect(ray, shape)) {
                        image[y][x] = true;  // Set pixel to white if there's an intersection
                        break;  // Exit loop once an intersection is found
                    }
                }
            }
        }
        
        return image;
    }

void Renderer::writeBinaryImageToPPM(const std::vector<std::vector<bool>>& image, const std::string& filename) {
        std::ofstream file(filename);
        file << "P5\n" << camera.width << " " << camera.height << "\n255\n";
        for (const auto& row : image) {
            for (bool pixel : row) {
                char value = pixel ? 255 : 0;  // Convert bool to char (255 for true, 0 for false)
                file.write(&value, 1);  // Write one byte for each pixel
            }
        }
    }

void Renderer::writeColorImageToPPM(const std::vector<std::vector<Color>>& image, const std::string& filename) {
        std::ofstream file(filename);
        file << "P6\n" << camera.width << " " << camera.height << "\n255\n";
        for (const auto& row : image) {
            for (Color pixel : row) {
                file.write(reinterpret_cast<char*>(&pixel), 3);  // Write 3 bytes (RGB) for each pixel
            }
        }
    }

std::vector<std::vector<Color>> Renderer::renderColor() {
        std::vector<std::vector<Color>> image(camera.height, std::vector<Color>(camera.width));
        
        for (int y = 0; y < camera.height; ++y) {
            for (int x = 0; x < camera.width; ++x) {
                Ray ray = computeRay(x, y);
                bool hit = false;
                for (Shape* shape : scene.shapes) {
                    if (intersect(ray, shape)) {
                        // Assume white color for intersection
                        image[y][x] = {255, 255, 255};  
                        hit = true;
                        break;
                    }
                }
                if (!hit) {  // No intersection, use background color
                    image[y][x] = {
                        static_cast<unsigned char>(scene.backgroundColor.r * 255),
                        static_cast<unsigned char>(scene.backgroundColor.g * 255),
                        static_cast<unsigned char>(scene.backgroundColor.b * 255)
                    };
                }
            }
        }
        
        return image;
    }