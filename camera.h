#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"
#include "color.h"
#include "hittable.h"

#include <iostream>
#include <fstream> // 確保引入檔案流

class camera {
public:
    double aspect_ratio = 1.0; // 影像寬高比
    int image_width = 100; // 影像像素寬度
    int samples_per_pixel = 10;   // 每個像素的隨機採樣數量 (預設 10)
    int max_depth = 10; // 光線最大彈跳次數 (預設 10)

    void render(const hittable& world) {
        initialize();

        // 建立並寫入 image.ppm
        std::ofstream outFile("image.ppm", std::ios::out | std::ios::binary);
        outFile << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    
        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                // 收集並疊加多條隨機光線的顏色
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                // 將加總的顏色乘以縮放因子 (除以 samples_per_pixel) 再寫入
                write_color(outFile, pixel_samples_scale * pixel_color); 
            }
        }
        std::clog << "\rDone.                 \n";
        outFile.close();
    }
    
private:
    int    image_height;   // 影像像素高度
    double pixel_samples_scale;  // 色彩縮放因子 (1.0 / samples_per_pixel)
    point3 center;         // 相機中心點
    point3 pixel00_loc;    // (0,0) 像素的中心座標
    vec3   pixel_delta_u;  // 往右一個像素的偏移量
    vec3   pixel_delta_v;  // 往下一個像素的偏移量

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        // 計算乘數因子
        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = point3(0, 0, 0);

        auto focal_length = 1.0;
        auto viewport_height = 2.0;
        auto viewport_width = viewport_height * (double(image_width) / image_height);
    
        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0); // Y是負的 螢幕座標向下遞增
    
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;
    
        auto viewport_upper_left = center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    }

    // 建立一條射向像素 (i,j) 周邊隨機採樣點的光線
    ray get_ray(int i, int j) const {
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = center;
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    // 產生一個位於 [-0.5, -0.5] 到 [0.5, 0.5] 單位正方形內的隨機偏移向量
    vec3 sample_square() const {
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        // 安全防護鎖:如果彈跳次數歸零 代表光線精疲力竭 直接返回純黑(不再遞迴)
        if (depth <= 0)
            return color(0, 0, 0);

        hit_record rec;
        // 光線有效測試區間設定從0.001(避免陰影痤瘡shadow acne)到無限
        if (world.hit(r, interval(0.001, infinity), rec)) {
            // 1.在撞擊點的法向量半球面上 隨機挑選一個反彈方向 => 法向量直接加上單位球面隨機向量
            vec3 direction = rec.normal + random_unit_vector();
            // 2.遞迴發射一條新光線 起點為撞擊點 rec.p 方向為隨機彈跳方向
            // 3.物理調變:每次彈跳 色彩能量率減一半(x0.5)
            return 0.5 * ray_color(ray(rec.p, direction), depth - 1, world);
        }
    
        // 背景:沒撞到求救維持原本的天空
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

#endif