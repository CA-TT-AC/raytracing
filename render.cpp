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
Color operator*(float scalar, const Color& color) {
    return color * scalar; // Utilize the existing Color * float overload
}
Vector3 operator*(float scalar, const Vector3& vec) {
    return vec * scalar; // Utilize the existing Vector3 * float overload
}
void Renderer::loadFromJSON(const std::string& filename) {
    // printf("Start loadFromJSON....\n");
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
        // Load light sources
        if (sceneJson.contains("lightsources")) {
            for (const auto& lightJson : sceneJson["lightsources"]) {
                LightSource* light = new LightSource();
                light->position = { lightJson["position"][0], lightJson["position"][1], lightJson["position"][2] };
                light->intensity = { lightJson["intensity"][0], lightJson["intensity"][1], lightJson["intensity"][2] };
                scene.lights.push_back(light);
            }
        }
        scene.backgroundColor = {
            sceneJson["backgroundcolor"][0],
            sceneJson["backgroundcolor"][1],
            sceneJson["backgroundcolor"][2]
        };
        // Load shapes
        for (const auto& shapeJson : sceneJson["shapes"]) {
            std::string type = shapeJson["type"];
            Material material;
            if (shapeJson.contains("material")) {
                const auto& matJson = shapeJson["material"];
                material.ks = matJson["ks"];
                material.kd = matJson["kd"];
                material.specularExponent = matJson["specularexponent"];
                material.diffuseColor = { matJson["diffusecolor"][0], matJson["diffusecolor"][1], matJson["diffusecolor"][2] };
                material.ambientColor = material.diffuseColor * 0.3f;
                material.specularColor = { matJson["specularcolor"][0], matJson["specularcolor"][1], matJson["specularcolor"][2] };
                material.isReflective = matJson["isreflective"];
                material.reflectivity = matJson["reflectivity"];
                material.isRefractive = matJson["isrefractive"];
                material.refractiveIndex = matJson["refractiveindex"];
            }
            if (type == "sphere") {
                Sphere* sphere = new Sphere();
                sphere->material = material;
                sphere->center = { shapeJson["center"][0], shapeJson["center"][1], shapeJson["center"][2] };
                sphere->radius = shapeJson["radius"];
                scene.shapes.push_back(sphere);
            } else if (type == "cylinder") {
                Cylinder* cylinder = new Cylinder();
                cylinder->material = material;
                cylinder->center = { shapeJson["center"][0], shapeJson["center"][1], shapeJson["center"][2] };
                cylinder->axis = { shapeJson["axis"][0], shapeJson["axis"][1], shapeJson["axis"][2] };
                cylinder->radius = shapeJson["radius"];
                cylinder->height = shapeJson["height"];
                cylinder->height *= 2;
                scene.shapes.push_back(cylinder);
            } else if (type == "triangle") {
                Triangle* triangle = new Triangle();
                triangle->material = material;
                triangle->v0 = { shapeJson["v0"][0], shapeJson["v0"][1], shapeJson["v0"][2] };
                triangle->v1 = { shapeJson["v1"][0], shapeJson["v1"][1], shapeJson["v1"][2] };
                triangle->v2 = { shapeJson["v2"][0], shapeJson["v2"][1], shapeJson["v2"][2] };
                scene.shapes.push_back(triangle);
            }
        }
    }
    // printf("Success loadFromJSON!\n");
}
Color blendColor(const Color& originalColor, const Color& reflectedColor, float reflectivity) {
    return (1 - reflectivity) * originalColor + reflectivity * reflectedColor;
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

    // Compute the camera's basis vectors
    Vector3 forward = Vector3::normalize(camera.lookAt - camera.position);  // z-axis
    Vector3 right = Vector3::normalize(Vector3::cross(camera.upVector, forward));   // x-axis
    Vector3 up = Vector3::cross(forward, right);                           // y-axis

    // Build the camera's view matrix
    Matrix4x4 viewMatrix = {
        right.x, up.x, forward.x, 0,
        right.y, up.y, forward.y, 0,
        right.z, up.z, forward.z, 0,
        -Vector3::dot(right, camera.position), -Vector3::dot(up, camera.position), -Vector3::dot(forward, camera.position), 1
    };

    // Transform the direction vector by the view matrix
    Vector3 transformedDirection = viewMatrix * direction;

    return Ray(camera.position, transformedDirection);
}

