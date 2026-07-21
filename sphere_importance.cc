#include "rtweekend.h"

#include <iostream>
#include <iomanip>

// 1.定義被積函數:f(d)=dz^2
double f(const vec3& d) {
    auto cosine_squared = d.z()*d.z();
    return cosine_squared;
}

// 2.定義均勻球面上採樣的PDF:1/(4*pi)
double  pdf(const vec3& d) {
    return 1 / (4*pi);
}

int main() {
    int N = 1000000;
    auto sum = 0.0;
    for (int i = 0; i < N; i++) {
        // 3.採樣:使用random_unit_vector() 在單位求面上產生均勻隨機方向d
        vec3 d = random_unit_vector();
        auto f_d = f(d);
        // 4.蒙地卡羅累加:f(d)/pdf(d)
        sum += f_d / pdf(d);
    }

    std::cout << std::fixed << std::setprecision(12);
    // 5.取平均得出估算值
    std::cout << "I = " << sum / N << '\n';
}