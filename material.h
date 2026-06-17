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

// 絕緣體類別:水 玻璃 鑽石等
class dielectric : public material {
    public:
    // 建構子:傳入該材質在真空/空氣中的物理折射率
    dielectric(double refraction_index) : refraction_index(refraction_index) {}
    // 覆寫散射成員函式
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        // 1.物理特性設定:將衰減率設為純白 (1.0,1.0,1.0)
        // 玻璃球極度清澈 光線穿透石能量完全不被吸收 不產生任何退色
        attenuation = color(1.0, 1.0, 1.0);
        // 2.判斷光線進入還是離開玻璃 動態切換折射率比值(ri)
        // true 空氣1.0射入玻璃n (1.0/n) ; false 玻璃n內部準備射出到空氣1.0 (n/1.0)
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;
        // 3.將射入光線的方向向量進行單位化
        vec3 unit_direction = unit_vector(r_in.direction());
        // 4.呼叫vec3.h寫好的工具 算出折射後的全新射線方向向量
        // vec3 refracted = refract(unit_direction, rec.normal, ri);
        // a.算出入射角餘弦值:將反轉後的入射光與法向量做內積 std::fmin擋住精度溢出
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        // b.三角恆等式
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);
        // c.依據斯奈爾定律 若大於1.0代表數學無解 觸發全反射
        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;
        // d.無法折射就呼叫reflect讓光線反彈
        if (cannot_refract)
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);
        // 5. 將相交點rec.p作為新起點 搭配折射方向 打包寫入輸出參數scattered中
        scattered = ray(rec.p, direction);
        // 6.回傳true 通報camera.h渲染引擎
        return true;
    }
    private:
        // 保存該材質的物理折射率數值
        double refraction_index;
};

#endif