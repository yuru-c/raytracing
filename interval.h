#ifndef INTERVAL_H
#define INTERVAL_H

#include <limits>

class interval {
public:
    double min, max;

    // 預設建構子:己念一個空區間(最小值是正無限大 最大值是負無限大)
    interval() : min(+std::numeric_limits<double>::infinity()), max(-std::numeric_limits<double>::infinity()) {}

    interval(double min, double max) : min(min), max(max) {}

    // 融合建構子:傳入兩個區間a b自動建立一個能將a與b包夾的最窄區間
    interval(const interval& a, const interval& b) {
        min = a.min <= b.min ? a.min : b.min;
        max = a.max >= b.max ? a.max : b.max;
    }
    // 區間大小(長度)
    double size() const {
        return max - min;
    }

    // 檢查x是否在區間內(包含邊界)
    bool contains(double x) const {
        return min <= x && x <= max;
    }

    // 檢查x是否被區間包夾(不含邊界)
    bool surrounds(double x) const {
        return min < x && x < max;
    }

    // 全局靜態快捷常數:空區間與全宇宙區間
    static const interval empty, universe;

    double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    // 區間擴展(padding)方法:當射線與盒子邊緣完全平行或極度貼合時(d_x=0) 加上delta微服填充避免nan或邊緣穿幫
    interval expand(double delta) const {
        auto padding = delta / 2.0;
        return interval(min - padding, max + padding);
    }
};

// 靜態成員變數初始化
const interval interval::empty = interval(+std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
const interval interval::universe = interval(-std::numeric_limits<double>::infinity(), +std::numeric_limits<double>::infinity());

interval operator+(const interval& ival, double displacement) {
    return interval(ival.min + displacement, ival.max + displacement);
}

interval operator+(double displacement, const interval& ival) {
    return ival + displacement;
}

#endif