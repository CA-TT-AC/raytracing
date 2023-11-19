#include <iostream>
#include <iomanip>
#include "head.h"

int main() {
    

    // 总帧数
    int total_frames = 240;

    for (int frame = 0; frame < total_frames; ++frame) {
        Renderer renderer;
        std::cout<<"processing frame "<<frame<<std::endl;
        // 构建文件名，如 "data/animation_frames/frame_0001.json"
        std::stringstream ss;
        ss << "data\\animation_frames\\frame_" << std::setw(4) << std::setfill('0') << frame << ".json";
        std::string filename = ss.str();

        // 加载JSON文件
        renderer.loadFromJSON(filename);

        // 渲染并保存PPM图像，如 "data/animation_frames/frame_0001.ppm"
        ss.str("");  // 清空 stringstream
        ss << "./data/rendered_frames/frame_" << std::setw(4) << std::setfill('0') << frame << ".ppm";
        std::string output_filename = ss.str();
        renderer.writeColorImageToPPM(renderer.render(), output_filename);
    }

    return 0;
}