bool Renderer::intersectBinary(const Ray& ray, Shape* shape) {
    if (shape->getType() == "sphere") {
        Sphere* sphere = static_cast<Sphere*>(shape);
        Vector3 oc = ray.origin - sphere->center;
        float a = Vector3::dot(ray.direction, ray.direction);
        float b = 2.0f * Vector3::dot(oc, ray.direction);
        float c = Vector3::dot(oc, oc) - sphere->radius * sphere->radius;
        float discriminant = b * b - 4 * a * c;
        return (discriminant > 0);
    }
    else if (shape->getType() == "cylinder") {
        Cylinder* cylinder = static_cast<Cylinder*>(shape);
        Vector3 oc = ray.origin - cylinder->center;
        float a = Vector3::dot(ray.direction, ray.direction) - pow(Vector3::dot(ray.direction, cylinder->axis), 2);
        float b = 2.0f * (Vector3::dot(oc, ray.direction) - Vector3::dot(ray.direction, cylinder->axis) * Vector3::dot(oc, cylinder->axis));
        float c = Vector3::dot(oc, oc) - pow(Vector3::dot(oc, cylinder->axis), 2) - cylinder->radius * cylinder->radius;
        float discriminant = b * b - 4 * a * c;

        // Check intersection with the sides of the cylinder
        if (discriminant >= 0) {
            float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
            float t2 = (-b + sqrt(discriminant)) / (2.0f * a);
            Vector3 point1 = ray.origin + ray.direction * t1;
            Vector3 point2 = ray.origin + ray.direction * t2;
            float heightStart = Vector3::projectAlongAxis(cylinder->getBottomCenter(), cylinder->axis);
            float heightEnd = Vector3::projectAlongAxis(cylinder->getTopCenter(), cylinder->axis);

            float point1Projection = Vector3::projectAlongAxis(point1, cylinder->axis);
            float point2Projection = Vector3::projectAlongAxis(point2, cylinder->axis);

            if ((point1Projection >= heightStart && point1Projection <= heightEnd)
                || (point2Projection >= heightStart && point2Projection <= heightEnd)) {
                return true;
            }
        }

        // Check intersection with the top cap of the cylinder
        Vector3 topCenter = cylinder->getTopCenter();
        float tTop = Vector3::dot(topCenter - ray.origin, cylinder->axis) / Vector3::dot(ray.direction, cylinder->axis);
        if (tTop >= 0) {
            Vector3 pointOnTopCap = ray.origin + ray.direction * tTop;
            if (Vector3::lengthSquared(pointOnTopCap - topCenter) <= cylinder->radius * cylinder->radius) {
                return true;
            }
        }

        // Check intersection with the bottom cap of the cylinder
        Vector3 bottomCenter = cylinder->getBottomCenter();
        float tBottom = Vector3::dot(bottomCenter - ray.origin, cylinder->axis) / Vector3::dot(ray.direction, cylinder->axis);
        if (tBottom >= 0) {
            Vector3 pointOnBottomCap = ray.origin + ray.direction * tBottom;
            if (Vector3::lengthSquared(pointOnBottomCap - bottomCenter) <= cylinder->radius * cylinder->radius) {
                return true;
            }
        }
    }
    else if (shape->getType() == "triangle") {
        Triangle* triangle = static_cast<Triangle*>(shape);
        Vector3 edge1 = triangle->v1 - triangle->v0;
        Vector3 edge2 = triangle->v2 - triangle->v0;
        Vector3 pvec = Vector3::cross(ray.direction, edge2);
        float det = Vector3::dot(edge1, pvec);
        if (fabs(det) < 1e-8) {
            return false;  // Ray is parallel to the triangle
        }
        float invDet = 1.0f / det;
        Vector3 tvec = ray.origin - triangle->v0;
        float u = Vector3::dot(tvec, pvec) * invDet;
        if (u < 0.0f || u > 1.0f) {
            return false;
        }
        Vector3 qvec = Vector3::cross(tvec, edge1);
        float v = Vector3::dot(ray.direction, qvec) * invDet;
        if (v < 0.0f || u + v > 1.0f) {
            return false;
        }
        return true;  // Ray intersects the triangle
    }
    // TODO: Implement intersection tests for other shape types (cylinder, triangle, etc.)
    return false;
}

