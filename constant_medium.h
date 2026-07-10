#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H

#include "hittable.h"
#include "material.h"
#include "texture.h"

// 等密度參與介質類別 (模擬煙霧/霧氣)
class constant_medium : public hittable {
    public:
        constant_medium(shared_ptr<hittable> boundary, double density, shared_ptr<texture> tex)
            : boundary(boundary), neg_inv_density(-1.0 / density),
              phase_function(make_shared<isotropic>(tex))
        {}

        constant_medium(shared_ptr<hittable> boundary, double density, const color& albedo)
        : boundary(boundary), neg_inv_density(-1.0 / density),
            phase_function(make_shared<isotropic>(albedo))
        {}

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            hit_record rec1, rec2;

            // 1.檢測射線是否擊中介質邊界的前後兩端
            if (!boundary->hit(r, interval::universe, rec1))
                return false;

            if(!boundary->hit(r, interval(rec1.t + 0.0001, infinity), rec2))
                return false;

            // 2.修正邊界範圍 使其符合相機射線的ray_t限制
            if (rec1.t < ray_t.min) rec1.t = ray_t.min;
            if (rec2.t > ray_t.max) rec2.t = ray_t.max;

            if (rec1.t >= rec2.t)
                return false;

            if (rec1.t < 0)
                rec1.t = 0;

            // 3.計算射線在介質內部穿過的最大物理距離
            auto ray_length = r.direction().length();
            auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;

            // 4.依據密度與隨機亂數 抽籤決定光線被煙味微粒擊中(射線)的距離
            auto hit_distance = neg_inv_density * std::log(random_double());

            // 如果射線距離大於穿透距離 代表光線直接穿透了霧氣 沒碰到微粒
            if (hit_distance > distance_inside_boundary)
                return false;

            // 5.確實命中霧氣內部的某一點 紀錄資訊
            rec.t = rec1.t + hit_distance / ray_length;
            rec.p = r.at(rec.t);

            rec.normal = vec3(1,0,0); // 隨機任意方向法向量(對霧氣無影響)
            rec.front_face = true; // 任意設定
            rec.mat = phase_function; // 套用各向同性散射材質
            
            return true;
        }

        aabb bounding_box() const override { return boundary->bounding_box(); }

    private:
        shared_ptr<hittable> boundary; // 介質的幾何外殼(如box或sphere)
        double neg_inv_density; // 密度的負倒數 優化計算用
        shared_ptr<material> phase_function; //相位函數(微粒散射材質)
};

#endif