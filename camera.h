#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"
#include "color.h"
#include "hittable.h"
#include "material.h"

#include <iostream>
#include <fstream> // 確保引入檔案流

class camera {
public:
    double aspect_ratio = 1.0; // 影像寬高比
    int image_width = 100; // 影像像素寬度
    int samples_per_pixel = 10;   // 每個像素的隨機採樣數量 (預設 10)
    int max_depth = 10; // 光線最大彈跳次數 (預設 10)
    color background; // 場景背景色彩
    double vfov = 90; // 垂直視野夾角fidld of view以角度為單位

    point3 lookfrom = point3(0,0,0); // 相機在世界座標中的擺放位置
    point3 lookat = point3(0,0,-1); // 相機目前正瞄準注視的3D空間目標點
    vec3 vup = vec3(0,1,0); // 用來定義相機頭頂朝向的引導向量

    // 景深
    double defocus_angle = 0; // 模擬光圈大小:射線穿過每個像素的隨機發散錐角(角度制 0代表不開景深)
    double focus_dist = 10; // 模擬對焦平面距離:從相機中心到完美對焦平面的絕對距離

    // 修改render 引入雙重迴圈對每個子像素格子進行採樣
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
                // 不隨機抽spp次數 改依據網格橫向 縱向均勻採購
                for (int s_j = 0; s_j < sqrt_spp;s_j++) {
                    for (int s_i = 0; s_i < sqrt_spp; s_i++) {
                        // 傳入當前子格子的索引 s_i, s_j
                    ray r = get_ray(i, j, s_i, s_j);
                    pixel_color += ray_color(r, max_depth, world);
                    }
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
    // 用於分層採樣的網格參數
    int sqrt_spp; // 採樣樹的平方根(spp=64 則sqrt_spp=8)
    double recip_sqrt_spp; // 1/sqrt_spp 用於計算子格子大小
    point3 center;         // 相機中心點
    point3 pixel00_loc;    // (0,0) 像素的中心座標
    vec3   pixel_delta_u;  // 往右一個像素的偏移量
    vec3   pixel_delta_v;  // 往下一個像素的偏移量

    // 線性代數變數:定義相機局部座標系的三個互相正交單位基底軸向向量
    vec3 u, v, w;

    // 定義實體光圈圓盤在X與Y方向上的半徑向量
    vec3 defocus_disk_u; // 景深圓盤的水平方向半徑
    vec3 defocus_disk_v; // 景深圓盤的垂直方向半徑

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        // 計算分層網格尺寸
        sqrt_spp = int(std::sqrt(samples_per_pixel));

        // 計算乘數因子
        pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp); // 確保縮放因子使用的是完美的平方數spp
        recip_sqrt_spp = 1.0 / sqrt_spp;

        // 相機中心為使用者指定的lookfrom座標點
        center = lookfrom;

        // 尺寸 焦距動態計算 = 相機位置到目標點之間的歐幾里得距離值
        // auto focal_length = (lookfrom - lookat).length();
        // 將使用者輸入的角度轉換為弧度
        auto theta = degrees_to_radians(vfov);
        // 三角函數:算出成像平面中心到頂點的相對半高度h
        auto h = std::tan(theta/2);
        // 真正視體總高度 2*h*焦距
        auto viewport_height = 2 * h * focus_dist;
        
        auto viewport_width = viewport_height * (double(image_width) / image_height);
    
        // 利用三次連續運算建立orthonormal basis
        // 第一步:用相機位置檢目標點 得到背離注視方向的Z軸 並單位化為W軸
        w = unit_vector(lookfrom - lookat);
        // 第二步:將頭頂引道向量vup與w軸進行外積(cross product)算出正右手邊的X軸 並單位化為u軸
        u = unit_vector(cross(vup, w));
        // 第三步:將w軸與右手邊u軸進行外積 反向倒推算出正交鎖定的正上方y軸 即v軸
        v = cross(w, u);
        
        // 依據新建立的相機局部座標軸向(u,-v)去投射展開成視體的邊緣向量
        vec3 viewport_u = viewport_width * u; // 沿著相機右手邊延伸的視體水平寬度向量
        vec3 viewport_v = viewport_height * -v; // 沿著相機正上方反方向(向下)延伸的視體垂直高度向量
        
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;
    
        // 視體左上角像素在世界座標系中的絕對位置 相機中心 - (焦距長度 * w軸向) - 水平向右向量的一半 - 垂直向下向量的一半
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // 計算相機虛擬光圈圓盤的物理半徑大小 半徑=對焦距離*tan(發散角/2)
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));

        // 將純量半徑乘以相機的局部軸向 u與v 轉化為3D空間中的半徑軸向量
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;

    }

    // 建立一條射向像素 (i,j) 周邊隨機採樣點的光線
    ray get_ray(int i, int j, int s_i, int s_j) const {
        // 1.在當前像素 (i,j) 內部進行二維反鋸齒微幅隨機偏移
        auto offset = sample_square_stratified(s_i, s_j);
        auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        // 2.如果 defocus_angle 設為0 代表不開景深 起點直接是相機中心center
        //    如果開啟景深 呼叫隨機函式 讓光線起點在實體光圈圓盤內部進行隨機漂移
        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        // 3.方向向量=焦點平面上的像素目標點-剛剛在光圈上隨機抽樣出來的起點座標
        auto ray_direction = pixel_sample - ray_origin;

        // 在快門開啟期間 [0,1) 內隨機抽選一個時間戳記
        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    // 在實體相機的虛擬光圈圓盤內隨機抽取一個3D空間位置點
    point3 defocus_disk_sample() const {
        // 1.取得二為單位圓盤內的隨機點
        auto p = random_in_unit_disk();
        // 2.將二維隨機點對應到相機的u軸與v軸上 並與相機中心疊加組裝出3D世界座標並回傳
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
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
        // -> 1.如果射線沒有擊中 回傳設定好的背景
        if (!world.hit(r, interval(0.001, infinity), rec))
            return background;
            
        ray scattered; // 準備用來接收材質產生的散射光線
        color attenuation; //準備用來接收材質產生的顏色衰減率

        // 2.取得交點材質主動發出光線顏色
        color color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);
            
            // 多型呼叫: 動態綁定 詢問撞擊點物體的材質如何散射光線
            // -> 如果材質不散射光線 則漫反射計算終止 直接回傳材質發光的光
            if (!rec.mat->scatter(r, rec, attenuation, scattered))
                return color_from_emission;
                
            // 若材質會散射(一般漫反射或金屬) 自身發光與反射收集的光疊加
            color color_from_scatter = attenuation * ray_color(scattered, depth-1, world);
            return color_from_emission + color_from_scatter;
        }
    
        // 背景:沒撞到求救維持原本的天空
        // vec3 unit_direction = unit_vector(r.direction());
        // auto a = 0.5 * (unit_direction.y() + 1.0);
        // return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);

        // 計算子格子內部的抖動(jittered)座標
        vec3 sample_square_stratified(int s_i, int s_j) const {
            // 將idealized單元向素[-.5,-.5]到[+.5,+.5]切隔成網格
            // 在指定的格子(s_i,s_j)內部再加上一個random_double()的隨機抖動
            auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
            auto py = ((s_j + random_double()) * recip_sqrt_spp) -0.5;
            return vec3(px, py, 0);
        }
    
};

#endif