bool Renderer::intersect(const Ray& ray, Shape* shape, float& distance) {
    if (shape->getType() == "sphere") {
        Sphere* sphere = static_cast<Sphere*>(shape);
        Vector3 oc = ray.origin - sphere->center;
        float a = Vector3::dot(ray.direction, ray.direction);
        float b = 2.0f * Vector3::dot(oc, ray.direction);
        float c = Vector3::dot(oc, oc) - sphere->radius * sphere->radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant > 0) {
            float sqrtDiscriminant = sqrt(discriminant);
            float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
            float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

            // Ensure t1 is the smaller (closer) t value
            if (t1 > t2) {
                std::swap(t1, t2);
            }

            // If t1 is positive, we use it
            if (t1 > 0) {
                distance = t1;
                return true;
            }
            
            // If t1 is negative but t2 is positive, we use t2
            if (t2 > 0) {
                distance = t2;
                return true;
            }
        }
    }
    else if (shape->getType() == "cylinder") {
        Cylinder* cylinder = static_cast<Cylinder*>(shape);
        Vector3 oc = ray.origin - cylinder->center;
        float a = Vector3::dot(ray.direction, ray.direction) - pow(Vector3::dot(ray.direction, cylinder->axis), 2);
        float b = 2.0f * (Vector3::dot(oc, ray.direction) - Vector3::dot(ray.direction, cylinder->axis) * Vector3::dot(oc, cylinder->axis));
        float c = Vector3::dot(oc, oc) - pow(Vector3::dot(oc, cylinder->axis), 2) - cylinder->radius * cylinder->radius;
        float discriminant = b * b - 4 * a * c;
                // Check intersection with the sides of the cylinder
        float minDistance = std::numeric_limits<float>::infinity();  // Initialize with max value
        bool hasIntersection = false;
        if (discriminant >= 0) {
   
            float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
            float t2 = (-b + sqrt(discriminant)) / (2.0f * a);
            // Ensure t1 is the smaller (closer) t value
            if (t1 > t2) {
                std::swap(t1, t2);
            }

            Vector3 point1 = ray.origin + ray.direction * t1;
            Vector3 point2 = ray.origin + ray.direction * t2;
            float heightStart = Vector3::projectAlongAxis(cylinder->getBottomCenter(), cylinder->axis);
            float heightEnd = Vector3::projectAlongAxis(cylinder->getTopCenter(), cylinder->axis);

            float point1Projection = Vector3::projectAlongAxis(point1, cylinder->axis);
            float point2Projection = Vector3::projectAlongAxis(point2, cylinder->axis);
            // Check if t1 is within the cylinder height
            if (t1 > 0 && point1Projection >= heightStart && point1Projection <= heightEnd) {
                minDistance = std::min(minDistance, t1);
                hasIntersection = true;
            }
            // If t1 is negative, check t2
            else if (t2 > 0 && point2Projection >= heightStart && point2Projection <= heightEnd) {
                minDistance = std::min(minDistance, t2);
                hasIntersection = true;
            }
        }

        // Check intersection with the top cap of the cylinder
        Vector3 topCenter = cylinder->getTopCenter();
        float tTop = Vector3::dot(topCenter - ray.origin, cylinder->axis) / Vector3::dot(ray.direction, cylinder->axis);
        if (tTop >= 0) {
            Vector3 pointOnTopCap = ray.origin + ray.direction * tTop;
            if (tTop > 0 && Vector3::lengthSquared(pointOnTopCap - topCenter) <= cylinder->radius * cylinder->radius) {
                minDistance = std::min(minDistance, tTop);
                hasIntersection = true;
            }
        }

        // Check intersection with the bottom cap of the cylinder
        Vector3 bottomCenter = cylinder->getBottomCenter();
        float tBottom = Vector3::dot(bottomCenter - ray.origin, cylinder->axis) / Vector3::dot(ray.direction, cylinder->axis);
        if (tBottom >= 0) {
            Vector3 pointOnBottomCap = ray.origin + ray.direction * tBottom;
            if (tBottom > 0 && Vector3::lengthSquared(pointOnBottomCap - bottomCenter) <= cylinder->radius * cylinder->radius) {
                minDistance = std::min(minDistance, tBottom);
                hasIntersection = true;
            }
        }
        // After all checks, if an intersection was found, update the distance
        if (hasIntersection) {
            distance = minDistance;
            return true;
        }
        // If no intersection is found, return false
        return false;
    }
    else if (shape->getType() == "triangle") {
        Triangle* triangle = static_cast<Triangle*>(shape);
        Vector3 edge1 = triangle->v1 - triangle->v0;
        Vector3 edge2 = triangle->v2 - triangle->v0;
        Vector3 pvec = Vector3::cross(ray.direction, edge2);
        float det = Vector3::dot(edge1, pvec);
        if (fabs(det) < 1e-8) {
            return false;  // Ray is parallel to the triangle
        }
        float invDet = 1.0f / det;
        Vector3 tvec = ray.origin - triangle->v0;
        float u = Vector3::dot(tvec, pvec) * invDet;
        if (u < 0.0f || u > 1.0f) {
            return false;
        }
        Vector3 qvec = Vector3::cross(tvec, edge1);
        float v = Vector3::dot(ray.direction, qvec) * invDet;
        if (v < 0.0f || u + v > 1.0f) {
            return false;
        }
        // Compute the distance to the intersection point
        float minDistance = Vector3::dot(edge2, qvec) * invDet;
        if (minDistance > 1e-8) { // Check for a positive distance to avoid intersections behind the ray origin
            distance = minDistance;
            return true;  // Ray intersects the triangle
        }
    }
    return false;

    // If no intersection is found, return false
    return false;
}
Color calculateLocalIllumination(const Vector3& intersectionPoint, 
                                 const Vector3& normal, 
                                 const Material& material, 
                                 const Vector3& viewDirection, 
                                 const std::vector<LightSource*>& lights) {
    Color globalAmbientLight{1, 1, 1}; // Assuming white color for global ambient light
    Color ambient = globalAmbientLight * material.ambientColor; // Global ambient light multiplied by the material's ambient color
    Color diffuse(0.0f, 0.0f, 0.0f);
    Color specular(0.0f, 0.0f, 0.0f);

    for (const auto& lightPtr : lights) {
        Vector3 lightDir = (lightPtr->position - intersectionPoint).normalize(); // Direction from point to light
        Vector3 halfVector = (viewDirection + lightDir).normalize(); // Halfway vector between view direction and light direction

        // Diffuse reflection
        float diff = std::max(Vector3::dot(normal, lightDir), 0.0f);
        diffuse += lightPtr->intensity * (material.diffuseColor * diff);

        // // Specular reflection
        // float spec = std::pow(std::max(Vector3::dot(normal, halfVector), 0.0f), material.specularExponent);
        // specular += lightPtr->intensity * (material.specularColor * spec);
        // Specular reflection
        float shininess = material.specularExponent;
        float specCoefficient = std::pow(std::max(Vector3::dot(normal, halfVector), 0.0f), shininess);

        // Reduce the intensity of the specular reflection if it's too strong
        float specularIntensityReduction = 0.5f; // Value less than 1 to reduce intensity, experiment with this value
        specCoefficient *= specularIntensityReduction;

        specular += lightPtr->intensity * (material.specularColor * specCoefficient);
    }
        
    // Combine the components
    Color pixelColor = ambient + diffuse + specular;

    // Ensure that the color values are within the valid range [0, 1] or [0, 255] depending on your color implementation
    pixelColor.clamp();

    return pixelColor;
}

bool Renderer::isInShadow(const Vector3& point, const std::vector<Shape*>& shapes, const std::vector<LightSource*>& lights) {
    for (const auto* light : lights) {
        Vector3 toLight = light->position - point;
        float distanceToLight = Vector3::length(toLight);
        Vector3 directionToLight = Vector3::normalize(toLight);

        // Bias to avoid shadow acne
        const float bias = 1e-4f; 
        Vector3 startPoint = point + directionToLight * bias;

        Ray shadowRay(startPoint, directionToLight);
        
        for (auto* shape : shapes) {
            float distance = std::numeric_limits<float>::max();
            if (intersect(shadowRay, shape, distance)) {
                if (distance < distanceToLight) {
                    return true; // The point is in shadow with respect to this light
                }
            }
        }
    }

    return false; // The point is not in shadow with respect to any light
}


Color Renderer::adjustForShadows(const Color& originalColor) {
    float shadowIntensity = 0.6f; // You can adjust this value to make the shadow lighter or darker
    return originalColor * shadowIntensity; // Simply darken the color
}

Color Renderer::calculateReflection(const Ray& incidentRay, const Vector3& intersectionPoint, const Vector3& normal, const Material& material) {
    Vector3 reflectedDirection = incidentRay.direction - normal * 2 * Vector3::dot(incidentRay.direction, normal);
    float bias = 1e-4; // A small bias to avoid self-intersection
    Ray reflectedRay(intersectionPoint + bias * normal, reflectedDirection);

    for (auto* shape : scene.shapes) {
        float distance;
        if (intersect(reflectedRay, shape, distance)) {
            Vector3 hitPoint = intersectionPoint + distance * reflectedRay.direction;
            Vector3 hitNormal = shape->getNormal(hitPoint);

            if (shape->material.isRefractive) {
                // Handle refraction for transparent objects
                Color refractedColor = calculateRefraction(reflectedRay, hitPoint, hitNormal, shape->material);
                return blendColor(refractedColor, calculateLocalIllumination(hitPoint, hitNormal, shape->material, -reflectedDirection, scene.lights), shape->material.reflectivity);
            } else if (isInShadow(hitPoint, scene.shapes, scene.lights)) {
                // If in shadow, use a darker color or the shadow color
                return adjustForShadows(calculateLocalIllumination(hitPoint, hitNormal, shape->material, -reflectedDirection, scene.lights));
            } else {
                // If not in shadow, calculate the full illumination
                return calculateLocalIllumination(hitPoint, hitNormal, shape->material, -reflectedDirection, scene.lights);
            }
        }
    }

    // If no intersection, return the background color
    return scene.backgroundColor;
}





