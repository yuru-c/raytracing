#ifndef QUAD_H
#define QUAD_H

#include "rtweekend.h"
#include "hittable.h"

class quad : public hittable {
    public:
        // 建構子:傳入起點Q 以及兩條沿伸邊的向量uv
        quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat)
            : Q(Q), u(u), v(v), mat(mat)
        {
            // 透過外積求出同時垂直於UV的法向量
            auto n = cross(u, v);
            normal = unit_vector(n);

            // 平面方程式Ax+By+Cz=D 其中D等於法向量與平面上任一點(如Q)的點積
            D = dot(normal, Q);

            // 計算並快取對偶基底的向量W 利用法向量除以N的自我點積(長度平方)
            w = n / dot(n, n);

            set_bounding_box();
        }

        // 計算包含四邊形四個頂底的包圍盒
        virtual void set_bounding_box() {
            // 分別計算兩條對角線所形成的包圍盒 再把這兩個盒子的範圍合併
            auto bbox_diagonal1 = aabb(Q, Q + u + v);
            auto bbox_diagonal2 = aabb(Q + u, Q + v);
            bbox = aabb(bbox_diagonal1, bbox_diagonal2);
        }

        aabb bounding_box() const override { return bbox; }

        // 擊中無限大平面的判定 -> 完整四邊形hit判定
        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            //計算分母:法向量與設限方向的點積
            auto denom = dot(normal, r.direction());

            // 如果[ 母絕對值極小(接近0) 代表射線與平面平行 永不相交
            if (std::fabs(denom) < 1e-8)
                return false;

            // 根據平面公式推導出相交的射先參數t
            auto t = (D - dot(normal, r.origin())) / denom;

            // 如果t不在目前射線允許的有效範圍(ray_t)內 視為未擊中
            if (!ray_t.contains(t))
                return false;

            // 計算射線在該t上的3D空間交點
            auto intersection = r.at(t);
            // 取得交點 從Q指向交底的相對向量p
            vec3 planar_hitpt_vector = intersection - Q;

            // 求出alpha beta
            auto alpha = dot(w, cross(planar_hitpt_vector, v));
            auto beta = dot(w, cross(u, planar_hitpt_vector));

            // 呼叫內部測試函數 如果點在四邊形外就沒擊中
            if (!is_interior(alpha, beta, rec))
                return false;

            // 暫時記錄交點資訊 -> 通過測試填寫交點紀錄
            rec.t = t;
            rec.p = intersection;
            rec.mat = mat;
            rec.set_face_normal(r, normal);

            return true;
        }

        // 獨立出來內部測試函數
        virtual bool is_interior(double a, double b, hit_record& rec) const {
            interval unit_interval = interval(0, 1);

            // 如果alpha beta超出[0,1]範圍 代表點在平行四邊形外
            if (!unit_interval.contains(a) || !unit_interval.contains(b))
                return false;

            // 存入局部二維座標 該交點的UV紋理座標
            rec.u = a;
            rec.v = b;
            return true;
        }

    private:
        point3 Q;
        vec3 u, v;
        vec3 w;
        shared_ptr<material> mat;
        aabb bbox;
        vec3 normal;
        double D;
};

#endif