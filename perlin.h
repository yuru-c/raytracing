#ifndef RERLIN_H
#define PERLIN_H

#include "rtweekend.h"
#include <cmath> // 引入std::floor

class perlin {
    public:
        perlin() {
            // 隨機填滿256個不連續的浮點數值[0,1) -> 改為儲存256個隨機的單位向量
            for (int i = 0; i < point_count; i++) {
                randvec[i] = unit_vector(vec3::random(-1, 1));;
            }

            // 生成三個軸向(X,Y,Z)的整數置換表 打亂空間索引
            perlin_generate_perm(perm_x);
            perlin_generate_perm(perm_y);
            perlin_generate_perm(perm_z);
        }

        // 支援三線性插值的噪音計算
        double noise(const point3& p) const {
            // 1.分離出小數部分[0,1) 作為3D插值的權重
            auto u = p.x() - std::floor(p.x());
            auto v = p.y() - std::floor(p.y());
            auto w = p.z() - std::floor(p.z());

            

            // 2.取出當前點所在空間晶格的整數座標基底
            auto i = int(std::floor(p.x()));
            auto j = int(std::floor(p.y()));
            auto k = int(std::floor(p.z()));

            // double -> vec3向量陣列
            vec3 c[2][2][2];
            
            // 3. 走訪周圍8個晶格頂點 把各點在置換表對應的隨機浮點數取出來 存入C陣列
            for (int di=0; di < 2; di++)
                for (int dj=0; dj < 2; dj++)
                    for (int dk=0; dk < 2; dk++)
                        c[di][dj][dk] = randvec[
                        perm_x[(i+di) & 255] ^
                        perm_y[(j+dj) & 255] ^
                        perm_z[(k+dk) & 255]
                    ];

            // 4.調用三線性插值輔助函式 根據權重柔和8個點的數值
            return perlin_interp(c, u, v, w);
        }

        // 計算多層頻率疊加的紊流函式
        double turb(const point3& p, int depth) const {
            auto accum = 0.0;
            auto temp_p = p;
            auto weight = 1.0;

            for (int i = 0; i < depth; i++) {
                // 取絕對值 std::fabs 是為了製造出銳利的邊緣(類似摺痕或河流紋理)
                accum += weight * std::fabs(noise(temp_p));
                weight *= 0.5; // 權重減半(頻率越高影響力越小)
                temp_p *= 2.0; // 頻率加倍(雜訊尺寸縮小一半)
            }

            return std::fabs(accum);
        }

    private:
        static const int point_count = 256;
        vec3 randvec[point_count];
        double randfloat[point_count];
        int perm_x[point_count];
        int perm_y[point_count];
        int perm_z[point_count];

        // 初始化[0,255]數組
        static void perlin_generate_perm(int* p) {
            for (int i = 0; i < point_count; i++)
            p[i] = i;

            permute(p, point_count);
        }

        // 隨機洗牌演算法
        static void permute(int* p, int n) {
            for (int i = n-1; i > 0; i--) {
                int target = random_int(0, i);
                int tmp = p[i];
                p[i] = p[target];
                p[target] = tmp;
            }
        }

        // 三線性插值數學函式
        static double perlin_interp(const vec3 c[2][2][2], double u, double v, double w) {
            // * 利用三次曲線 f(x)=x^2(3-2x) 重新映射權重 變化率在接近經格邊界(0 1)時自動慢下來 斜率趨近於0
            auto uu = u*u*(3-2*u);
            auto vv = v*v*(3-2*v);
            auto ww = w*w*(3-2*w);
            auto accum = 0.0;
            // 在三個維度上進行加權疊加
            for (int i=0; i < 2; i++)
                for (int j=0; j < 2; j++)
                    for (int k=0; k < 2; k++) {
                        // 建立一個從晶格頂點指向採樣點的方向向量
                        vec3 weight_v(u-i, v-j, w-k);
                        accum += (i*uu + (1-i)*(1-uu)) * (j*vv + (1-j)*(1-vv)) * (k*ww + (1-k)*(1-ww)) * dot(c[i][j][k], weight_v);
                    }
            return accum;
        }
};

#endif