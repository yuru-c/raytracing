#include "rtweekend.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

int main() {
    // world 場景物件建構
    hittable_list world;

    // 建立各式各樣不同的材質實例
    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0)); // 綠黃色泥土
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5)); // 藍色漫反射球
    // 1.宣告外部厚玻璃材質 (空氣到玻璃=1.50)
    auto material_left = make_shared<dielectric>(1.50);
    // 2.宣告內部空氣泡材質 (玻璃道控器=1.00/1.50=0.67)
    auto material_bubble = make_shared<dielectric>(1.00 / 1.50);
    auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);      // 金色金屬球

    // 將材質指標綁定到各自對應的球體上
    world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<sphere>(point3( 0.0, 0.0, -1.2), 0.5, material_center));
    // 外球半徑0.5使用1.50玻璃材質
    world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
    // 同圓心位置塞入半徑0.4的內球 綁定0.67的反向空氣材質
    world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.4,material_bubble));
    world.add(make_shared<sphere>(point3( 1.0, 0.0, -1.0), 0.5, material_right));
    
    // camera 相機參數配置與啟動
    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 100; // 每個像素的採樣數量(反鋸齒)
    cam.max_depth = 50; // 光線最多可在場景內連續反彈50次

    // 一鍵啟動渲染流程
    cam.render(world);
}