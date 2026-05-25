#ifndef INTERVAL_H
#define INTERVAL_H

#include <limits>

class interval {
public:
    double min, max;

    // 預設建構子:己念一個空區間(最小值是正無限大 最大值是負無限大)
    interval() : min(+std::numeric_limits<double>::infinity()), max(-std::numeric_limits<double>::infinity()) {}

    interval(double min, double max) : min(min), max(max) {}

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
};

// 靜態成員變數初始化
inline const interval interval::empty = interval(+std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
inline const interval interval::universe = interval(-std::numeric_limits<double>::infinity(), +std::numeric_limits<double>::infinity());

#endif