#ifndef RAY_H
#define RAY_H
#include "vec3.h"

class ray {
    public:
        // 1.預設建構子
        ray() {}
        // 2.有參數的建構子
        ray(const point3& origin, const vec3& direction)
            : orig(origin), dir(direction) {}
        // 3.唯獨
        const point3& origin() const {return orig;}
        const vec3& direction() const {return dir;}
        // 4.P(t) = A + t*b
        point3 at(double t) const {
            return orig + t*dir;
        }

    private:
        point3 orig; // 光線的起點 A
        vec3 dir; // 光線的方向向量 b
};

#endif