// Helper function to clamp a value
float Renderer::clamp(float min, float max, float value) {
    return std::max(min, std::min(max, value));
}

Color Renderer::traceRefractedRay(const Ray& refractedRay) {
    // Find the closest shape that the refracted ray intersects
    Shape* closestShape = nullptr;
    float closestDistance = std::numeric_limits<float>::max();
    Vector3 closestIntersectionPoint;
    Vector3 closestNormal;

    for (Shape* shape : scene.shapes) {
        float distance;
        if (intersect(refractedRay, shape, distance)) {
            if (distance < closestDistance) {
                closestDistance = distance;
                closestShape = shape;
                closestIntersectionPoint = refractedRay.origin + refractedRay.direction * distance;
                closestNormal = shape->getNormal(closestIntersectionPoint);
            }
        }
    }

    if (closestShape) {
        // Compute the color at the intersection point
        return calculateLocalIllumination(closestIntersectionPoint, closestNormal, closestShape->material, -refractedRay.direction, scene.lights);
    } else {
        // If the ray does not intersect anything, return the background color
        return scene.backgroundColor;
    }
}

Vector3 Renderer::refract(const Vector3& incident, const Vector3& normal, float eta) {
    float cosi = clamp(-1.0f, 1.0f, Vector3::dot(incident, normal));
    float etai = 1, etat = eta;
    Vector3 n = normal;
    if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -normal; }
    float etaRatio = etai / etat;
    float k = 1 - etaRatio * etaRatio * (1 - cosi * cosi);
    return k < 0 ? Vector3(0,0,0) : etaRatio * incident + (etaRatio * cosi - sqrtf(k)) * n;
}

Color Renderer::calculateRefraction(const Ray& ray, const Vector3& intersectionPoint, const Vector3& normal, const Material& material) {
    float refractionRatio = material.isRefractive ? (1.0f / material.refractiveIndex) : 1.0f;
    Vector3 refractedDirection = refract(ray.direction, normal, refractionRatio);
    
    // Create the refracted ray
    Ray refractedRay(intersectionPoint, refractedDirection);

    // Trace the refracted ray through the scene
    return traceRefractedRay(refractedRay);
}



Color toneMappingLinear(const Color& hdrColor, float exposure=1.0) {
    // Apply exposure to the HDR color
    Color mapped = hdrColor * exposure;

    // Clamp the color values to the [0.0, 1.0] range
    mapped.clamp();

    return mapped;
}



void Renderer::writeColorImageToPPM(const std::vector<std::vector<Color>>& image, const std::string& filename) {
    std::ofstream file(filename);
    file << "P6\n" << camera.width << " " << camera.height << "\n255\n";

    for (const auto& row : image) {
        for (const Color& pixel : row) {
            unsigned char r, g, b;
            pixel.getAsIntegers(r, g, b); // Convert float color values to unsigned char
            file.write(reinterpret_cast<char*>(&r), 1); // Write red component
            file.write(reinterpret_cast<char*>(&g), 1); // Write green component
            file.write(reinterpret_cast<char*>(&b), 1); // Write blue component
        }
    }
}


