#include "rtweekend.h"
#include "camera.h"
#include "hittable_list.h"
#include "sphere.h"

int main() {
    // world 場景物件建構
    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

    // camera 相機參數配置與啟動
    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;

    // 一鍵啟動渲染流程
    cam.render(world);
}