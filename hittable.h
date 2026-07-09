#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "interval.h"
#include "aabb.h"

class material; //前向宣告 (forward declaration) 絕殺循環引用
// 碰撞紀錄本:當光線射中物體 用來打包帶回所有幾何資訊
class hit_record {
public:
    point3 p; // 交點
    vec3 normal; // 法向量
    shared_ptr<material> mat; // 新增 指向材質的智慧型指標
    double t; // 交點參數t
    double u; // U座標[0,1]
    double v; // V座標[0,1]
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

    // 強迫所有子類別實作此方法 以便樹狀節點收集並融合邊界
    virtual aabb bounding_box() const = 0;
};

// 平移實例類別translate
class translate : public hittable {
    public:
        translate(shared_ptr<hittable> object, const vec3& offset)
            : object(object), offset(offset)
        {
            bbox = object->bounding_box() + offset;
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // 將射線往反方向平移
            ray offset_r(r.origin() - offset, r.direction(), r.time());

            if (!object->hit(offset_r, ray_t, rec))
                return false;

            // 將命中點加回位移量
            rec.p += offset;

            return true;
        }

        aabb bounding_box() const override { return bbox; }
    
    private:
        shared_ptr<hittable> object;
        vec3 offset;
        aabb bbox;
};

// 繞Y軸旋轉實例類別 rotate Y
class rotate_y : public hittable {
    public:
        rotate_y(shared_ptr<hittable> object, double angle) : object(object) {
            auto radians = degrees_to_radians(angle);
            sin_theta = std::sin(radians);
            cos_theta = std::cos(radians);
            bbox = object->bounding_box();

            point3 min(infinity, infinity, infinity);
            point3 max(-infinity, -infinity, -infinity);

            // 計算旋轉後新包圍盒AABB的8個頂點極值
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        auto x = i*bbox.x.max + (1-j)*bbox.x.min;
                        auto y = i*bbox.y.max + (1-j)*bbox.y.min;
                        auto z = i*bbox.z.max + (1-k)*bbox.z.min;

                        auto newx = cos_theta*x + sin_theta*z;
                        auto newz = -sin_theta*x + cos_theta*z;

                        vec3 tester(newx, y, newz);

                        for (int c = 0; c < 3; c++) {
                            min[c] = std::fmin(min[c], tester[c]);
                            min[c] = std::fmax(max[c], tester[c]);
                        }
                    }
                }
            }

            bbox = aabb(min, max);
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // 將射線從世界空間逆向旋轉到物體空間(-theta)
            auto origin = point3(
                (cos_theta * r.origin().x()) - (sin_theta * r.origin().z()),
                r.origin().y(),
                (sin_theta * r.origin().x()) + (cos_theta * r.origin().z())
            );
            
            auto direction = vec3(
                (cos_theta * r.direction().x()) - (sin_theta * r.direction().z()),
                r.direction().y(),
                (sin_theta * r.direction().x()) + (cos_theta * r.direction().z())            
            );

            ray rotated_r(origin, direction, r.time());

            // 在物體空間測試命中
            if (!object->hit(rotated_r, ray_t, rec))
                return false;

            // 將命中點P與法向量normal順向旋轉回世界空間(+theta)
            rec.p = point3(
                (cos_theta * rec.p.x()) + (sin_theta * rec.p.z()),
                rec.p.y(),
                (-sin_theta * rec.p.x()) + (cos_theta * rec.p.z())
            );

            rec.normal = vec3(
                (cos_theta * rec.normal.x()) + (sin_theta * rec.normal.z()),
                rec.normal.y(),
                (-sin_theta * rec.normal.x()) + (cos_theta * rec.normal.z())
            );

            return true;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        shared_ptr<hittable> object;
        double sin_theta;
        double cos_theta;
        aabb bbox;
};

#endif