std::vector<std::vector<Color>> Renderer::render(){
    printf("Start render....\n");
    if (renderMode == "phong"){
        return renderPhong();
    }
    else if (renderMode == "binary"){
        return renderBinary();
    }
    else{
        std::cerr << "Error: Unknown render mode " << renderMode << std::endl;
        return std::vector<std::vector<Color>>();
    }
    return std::vector<std::vector<Color>>();
}

std::vector<std::vector<Color>> Renderer::renderBinary() {
    std::vector<std::vector<Color>> image(camera.height, std::vector<Color>(camera.width));

    for (int y = 0; y < camera.height; ++y) {
        for (int x = 0; x < camera.width; ++x) {
            Ray ray = computeRay(x, y);
            bool hit = false;
            for (Shape* shape : scene.shapes) {
                if (intersectBinary(ray, shape)) {
                    // Red color for intersection (assuming float range 0.0 to 1.0)
                    image[y][x] = {1.0f, 0.0f, 0.0f};  
                    hit = true;
                    break;
                }
            }
            if (!hit) {  // No intersection, use background color
                image[y][x] = scene.backgroundColor; // Directly use background color
            }
        }
    }
    return image;
}


std::vector<std::vector<Color>> Renderer::renderPhong() {
    std::vector<std::vector<Color>> image(camera.height, std::vector<Color>(camera.width));
    
    // Iterate over each pixel
    for (int y = 0; y < camera.height; ++y) {
        for (int x = 0; x < camera.width; ++x) {
            Ray ray = computeRay(x, y); // Compute the ray for the current pixel
            Color pixelColor = scene.backgroundColor; // Start with the background color

            // Intersection test
            float minDistance = std::numeric_limits<float>::max();
            Shape* closestShape = nullptr;
            for (Shape* shape : scene.shapes) {
                float distance = std::numeric_limits<float>::max(); // Initialize distance to max value
                if (intersect(ray, shape, distance) && distance < minDistance) {
                    minDistance = distance;
                    closestShape = shape;
                }
            }
            // If a shape is hit by the ray
            if (closestShape != nullptr) {
                // Calculate intersection point and normal
                Vector3 intersectionPoint = ray.origin + ray.direction * minDistance;
                Vector3 normal = closestShape->getNormal(intersectionPoint);

                // Calculate local illumination (Blinn-Phong)
                pixelColor = calculateLocalIllumination(intersectionPoint, normal, closestShape->material, ray.direction, scene.lights);
                // Shadows - check if the intersection point is in shadow
                // (Optional: Could be optimized with shadow rays)
                if (isInShadow(intersectionPoint, scene.shapes, scene.lights)) {
                    pixelColor = adjustForShadows(pixelColor);
                }

                // // Reflection
                // if (closestShape->material.isReflective) {
                //     Color reflectedColor = calculateReflection(ray, intersectionPoint, normal, closestShape->material);
                //     pixelColor = blendColor(pixelColor, reflectedColor, closestShape->material.reflectivity);
                // }

                // // Refraction
                // if (closestShape->material.isRefractive) {
                //     Color refractedColor = calculateRefraction(ray, intersectionPoint, normal, closestShape->material);
                //     pixelColor = blendColor(pixelColor, refractedColor, 1);
                // }

                // // Textures
                // if (closestShape->hasTexture()) {
                //     Color textureColor = getTextureColor(intersectionPoint, closestShape);
                //     pixelColor = blendTextureColor(pixelColor, textureColor);
                // }

                // Tone mapping - linear
                pixelColor = toneMappingLinear(pixelColor);
            }
            
            // Set the color of the pixel in the image
            image[y][x] = pixelColor;

            // Bounding volume hierarchy (BVH) and other acceleration structures can be integrated into the intersection tests
            // to speed up the rendering process. This is a more advanced topic and would significantly alter the structure of your code.
        }
    }

    return image;
}

// Note: The above functions such as calculateLocalIllumination, isInShadow, calculateReflection, etc., are placeholders
// for the respective algorithms you would need to implement.

// Additional functions needed for the Phong rendering would include the following:
// - calculateLocalIllumination: Computes Blinn-Phong shading
// - isInShadow: Determines if a point is in shadow or not
// - adjustForShadows: Adjusts the color of a pixel based on shadowing
// - calculateReflection: Calculates the reflected color from a surface
// - calculateRefraction: Calculates the refracted color through a transparent object
// - blendColor: Blends two colors based on a coefficient
// - getTextureColor: Retrieves the color from a texture at a given point on a surface
// - blendTextureColor: Blends the texture color with the object's base color
// - toneMapping: Applies tone mapping to the final color