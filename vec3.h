#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>
#include "rtweekend.h"

class vec3 {
    public:
        double e[3];
        //預設建構子
        vec3() : e{0,0,0} {}
        //有參數的建構子
        vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}
        //便捷函式
        double x() const { return e[0]; }
        double y() const { return e[1]; }
        double z() const { return e[2]; }
        //反向向量
        vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
        //中括號(唯獨):在const環境讀取v[i]時複製一分數值回傳
        double operator[](int i) const {return e[i]; }
        //中括號(可寫):回傳double&(引用) 讓別人可以直接用b[i] = 5.0 改到最底層的記憶體
        double& operator[](int i) {return e[i]; }
        //複合加法(v1 += v2):把我自己的XYZ加上對方的XYZ 最後回傳我自己(*this)的地址
        vec3& operator+=(const vec3& v) {
            e[0] += v.e[0];
            e[1] += v.e[1];
            e[2] += v.e[2];
            return *this;
        }
        //複合乘法(v1 *= t):把我自己的XYZ都放大t倍 回傳自己
        vec3& operator*=(double t) {
            e[0] *=t;
            e[1] *=t;
            e[2] *=t;
            return *this;
        }
        //複合除法(v1 /= t):先把1/t算出來(變乘數) 再回傳呼叫 *=
        vec3& operator/=(double t) {
            return *this *= 1/t;
        }

        double length() const {
            return std::sqrt(length_squared());
        }
        // 雙精度長度平方計算
        double length_squared() const {
            return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
        }
        // 數值安全防護鎖 判定向量是否退化
        bool near_zero() const {
            auto s = 1e-8; // 設定一個極小的閥值10^-8
            return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
        }
        // 產生一個XYZ接落在[0,1)之間的隨機向量
        static vec3 random() {
            return vec3(random_double(), random_double(), random_double());
        }
        // 產生一個XYZ接落在指定[min,max)之間的隨機向量
        static vec3 random(double min, double max) {
            return vec3(random_double(min,max), random_double(min,max), random_double(min,max));
        }

        private:
        vec3* n = this;
};


using vec3 = class vec3; 
using point3 = vec3;
using color = vec3;

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3& v) {
    return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline vec3 operator*(const vec3& v, double t) {
    return t * v;
}

inline vec3 operator/(const vec3& v, double t) {
    return (1/t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
    return u.e[0] * v.e[0]
         + u.e[1] * v.e[1]
         + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}
// 透過拒絕採樣法產生精準的單位球面隨機向量
inline vec3 random_unit_vector() {
    while (true) {
        auto p = vec3::random(-1,1);    // 1.在正立方體內隨機抓一個3D點
        auto lensq = p.length_squared();    // 2.計算他到中心點的距離平方
        // 3.數值安全鎖:防止極度靠近中心的點(10^-160)引發除以0的黑洞危機
        if (1e-160 < lensq && lensq <=1)
            return p / sqrt(lensq); // 4.符合條件 將向量長度除以自身長度(單位化)並回傳
    }
}
// 確保產生的隨機向量一定在表面法向量的同一側半球
inline vec3 random_on_hemisphere(const vec3& normal) {
    vec3 on_unit_sphere = random_unit_vector();
    if (dot(on_unit_sphere, normal) > 0.0)  // 如果與法向量同方向(夾角為銳角)
        return on_unit_sphere;
    else    // 如果反方向 直接加上負號翻轉回來
        return -on_unit_sphere;
}
// 鏡面反射向量計算函式
inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2*dot(v,n)*n;
}
// 幾何工具:計算光線穿透介質時的折射方向向量
inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
    // 1.內積計算射光與法向量的夾角餘弦值 cos
    // std::fmin() 數值安全鎖
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    // 2.折射光線 橫向平貼表面的垂直分量向量
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta*n);
    // 3.折射光線 縱向沉入內部的平行分量向量
    // std::fabs 取得絕對值
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    // 4.橫向與縱向分量相加
    return r_out_perp + r_out_parallel;
}


#endif