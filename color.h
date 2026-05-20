#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include <iostream>

using color = vec3;

void write_color(std::ostream& out, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // 將 [0,1] 的元件值轉換為位元組範圍 [0,255]。
    int rbyte = int(255.999 * r);
    int gbyte = int(255.999 * g);
    int bbyte = int(255.999 * b);

    // 寫出像素顏色元件。
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif