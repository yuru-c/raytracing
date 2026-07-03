#include "rtweekend.h"
#include "bvh.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "texture.h"

// 場景1:原本的動態隨機漫天球體
void bouncing_spheres() {
    // world 場景物件建構
    hittable_list world;
    
    // 建立一個中性灰色的漫反射材質，當作一望無際的宏大地面
    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));    
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));    
    
    // 👈 雙重嵌套迴圈：在 X 軸與 Z 軸從 -11 到 +10 的網格區間內（共 22x22 = 484 個潛在位置）隨機撒球
    for (int a = -11; a < 11; a++) {        
        for (int b = -11; b < 11; b++) {            
            auto choose_mat = random_double(); // 抽籤機：決定這個位置要放哪種材質
            
            // 隨機微調球心位置，讓球體看起來散落得更自然，而不是死板地排成正方形陣列
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());            
            
            // 防禦性幾何碰撞檢測：確保隨機生成的無數小球，不會無情地穿插進前排那三顆大主角球的體積內（距離大於 0.9）
            if ((center - point3(4, 0.2, 0)).length() > 0.9) {                
                shared_ptr<material> sphere_material;                
                
                if (choose_mat < 0.8) {                    
                    // 🔴 80% 的機率生成：漫反射小球（Diffuse）
                    // 物理魔法：兩個隨機顏色向量進行「分量相乘（Hadamard product）」，可以自然調配出更飽和、層次更豐富的隨機色調值值
                    auto albedo = color::random() * color::random();                    
                    sphere_material = make_shared<lambertian>(albedo);     
                    // 計算時間 t=1 時的終點球心位置：在 Y 軸方向加上一個 [0, 0.5) 之間的隨機彈跳高度
                    auto center2 = center + vec3(0, random_double(0,.5), 0);                    
                    // 呼叫全新動態球體建構子：傳入 center1 與 center2
                    world.add(make_shared<sphere>(center, center2, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {                    
                    // 🟡 15% 的機率生成：磨砂金屬小球（Metal）
                    auto albedo = color::random(0.5, 1);    // 偏亮、高反射率的隨機金屬顏色值
                    auto fuzz = random_double(0, 0.5);     // 隨機的粗糙磨砂度 (0.0 代表鏡面)
                    sphere_material = make_shared<metal>(albedo, fuzz);                    
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));                
                } else {                    
                    // 🔵 5% 的機率生成：純淨玻璃小球（Glass / Dielectric）
                    sphere_material = make_shared<dielectric>(1.5); // 折射率設為 1.5                    
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));                
                }            
            }        
        }    
    }    
    
    // 1. 中間大球：100% 純淨無瑕的折射玻璃球
    auto material1 = make_shared<dielectric>(1.5);    
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));    
    
    // 2. 左側大球：帶有溫暖泥土色調的暗紅漫反射球
    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));    
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));    
    
    // 3. 右側大球：帶著高貴淡金色澤、完美鏡面的絕美金屬球 (fuzz=0.0)
    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);    
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));
    
    // 將整個塞滿百顆球的world列表丟進bvh_node
    world = hittable_list(make_shared<bvh_node>(world));

    // camera 相機參數配置與啟動
    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 100; // 每個像素的採樣數量(反鋸齒)
    cam.max_depth = 50; // 光線最多可在場景內連續反彈50次
    cam.vfov = 20; // 垂直視野強行收窄到 20° 望遠變焦
    cam.lookfrom = point3(13, 2, 3); // 相機拉到極遠的斜高空
    cam.lookat = point3(0, 0, 0); // 注視座標原點
    cam.vup = vec3(0, 1, 0); // 保持地平線水平向上

    // 光圈設定
    cam.defocus_angle = 0.6; // 設定發散圓錐角為 0.6°
    cam.focus_dist = 10.0; // 完美聚焦在距離 10 單位的主角球區域

    // 一鍵啟動渲染流程
    cam.render(world);
}

// 場景2:兩顆巨大的 3D 空間格子球
void checkered_spheres() {
    hittable_list world;

    // 建立一個縮放係數為0.32的棋盤格紋理
    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));

    // 將同一個材質套用給上下相疊的兩顆大球
    world.add(make_shared<sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));
    world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    
    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(world);
}

// 場景3:貼上地球表面圖片的蔚藍地球儀
void earth() {
    // 讀取執行目標下的地球貼圖
    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    
    cam.vfov     = 20;
    cam.lookfrom = point3(0,0,12); // 正對著 z 軸看過去
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(hittable_list(globe));
}

int main() {
    switch (3){
        case 1: bouncing_spheres(); break;
        case 2: checkered_spheres(); break;
        case 3: earth(); break;
    }
    
    
}