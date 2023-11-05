#include <iostream>
#include "head.h"
int main() {
    Renderer renderer;
    renderer.loadFromJSON("binary_primitves.json");
    renderer.writeBinaryImageToPPM(renderer.renderBinary(), "binary_primitives.ppm");
    return 0;
}
