#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"

#include <memory>
#include <vector>

// 使用std命名空間內特定的智慧指標工具 免去每次都要打std::
using std::make_shared;
using std::shared_ptr;

// hittable_list 幾何物件 繼承hittable
class hittable_list : public hittable {
public:
    // 存無數個指向幾何物件的智慧指標容器
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() {}
    // 建構子:允許在創建清單的同事 塞入第一個物件
    hittable_list(shared_ptr<hittable> object) { add(object); }

    // 清空場景
    void clear() {objects.clear(); }

    // 塞入新物件到場景
    void add(shared_ptr<hittable> object) {
        objects.push_back(object);
    }

    // 核心求交點演算法 遍歷所有物件 找出離相機最近的交點
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        // 核心迴圈:依序詢問場景裡的每個幾何物件
        for (const auto& object : objects) {
            // 把當前的 closest_so_far 傳進去當作物件求交點的tmax
            // 這樣如果物件化 xlosest_so_far 還要遠 該物件就會直接拒絕這次碰撞
            if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t; // 成功撞擊更近的物件
                rec = temp_rec; // 將最近的碰撞事故報告複製出去
            }
        }

        return hit_anything;
    }
};

#endif