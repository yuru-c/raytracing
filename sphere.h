#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable {
public:
    // 建構子:靜態球體
    // 物理巧思：利用私有變數 center(static_center, vec3(0,0,0)) 構造一條方向為零向量的「死線」
    // 當任意時間 t 傳入時，center.at(t) 永遠都會回傳初始點 static_center，完美相容舊版
    sphere(const point3& static_center, double radius, shared_ptr<material> mat)
        : center(static_center, vec3(0,0,0)), radius(std::fmax(0, radius)), mat(mat)  // 成員 塞進類別私有變數 回傳0.0和radius之間比較大的那一個數字
    {
        // 建立一個三維皆為半徑長度的半徑向量
        auto rvec = vec3(radius, radius, radius);
        // 球心減去半徑為最小頂點 球心加上半徑為最大頂點
        bbox = aabb(static_center - rvec, static_center + rvec);
    }

    // 建構子:動態移動球體moving sphere
    // 物理巧思：射線起點為 time=0 的 center1，而射線方向向量設為位移量（center2 - center1）
    // 如此一來，當時間 t 傳入時，center.at(t) 就等於 center1 + t * (center2 - center1)，剛好是標準的線性插值
    sphere(const point3& center1, const point3& center2, double radius, shared_ptr<material> mat)
        : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat) {
            auto rvec = vec3(radius, radius, radius);
            // 算出時間點0時球體所在的包圍盒box1
            aabb box1(center.at(0) - rvec, center.at(0) + rvec);
            // 算出時間點1時球體經線性位移後所在的包圍盒box2
            aabb box2(center.at(1) - rvec, center.at(1) + rvec);
            // 呼叫AABB的融合建構子 產生一個能包住box1與box2的集大包圍盒
            bbox = aabb(box1, box2);
        }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // 透過軌跡射線 center.at() 一鍵算出該球體在「該瞬間」處於 3D 空間的哪個確切位置
        point3 current_center = center.at(r.time());
        vec3 oc = current_center - r.origin();
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
        vec3 outward_normal = (rec.p - current_center) / radius;
    
        // 讓紀錄本自動判定光線式從內側還式外側射入 並調整法向量
        rec.set_face_normal(r, outward_normal);
        // 取得二維紋理座標
        // get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat; // 核心綁定 把球體自己的材質主動交給撞擊紀錄
    
        return true;
    }

    
    aabb bounding_box() const override { return bbox; }

private:
    ray center;
    double radius;
    shared_ptr<material> mat; // 球體內部保存的材質擁有權
    aabb bbox; // 球體的包圍盒
};

#endif