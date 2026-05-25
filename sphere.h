#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable {
public:
    // 建構子:保護半徑不為負數
    sphere(const point3& center, double radius)
        : center(center), radius(std::fmax(0.0, radius)) {} // 成員 塞進類別私有變數 回傳0.0和radius之間比較大的那一個數字

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        vec3 oc = center - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = h*h - a*c;
        if (discriminant < 0)
            return false; // 判別式小於0 沒碰到

        auto sqrtd = std::sqrt(discriminant);

        // 找落在[ray_tmin, ray_tmax]最近的根 (先嘗試減號的根)
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            // 如果減號的不合法 嘗試加號的(比較遠的那個)
            root = (h + sqrtd) / a;
            if(!ray_t.surrounds(root))
                return false; // 兩個根都不在合理的區間內
        }

        // 成功 寫入紀錄
        rec.t = root;
        rec.p = r.at(rec.t);

        // 標準朝外的法向量(交點-球心 再除以半徑剛好完成標準化
        vec3 outward_normal = (rec.p - center) / radius;
    
        // 讓紀錄本自動判定光線式從內側還式外側射入 並調整法向量
        rec.set_face_normal(r, outward_normal);
    
        return true;
    }

private:
    point3 center;
    double radius;
};

#endif