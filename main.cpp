#include <iostream>
#include "head.h"
int main() {
    Renderer renderer;
    renderer.loadFromJSON("./data/binary_primitves.json");
    renderer.writeBinaryImageToPPM(renderer.renderBinary(), "./data/binary_primitives.ppm");
    return 0;
}
