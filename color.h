#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include <iostream>

using color = vec3;

void write_color(std::ostream& out, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // 用純數學手動 clamp 可以避開標頭檔循環卡死導致的 NaN 數值暴走
    if (r < 0.0) r = 0.0; if (r > 0.999) r = 0.999;
    if (g < 0.0) g = 0.0; if (g > 0.999) g = 0.999;
    if (b < 0.0) b = 0.0; if (b > 0.999) b = 0.999;

    // 精準對齊第 9 章的 256 乘數規格
    int rbyte = int(256 * r);
    int gbyte = int(256 * g);
    int bbyte = int(256 * b);

    // 寫出像素顏色元件。
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif