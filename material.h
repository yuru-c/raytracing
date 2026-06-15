#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

class material {
    public:
        // 虛擬解構子 確保衍生類別釋放記憶體時能正確被呼叫
        virtual ~material() = default;

        // 純虛擬 / 虛擬函式 定義材質的散射行為
        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const {
            return false;
        }
};

// 繼承材質基底類別
class lambertian : public material {
    public:
        // 建構子傳入反照率(色彩向量)
        lambertian(const color& albedo) : albedo(albedo) {}

        // 覆寫(override)散射行為
        bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
        const override {
            auto scatter_direction = rec.normal + random_unit_vector();
            // 防彈安全鎖 捕捉退化的散射方向
            if (scatter_direction.near_zero())
                scatter_direction = rec.normal; // 如歸零 就強制讓他等於法向量
            
            scattered = ray(rec.p, scatter_direction);
            attenuation = albedo; // 衰減率直接等於材質本身的顏色(反照率)
            return true;
        }
    private:
        color albedo;
};

// 金屬材質類別
class metal : public material {
    public:
    // 建構子防禦鎖:利用三元運算子限制fuzz必須小於1.0
    metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        // 1.幾何公式算出鏡面反射方向
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        // 先將反射向量轉為長度1 再疊加上模糊擾動
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        // 2.建立從相交點rec.p出發的反射光線
        scattered = ray(rec.p, reflected);
        // 3.反射率(衰減率)等於金屬自身的顏色
        attenuation = albedo;
        // 只有當散射方向朝向表面外側(與法向量內積大於0)時才算反射成功
        return (dot(scattered.direction(), rec.normal) > 0);
    }    
    private:
    color albedo;
    double fuzz;
};

#endif