#ifndef BVH_H
#define BVH_H

#include "rtweekend.h"
#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"

#include <algorithm> // 引入std::sort必備

class bvh_node : public hittable {
    public:
    // 建構函式1:對外公開介面 直接傳入hittable_list
    bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {}
    // 建構函式2:內部遞迴介面 利用start與end索引控制目前處理的std::vector區段(span)
    bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end) {
        // 建立一個涵蓋當前區段所有物件的包圍盒
        bbox = aabb::empty;
        for (size_t object_index = start; object_index < end; object_index++) {
            bbox = aabb(bbox, objects[object_index]->bounding_box());
        }
        // 找出目前外包圍盒 最長的那一個軸向來進行切分
        int axis = bbox.longest_axis();
        // 靜態指標比較函式
        auto comparator = (axis == 0) ? box_x_compare
            : (axis == 1) ? box_y_compare
            : box_z_compare;

        size_t object_span = end - start;

        if (object_span == 1) {
            // 基礎終止條件1:如果只剩一個物件 左右子樹都指向他 避免出現nullptr造成崩潰
            left = right = objects[start];
        } else if (object_span == 2) {
            // 基礎終止條件2:如果剛好兩個物件 一左一右 完美拆分並結束遞迴
            left = objects[start];
            right = objects[start+1];
        } else {
            // 遞迴步驟:快速排序 根據幾何位置將物件在vector中重新排列
            std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);
            // 抓出中間點 對半對稱拆分
            auto mid = start + object_span / 2;
            // 遞迴建立左子樹
            left = make_shared<bvh_node>(objects, start, mid);
            // 遞迴建立右子樹
            right = make_shared<bvh_node>(objects, mid , end);
        }
    }

    // BVH設限碰撞測試
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // 1.先測試設限有沒有射中 代表裡面幾百顆球全部安全 立刻剪枝回傳flase
        if (!bbox.hit(r, ray_t))
            return false;
        // 2.包圍盒中了 代表裡面的東西可能被射中 先走左子樹
        bool hit_left = left->hit(r, ray_t, rec);
        // 3.走右子樹 如果左子樹有射中 左子樹算出來的最近撞擊距離 rec.t 就會直接變成右子樹的新距離上限 如果右子樹裡有東西比左子樹更遠 右子樹的aabb測試會直接被剔除
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);
        return hit_left || hit_right;
    }
    aabb bounding_box() const override {return bbox; }

    private:
        shared_ptr<hittable> left; // 左子節點 可能是另一個bvh_node 也可能是一顆真正的sphere葉子
        shared_ptr<hittable> right; // 右子節點
        aabb bbox; // 當前節點的融合包圍盒

    // 實作通用軸向包圍盒座標比較函式
    static bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index) {
        auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
        auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 2);
    }
};

#endif