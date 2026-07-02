#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <random> // C++標準隨機數庫

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

// 回傳[0,1)之間的隨機雙精度浮點數
inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator; // 梅森選轉演算法引擎
    return distribution(generator);
}

// 回傳[min, max)之間的隨機雙精度浮點數
inline double random_double(double min, double max){
    return min + (max - min) * random_double();
}

// 新增區間隨機整數生成器
inline int random_int(int min, int max) {
    return int(random_double(min, max+1));
}

// 全局基礎標頭檔打包引入
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif