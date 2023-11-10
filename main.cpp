#include <iostream>
#include "head.h"
int main() {
    Renderer renderer;
    // renderer.loadFromJSON("./data/binary_primitves.json");
    renderer.loadFromJSON("./data/simple_phong.json");
    renderer.writeColorImageToPPM(renderer.render(), "./data/binary_primitives.ppm");
    return 0;
}
