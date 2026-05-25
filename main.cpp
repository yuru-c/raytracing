#include "rtweekend.h"

#include "color.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"

#include <iostream>

// 著色 結合法向量
color ray_color(const ray& r, const hittable& world) {
    hit_record rec;
    // 光線有效測試區間設定從0到無限
    if (world.hit(r, interval(0, infinity), rec)) {
        // 如果設中物件 將法向量的XYZ 映射到RGB顯色
        return 0.5 * (rec.normal + color(1.0, 1.0, 1.0));
    }
    
    // 背景:沒撞到求救維持原本的天空
    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

int main() {
    auto aspect_ratio = 16.0 / 9.0; //16:9
    int image_width = 400;
    // 計算高度 強制轉為int
    int image_height = int(image_width / aspect_ratio);
    // 高度至少為1像素 防止除以0
    image_height = (image_height < 1) ? 1 : image_height;

    // --- World 場景物件管理器 ---
    hittable_list world;

    // 放入球體 1：中央的彩色立體球
    world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5));

    // 放入球體 2：一顆在下方、半徑 100 的超級無敵巨球，用來模擬地平線巨型地面！
    world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0));
    
    // camera
    auto focal_length = 1.0; // 焦距:相機中心到畫布的距離
    auto viewport_height = 2.0; // 虛擬畫布的3D高度固定為2.0

    // 實際整數像素比例計算3D畫布的真實寬度
    auto viewport_width = viewport_height * (double(image_width) / image_height);
    auto camera_center = point3(0, 0, 0); // 相機擺在原點(0,0,0)

    // 幾何網格計算
    // 1.水平U 垂直V 的3D向量
    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0); // Y是負的 螢幕座標向下遞增
    // 2.像素間的水平與垂直間距(Delta)
    auto pixel_delta_u = viewport_u / image_width;
    auto pixel_delta_v = viewport_v / image_height;
    // 3.畫布左上角的3D座標
    auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
    // 4.定位(0,0)像素的中心點
    auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    // Render 渲染
    std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

    for (int j = 0; j < image_height; j++) {
        // 在終端機標準錯誤流（clog）印出進度條
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        
        for (int i = 0; i < image_width; i++) {
            // 1.根據目前的迴圈步數 (i, j)，算出當前像素在 3D 空間中的精確中心座標
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            
            // 2.光線方向 = 像素中心座標 - 相機中心座標
            auto ray_direction = pixel_center - camera_center;
            
            // 3.建立一條從相機射向該像素的光線
            ray r(camera_center, ray_direction);
            
            // 4.計算這條光線看到的顏色，並寫入標準輸出流
            color pixel_color = ray_color(r, world);
            write_color(std::cout, pixel_color);
        }
    }
    std::clog << "\rDone.                 \n";
}