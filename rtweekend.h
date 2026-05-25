#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

// C++ 標準庫常用工具宣告
using std::make_shared;
using std::shared_ptr;

// 常數定義
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// 工具函式:角度轉弧度
inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

// 全局基礎標頭檔打包引入
#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif