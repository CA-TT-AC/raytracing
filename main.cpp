#include <iostream>
#include "head.h"
int main() {
    Renderer renderer;
    // renderer.loadFromJSON("./data/binary_primitves.json");
    renderer.loadFromJSON("./data/mirror_image.json");
    renderer.writeColorImageToPPM(renderer.render(), "./data/binary_primitives.ppm");
    return 0;
}
