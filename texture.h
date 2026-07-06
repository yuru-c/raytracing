#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtweekend.h"
#include "rtw_stb_image.h"
#include "perlin.h"
#include <cmath>

// 紋理抽象基類
class texture {
    public:
        virtual ~texture() = default;

        // 傳入2D紋理座標(u,v)以及該點在3D空間中的座標p 回傳對應顏色
        virtual color value(double u, double v, const point3& p) const = 0;
};

// 純色紋理 包裝原本的color
class solid_color : public texture {
    public:
        solid_color(const color& albedo) : albedo(albedo) {}
        solid_color(double red, double green, double blue) : solid_color(color(red, green, blue)) {}

        // 純色紋理不管(u,v)或位置p是甚麼 一律回傳固定的顏色
        color value(double u, double v, const point3& p) const override {
            return albedo;
        }
    private:
        color albedo;
};

// 空間固體棋盤格紋理
class checker_texture : public texture {
    public:
        // 函式1:傳入另外兩個紋理指標(可以嵌套其他程序化紋理)
        checker_texture(double scale, shared_ptr<texture> even, shared_ptr<texture> odd)
            : inv_scale(1.0 / scale), even(even), odd(odd) {}

        // 函式2:便利介面 直接傳入兩種常數顏色
        checker_texture(double scale, const color& c1, const color& c2)
            : checker_texture(scale, make_shared<solid_color>(c1), make_shared<solid_color>(c2)) {}

        color value(double u, double v, const point3& p) const override {
            // 使用std::floor向下取整 確保正負數空間的格子交錯邏輯一致
            auto xInterger = int(std::floor(inv_scale * p.x()));
            auto yInterger = int(std::floor(inv_scale * p.y()));
            auto zInterger = int(std::floor(inv_scale * p.z()));

            // 三個座標整數相加 若為偶數著色even 奇數著色odd
            bool isEven = (xInterger + yInterger + zInterger) % 2 == 0;
            return isEven ? even->value(u, v, p) : odd->value(u, v, p);
        }

    private:
        double inv_scale;
        shared_ptr<texture> even;
        shared_ptr<texture> odd;
};

// 外部點陣圖片紋理
class image_texture : public texture {
    public:
        image_texture(const char* filename) : image(filename) {}
        
        color value(double u, double v, const point3& p) const override {
            // 如果載入失敗或寬高為0 回傳青色cyan 作為畫面除錯輔助
            if (image.height() <= 0) return color(0, 1, 1);

            // 將傳入的常規UV座標限縮在[0,1]之間
            u = interval(0, 1).clamp(u);
            v = 1.0 - interval(0, 1).clamp(v); // 反轉V軸以符合影像頂部為0的螢幕坐標系

            // 將[0,1]的比例換算成真實圖片的像素行列索引(i,j)
            auto i = int(u * image.width());
            auto j = int(v * image.height());

            // 抓出該像素的RGB 8bit 數值(0-255)
            auto pixel = image.pixel_data(i, j);

            auto color_scale = 1.0 / 255.0;
            // 將0-255的數值重新歸一化回光學計算用[0,1]浮點數顏色
            return color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
    }

    private:
        rtw_image image; // 內部持有封裝好的圖片物件
};

// 初版雜訊紋理
class noise_texture : public texture {
    public:
        // 預設建構子 預設1.0倍
        noise_texture() : scale(1.0) {}

        // 自訂建構子 允許傳入縮放系數控制密集度
        noise_texture(double scale) : scale(scale) {}

            color value(double u, double v, const point3& p) const override {
                // 將雜訊回傳的0-1純標量乘上白色(1,1,1) 得到不同深淺的灰色 X
                // 大理石公式
                // 1.scale*p.z():在Z軸方向建立基礎正弦條紋
                // 2.10.0*noise.turb(p,7):用7層深度的紊流強烈扭曲正弦波的相位
                // 3.(1.0+sin(...))*0.5:將sin的[-1,1]範圍放到安全的[0,1]
                return color(0.5, 0.5, 0.5) * (1.0 + std::sin(scale * p.z() + 10.0 * noise.turb(p, 7)));       
        }

    private:
        perlin noise;
        double scale;
};

#endif