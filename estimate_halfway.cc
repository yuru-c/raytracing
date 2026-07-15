#include "rtweekend.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>

struct sample {
    double x;
    double p_x;
};

// 排序比較函式:按x座標由小到大排序
bool compare_by_x(const sample& a,const sample& b) {
    return a.x < b.x;
}

int main() {
    const unsigned int N = 10000;
    sample samples[N];
    double sum = 0.0;

    for (unsigned int i = 0; i < N; i++) {
        auto x = random_double(0, 2*pi);
        auto sin_x = std::sin(x);
        auto p_x = exp(-x / (2*pi)) * sin_x * sin_x;
        sum += p_x;
        samples[i] = {x, p_x};
    }

    // 依X進行排序 (排序時間複雜度為O(NlogN))
    std::sort(std::begin(samples), std::end(samples), compare_by_x);

    double half_sum = sum / 2.0;
    double halfway_point = 0.0;
    double accum = 0.0;

    // 累加面積直到達到總面積的一半
    for (unsigned int i = 0; i < N; i++) {
        accum += samples[i].p_x;
        if (accum >= half_sum) {
            halfway_point = samples[i].x;
            break;
        }
    }

    std::cout << "Halfway = " << halfway_point << '\n';
}