#ifndef AABB_H
#define AABB_H

#include "rtweekend.h"

class aabb {
    public:
        interval x, y, z; // 3D包圍盒由XYZ三個維度的區間共同決定
        aabb() {} // 三個區間皆無空 代表這是一個空的包圍盒
        // 在三個區間傳入的建構子中調用墊高函式
        aabb(const interval& x, const interval& y, const interval& z)
            : x(x), y(y), z(z) {
                pad_to_minimums();
            }
        
        // 建構子:傳入任意兩個3D頂點點座標A B 自動拉出一個立方體包圍盒 -> 在兩點傳入的建構子中調用墊高函式
        aabb(const point3& a, const point3& b) {
            x = interval(std::fmin(a[0],b[0]), std::fmax(a[0],b[0]));
            y = interval(std::fmin(a[1],b[1]), std::fmax(a[1],b[1]));
            z = interval(std::fmin(a[2],b[2]), std::fmax(a[2],b[2]));
            pad_to_minimums();
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

        // 最長軸向判定:比較XYZ三個維度的包圍盒尺寸
        int longest_axis() const {
            if (x.size() > y.size())
                return x.size() > z.size() ? 0 : 2;
            else
                return y.size() > z.size() ? 1 : 2;
        }

        static const aabb empty, universe;

    private:
        // 防止包圍盒厚度為零的墊高機制
        void pad_to_minimums() {
            double delta = 0.0001;
            if (x.size() < delta) x = x.expand(delta);
            if (y.size() < delta) y = y.expand(delta);
            if (z.size() < delta) z = z.expand(delta);
        }
};

// 在類別外部為靜態常量賦值
const aabb aabb::empty = aabb(interval::empty, interval::empty, interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

aabb operator+(const aabb& bbox, const vec3& offset) {
    return aabb(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}
aabb operator+(const vec3& offset, const aabb& bbox) {
    return bbox + offset;
}

#endif