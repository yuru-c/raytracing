#ifndef AABB_H
#define AABB_H

#include "rtweekend.h"

class aabb {
    public:
        interval x, y, z; // 3D包圍盒由XYZ三個維度的區間共同決定
        aabb() {} // 三個區間皆無空 代表這是一個空的包圍盒
        aabb(const interval& x, const interval& y, const interval& z)
            : x(x), y(y), z(z) {}
        
        // 建構子:傳入任意兩個3D頂點點座標A B 自動拉出一個立方體包圍盒
        aabb(const point3& a, const point3& b) {
            x = (a[0] <= b[0]) ? interval(a[0],b[0]) : interval(b[0], a[0]);
            y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
            z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);
        }

        // 建構子:傳入兩個現成的AABB包圍盒 融合出一個能包住這兩個盒子的集大包圍盒
        aabb(const aabb& box0, const aabb& box1) {
            x = interval(box0.x, box1.x);
            y = interval(box0.y, box1.y);
            z = interval(box0.z, box1.z);
        }

        // 輔助函式:根據索引0,1,2分別快速獲取XYZ軸的區間引用
        const interval& axis_interval(int n) const {
            if (n == 1) return y;
            if (n == 2) return z;
            return x;
        }

        // 射線與AABB的Slab相交測試 在任一軸向上發現區間不重疊立刻剔除
        bool hit(const ray& r, interval ray_t) const {
            const point3& ray_orig = r.origin();
            const vec3& ray_dir = r.direction();

            for (int axis = 0; axis < 3; axis++) {
                const interval& ax = axis_interval(axis);

                // 用乘法代替除法 事先算出方向向量的倒數 
                const double adinv = 1.0 / ray_dir[axis];

                auto t0 = (ax.min - ray_orig[axis]) * adinv;
                auto t1 = (ax.max - ray_orig[axis]) * adinv;

                // 確保t0是進入點的參數 t1是離開點的參數
                if (t0 < t1) {
                    if (t0 < ray_t.min) ray_t.min = t0;
                    if (t1 > ray_t.max) ray_t.max = t1;
                } else {
                    // 如果射線反向朝負方向前進
                    if (t1 > ray_t.min) ray_t.min = t1;
                    if (t0 < ray_t.max) ray_t.max = t0;
                }

                // 剪枝pruning 軸向的進入時間大於等於離開時間 代表3D空間區間斷開 完全沒有重疊 這條射線絕對沒射中盒子
                if (ray_t.max <= ray_t.min)
                    return false;
            }
            return true; // 三個軸向完美交疊 射線切實貫穿包圍盒
        }
};

#endif