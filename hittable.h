#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "interval.h"

// 碰撞紀錄本:當光線射中物體 用來打包帶回所有幾何資訊
class hit_record {
public:
    point3 p; // 交點
    vec3 normal; // 法向量
    double t; // 交點參數t    
    bool front_face; // true代表光線從外側撞擊 false從內側

    // 依光線入射方向自動校正法向量 永遠與光線迎面對決
    void set_face_normal(const ray& r, const vec3& outward_normal) {
        // 內積小於0 兩向量逆向(從外側射入)
        front_face = dot(r.direction(), outward_normal) < 0;
        // 正面則保持朝外 背面則反轉朝內
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// 所有3D幾何物件的虛擬基底類別
class hittable {
public:
    virtual ~hittable() = default;
    // 純虛擬函式:子類別必須實作「光線如何與自己求交點」的邏輯
    // 只有當交點的t落在[ray_tmin, ray_tmax]區間才算有較撞擊
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
};

#endif