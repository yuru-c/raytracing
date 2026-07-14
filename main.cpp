#include "rtweekend.h"
#include "bvh.h"
#include "camera.h"
#include "constant_medium.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "quad.h"
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
    cam.background = color(0.70, 0.80, 1.00); // 天空藍白色
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
    cam.background = color(0.70, 0.80, 1.00); // 天空藍白色
    
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
    cam.background = color(0.70, 0.80, 1.00); // 天空藍白色
    
    cam.vfov     = 20;
    cam.lookfrom = point3(0,0,12); // 正對著 z 軸看過去
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(hittable_list(globe));
}

// 場景4:兩個為平滑的塊狀雜訊球體
void perlin_spheres() {
    hittable_list world;
    auto pertext = make_shared<noise_texture>();

    // 建立地面大球與空中懸浮小球
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background = color(0.70, 0.80, 1.00); // 天空藍白色
    
    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(world);
}

// 場景5:五彩四邊形保持不變
void quads() {
    hittable_list world;

    // 建立5種唇色漫反射材質
    auto left_red = make_shared<lambertian>(color(1.0, 0.2, 0.2));
    auto back_green = make_shared<lambertian>(color(0.2, 1.0, 0.2));
    auto right_blue = make_shared<lambertian>(color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<lambertian>(color(1.0, 0.5, 0.0));
    auto lower_teal = make_shared<lambertian>(color(0.2, 0.8, 0.8));

    // 向世界加入5個方向不同的四邊形平面
    world.add(make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
    world.add(make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
    world.add(make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
    world.add(make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

    camera cam;
    cam.aspect_ratio      = 1.0; // 此場景原著指定為1:1正方形
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background = color(0.70, 0.80, 1.00); // 天空藍白色
    
    cam.vfov     = 80;
    cam.lookfrom = point3(0,0,9);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(world);
}

// 場景6:
void simple_light() {
    hittable_list world;

    // 建立柏林雜訊紋理
    auto pertext = make_shared<noise_texture>(4);

    // 地面大球體與空中小球體(一般漫反射材質)
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    // 建立發光強度為(4,4,4)的超亮白光材質
    auto difflight = make_shared<diffuse_light>(color(4,4,4));    

    // 在右上方放置一個發光四邊形(矩形燈)
    world.add(make_shared<sphere>(point3(0,7,0), 2, difflight));

    // （可選）原著 Listing 60 提到的發光球體，如果想加也可以取消註解：
    world.add(make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    // 背景設為純黑 確保唯一光線來源就是發光四邊形
    cam.background        = color(0,0,0);

    cam.vfov     = 20;
    cam.lookfrom = point3(26,3,6);
    cam.lookat   = point3(0,2,0);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(world);
}

// 場景7:空康乃爾盒子場景
void cornell_box() {
    hittable_list world;

    // 1.定義3種漫反射材質與1種強發光材質
    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15)); // 強烈的白光

    // 2.構建5面牆壁與1個天空板光源(全由quad四邊形組成)
    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green)); // 右牆 (綠)
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));   // 左牆 (紅)
    world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light)); // 頂光
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white)); // 地板 (白)
    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white)); // 天花板 (白)
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white)); // 後牆 (白)

    shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));
    world.add(box2);

    camera cam;
    cam.aspect_ratio = 1.0;
    cam.image_width = 600;  // 提高解析度以觀察質感
    cam.samples_per_pixel = 200;  // 採樣數提高到 200
    cam.max_depth = 50;
    cam.background = color(0,0,0); // 純黑背景

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(world);
}

// 場景8:煙霧版康乃爾盒子
void cornell_smoke() {
    hittable_list world;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7)); // 更大更溫和的光源

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(113,554,127), vec3(330,0,0), vec3(0,0,305), light)); // 擴大後的頂燈
    world.add(make_shared<quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    // 建立原本的兩個3D方塊作為外殼
    shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));

    shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));

    // 將兩個方塊外殼包裝進constant_medium中 轉化為黑煙與白霧
    world.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0))); // 高方塊變成黑煙(密度0.01)
    world.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1))); // 矮方塊變成白霧(密度0.01)

    camera cam;
    cam.aspect_ratio      = 1.0;
    cam.image_width       = 600;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(world);
}

