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

    void render(const hittable& world) {
        initialize();

        // 建立並寫入 image.ppm
        std::ofstream outFile("image.ppm");
        outFile << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    
        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                auto ray_direction = pixel_center - center;
                ray r(center, ray_direction);
            
                color pixel_color = ray_color(r, world);
                write_color(outFile, pixel_color); // 直接寫入檔案流
            }
        }
        std::clog << "\rDone.                 \n";
        outFile.close();
    }
    
private:
    int    image_height;   // 影像像素高度
    point3 center;         // 相機中心點
    point3 pixel00_loc;    // (0,0) 像素的中心座標
    vec3   pixel_delta_u;  // 往右一個像素的偏移量
    vec3   pixel_delta_v;  // 往下一個像素的偏移量

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

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

    color ray_color(const ray& r, const hittable& world) const {
        hit_record rec;
        // 光線有效測試區間設定從0到無限
        if (world.hit(r, interval(0, infinity), rec)) {
            return 0.5 * (rec.normal + color(1.0, 1.0, 1.0));
        }
    
        // 背景:沒撞到求救維持原本的天空
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

#endif