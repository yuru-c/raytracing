#ifndef RAY_H
#define RAY_H
#include "vec3.h"

class ray {
    public:
        // 1.預設建構子
        ray() {}
        // 2.支援時間戳記的建構子 將光線發射的時間點記錄到私有變數tm中
        ray(const point3& origin, const vec3& direction, double time)
            : orig(origin), dir(direction), tm(time) {}

        // 相容舊版專案的建構子 若呼叫時沒傳入時間 預設該光線發生在時間點0
        ray(const point3& origin, const vec3& direction)      
        : ray(origin, direction, 0) {}
        // 3.唯獨
        const point3& origin() const {return orig;}
        const vec3& direction() const {return dir;}
        double time() const { return tm; } // 供外部幾何物件查詢這條光線正在看哪一個時間點的世界
        // 4.P(t) = A + t*b
        point3 at(double t) const {
            return orig + t*dir;
        }

    private:
        point3 orig; // 光線的起點 A
        vec3 dir; // 光線的方向向量 b
        double tm; // 這條光線所在的「時間戳記」
};

#endif