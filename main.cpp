#include <iostream>

int main() {
    int image_width = 256;
    int image_height = 256;
    //PPM格式的標頭 P3代表純文字的彩色圖像格式 255代表色彩的最大值(0-255)
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (int j = 0; j < image_height; j++) {
        // \r 讓游標回到行首，std::flush 讓文字立刻顯示在終端機上
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for ( int i = 0; i < image_width; i++) {
            //正規化
            auto r = double(i) / (image_width-1); //左右 紅光
            auto g = double(j) / (image_height-1); //上下 綠光
            auto b = 0.0; //藍固定0
            int ir = static_cast<int>(255.999 * r);
            int ig = static_cast<int>(255.999 * g);
            int ib = static_cast<int>(255.999 * b);
            std::cout << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }
    std::clog << "\rDone.                 \n";
}