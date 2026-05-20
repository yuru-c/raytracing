#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

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

        double length_squared() const {
            return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
        }
};

using point3 = vec3;

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

#endif