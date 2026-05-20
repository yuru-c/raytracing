#include "color.h"
#include "vec3.h"

#include <iostream>

int main() {
    // 影像設定
    int image_width = 256;
    int image_height = 256;

    // 渲染與標頭輸出
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            // 使用物件導向，把 RGB 包進 color 向量物件中
            auto pixel_color = color(double(i)/(image_width-1), double(j)/(image_height-1), 0);
            
            // 透過定義好的函式輸出
            write_color(std::cout, pixel_color);
        }
    }

    std::clog << "\rDone.                 \n";
}