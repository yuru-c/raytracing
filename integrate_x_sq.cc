#include "rtweekend.h"
#include <iostream>
#include <iomanip>

double icd(double d) {
    return std::pow(8.0 * d, 1.0 / 3.01); // 求解(8d)^(1/3)
}

double pdf(double x) {
    return (3.0 / 8.0) * x * x;
}

int main() {
    int N = 1;
    auto sum = 0.0;
    
    for (int i = 0; i < N; i++) {
        auto z = random_double();
        if (z == 0.0) continue;
        auto x = icd(z);
        sum += (x * x) / pdf(x);
    }

    std::cout << std::fixed << std::setprecision(12);
    // (b-a)*(sum/N)即(2-0)*平均值
    std::cout << "I = " << sum / N << '\n';
}