// 場景9
void final_scence(int image_width, int samples_per_pixel, int max_depth) {
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));
    int boxes_per_side = 20;

    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 1010); // 隨機生成高度1-100的方塊
            auto z1 = z0 + w;

            boxes1.add(box(point3(x0,y0,z0), point3(x1,y1,z1), ground));
        }
    }
    hittable_list world;
    // 將400個方塊全部放入BVH加速樹中
    world.add(make_shared<bvh_node>(boxes1));

    auto light = make_shared<diffuse_light>(color(7,7,7));
    world.add(make_shared<quad>(point3(123,554,147), vec3(300,0,0), vec3(0,0,265), light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30, 0, 0);
    auto sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    world.add(make_shared<sphere>(center1, center2, 50, sphere_material));

    world.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    world.add(make_shared<sphere>(
        point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
    ));

    // 次表面散射球(外殼玻璃+內含高密度藍色參與介介質)
    auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
    world.add(boundary);
    world.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));

    // 全局大氣稀薄迷霧
    boundary = make_shared<sphere>(point3(0,0,0), 5000, make_shared<dielectric>(1.5));
    world.add(make_shared<constant_medium>(boundary, .0001, color(1,1,1)));

    auto emat = make_shared<lambertian>(make_shared<image_texture>("earthmap.jpg"));
    world.add(make_shared<sphere>(point3(400,200,400), 100, emat));

    auto pertext = make_shared<noise_texture>(0.2);
    world.add(make_shared<sphere>(point3(220,280,300), 80, make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<sphere>(point3::random(0,165), 10, white));
    }

    world.add(make_shared<translate>(
        make_shared<rotate_y>(
            make_shared<bvh_node>(boxes2), 15),
            vec3(-100,270,395)
        )
    );

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = image_width;
    cam.samples_per_pixel = samples_per_pixel;
    cam.max_depth         = max_depth;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = point3(478, 278, -600);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(world);
}

// 場景10:空康乃爾盒子場景(分層)
void cornell_box2() {
    hittable_list world;

    // 1.定義3種漫反射材質與1種強發光材質
    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15)); // 強烈的白光

    // 2.構建5面牆壁與1個天空板光源(全由quad四邊形組成)
    world.add(make_shared<quad>(point3(555,0,0), vec3(0,0,555), vec3(0,555,0), green));
    world.add(make_shared<quad>(point3(0,0,555), vec3(0,0,-555), vec3(0,555,0), red));
    world.add(make_shared<quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(555,0,555), vec3(-555,0,0), vec3(0,555,0), white));

    // 頂部光源
    world.add(make_shared<quad>(point3(213,554,227), vec3(130,0,0), vec3(0,0,105), light));

    // 方塊 1
    shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    // 方塊 2
    shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));
    world.add(box2);

    camera cam;
    cam.aspect_ratio = 1.0;
    cam.image_width = 600;  // 提高解析度以觀察質感
    cam.samples_per_pixel = 64;  // 採樣數字動切割成完美的8*8網格分層採樣
    cam.max_depth = 50;
    cam.background = color(0,0,0); // 純黑背景

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    cam.render(world);
}

int main() {
    switch (10){
        case 1: bouncing_spheres(); break;
        case 2: checkered_spheres(); break;
        case 3: earth(); break;
        case 4: perlin_spheres(); break;
        case 5: quads(); break;
        case 6: simple_light(); break;
        case 7: cornell_box(); break;
        case 8: cornell_smoke(); break;
        case 9: final_scence(800, 10000, 40); break;
        case 10: cornell_box2(); break;
        default: final_scence(400, 250, 4); break;
    }
    
    
}