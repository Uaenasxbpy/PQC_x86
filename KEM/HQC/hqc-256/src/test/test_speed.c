#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>  // 系统内核信息
#include <sys/sysinfo.h>  // 内存信息
#include <string.h>       // 字符串处理
#include <stdint.h>       // uint64_t 类型

// 项目原有头文件
#include "./api.h"
#include "./parameters.h"
#include "cpucycles.h"

// 测试次数（1000次）
#define TEST_ROUNDS 1000

// 排序函数：用于计算中位数（qsort 依赖）
int compare_uint64(const void *a, const void *b) {
    return (*(uint64_t *)a - *(uint64_t *)b);
}

// 函数：获取CPU基础频率（单位：MHz）
static double get_cpu_freq(void) {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/cpuinfo");
        return -1.0;  // 标记获取失败
    }

    char buf[256];
    double freq = -1.0;
    // 读取 cpuinfo，提取第一个核心的 "cpu MHz" 字段（多核心频率通常一致）
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, "cpu MHz") != NULL) {
            sscanf(buf, "cpu MHz\t: %lf", &freq);
            break;
        }
    }

    fclose(fp);
    return freq;
}

// 函数：获取系统总内存（单位：GB）
static double get_total_memory(void) {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        perror("Failed to get sysinfo");
        return -1.0;  // 标记获取失败
    }
    // 转换公式：总内存（字节）= totalram * mem_unit，再转 GB（1GB=1024^3 字节）
    return (double)info.totalram * info.mem_unit / (1024.0 * 1024.0 * 1024.0);
}

// 函数：获取编译器名称（GCC/Clang 等）
static const char *get_compiler_name(void) {
#ifdef __GNUC__
    return "GCC";          // GNU Compiler Collection
#elif defined(__clang__)
    return "Clang";        // LLVM Clang
#elif defined(_MSC_VER)
    return "MSVC";         // Microsoft Visual C++（Windows 平台，此处备用）
#else
    return "Unknown";      // 未知编译器
#endif
}

// 函数：打印测试平台信息（新增编译器名称）
static void print_platform_info(double *cpu_freq_out) {
    struct utsname un;
    // 1. 获取系统内核信息
    if (uname(&un) != 0) {
        perror("Failed to get uname info");
        *cpu_freq_out = -1.0;
        return;
    }

    // 2. 获取硬件信息（CPU频率、内存）
    double cpu_freq = get_cpu_freq();
    double total_mem = get_total_memory();
    *cpu_freq_out = cpu_freq;  // 传出CPU频率，用于后续ms换算

    // 3. 格式化打印（补充编译器名称）
    printf("=============================================================\n");
    printf("                      测试平台信息                            \n");
    printf("=============================================================\n");
    printf("操作系统内核版本: %s %s\n", un.sysname, un.release);
    printf("编译器          : %s %s\n", get_compiler_name(), __VERSION__);  // 新增编译器名称
    printf("CPU基础频率     : %.2f MHz\n", (cpu_freq >= 0) ? cpu_freq : -1.0);
    printf("系统总内存      : %.2f GB\n", (total_mem >= 0) ? total_mem : -1.0);
    printf("测试次数        : %d 次\n", TEST_ROUNDS);
    printf("=============================================================\n\n");
}

// 函数：计算数组的均值、中位数、最小值、最大值
static void calc_stats(uint64_t *data, int len, 
                       double *avg, uint64_t *median, 
                       uint64_t *min, uint64_t *max) {
    if (len == 0 || data == NULL) return;

    // 1. 计算总和、最小值、最大值
    *min = data[0];
    *max = data[0];
    uint64_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data[i];
        *min = (data[i] < *min) ? data[i] : *min;
        *max = (data[i] > *max) ? data[i] : *max;
    }

    // 2. 计算均值（用double避免整数溢出）
    *avg = (double)sum / len;

    // 3. 计算中位数（排序后取中间位置，偶数长度取平均）
    qsort(data, len, sizeof(uint64_t), compare_uint64);
    if (len % 2 == 0) {
        *median = (data[len/2 - 1] + data[len/2]) / 2;
    } else {
        *median = data[len/2];
    }
}

// 辅助函数：将周期数换算为ms（基于CPU频率）
static double cycles_to_ms(uint64_t cycles, double cpu_freq) {
    if (cpu_freq <= 0) return -1.0;  // 频率获取失败，返回无效值
    // 换算公式：ms = 周期数 / (CPU频率(MHz) * 1000) 
    // 推导：1MHz=1e6周期/秒 → 1周期=1/(1e6*freq)秒 → 转ms需×1000 → 1周期=1/(1e3*freq) ms
    return (double)cycles / (cpu_freq * 1000.0);
}

int main() {
    double cpu_freq;  // 存储CPU频率（用于后续ms换算）
    // 1. 打印平台信息（传出CPU频率）
    print_platform_info(&cpu_freq);

    // 2. 定义算法相关变量
    unsigned char pk[PUBLIC_KEY_BYTES];
    unsigned char sk[SECRET_KEY_BYTES];
    unsigned char ct[CIPHERTEXT_BYTES];
    unsigned char key1[SHARED_SECRET_BYTES];
    unsigned char key2[SHARED_SECRET_BYTES];

    // 3. 定义计时数组（存储1000次测试的周期耗时）
    uint64_t keypair_cycles[TEST_ROUNDS] = {0};
    uint64_t enc_cycles[TEST_ROUNDS] = {0};
    uint64_t dec_cycles[TEST_ROUNDS] = {0};

    // 4. 预热测试（避免CPU冷启动导致前几次结果偏差）
    crypto_kem_keypair(pk, sk);
    crypto_kem_enc(ct, key1, pk);
    crypto_kem_dec(key2, ct, sk);

    // 5. 执行1000次测试，记录每次耗时（周期数）
    printf("正在执行 %d 次测试...\n", TEST_ROUNDS);
    for (int i = 0; i < TEST_ROUNDS; i++) {
        uint64_t start, end;

        // 密钥对生成计时
        start = cpucycles();
        crypto_kem_keypair(pk, sk);
        end = cpucycles();
        keypair_cycles[i] = end - start;

        // 加密计时
        start = cpucycles();
        crypto_kem_enc(ct, key1, pk);
        end = cpucycles();
        enc_cycles[i] = end - start;

        // 解密计时
        start = cpucycles();
        crypto_kem_dec(key2, ct, sk);
        end = cpucycles();
        dec_cycles[i] = end - start;
    }

    // 6. 计算各测试项的周期统计指标
    double kp_avg_cy, enc_avg_cy, dec_avg_cy;
    uint64_t kp_med_cy, enc_med_cy, dec_med_cy;
    uint64_t kp_min_cy, enc_min_cy, dec_min_cy;
    uint64_t kp_max_cy, enc_max_cy, dec_max_cy;

    calc_stats(keypair_cycles, TEST_ROUNDS, &kp_avg_cy, &kp_med_cy, &kp_min_cy, &kp_max_cy);
    calc_stats(enc_cycles, TEST_ROUNDS, &enc_avg_cy, &enc_med_cy, &enc_min_cy, &enc_max_cy);
    calc_stats(dec_cycles, TEST_ROUNDS, &dec_avg_cy, &dec_med_cy, &dec_min_cy, &dec_max_cy);

    // 7. 换算为ms（基于CPU频率）
    // 周期统计 → ms统计（失败时标记为-1.0）
    double kp_avg_ms = cycles_to_ms((uint64_t)kp_avg_cy, cpu_freq);
    double kp_med_ms = cycles_to_ms(kp_med_cy, cpu_freq);
    double kp_min_ms = cycles_to_ms(kp_min_cy, cpu_freq);
    double kp_max_ms = cycles_to_ms(kp_max_cy, cpu_freq);

    double enc_avg_ms = cycles_to_ms((uint64_t)enc_avg_cy, cpu_freq);
    double enc_med_ms = cycles_to_ms(enc_med_cy, cpu_freq);
    double enc_min_ms = cycles_to_ms(enc_min_cy, cpu_freq);
    double enc_max_ms = cycles_to_ms(enc_max_cy, cpu_freq);

    double dec_avg_ms = cycles_to_ms((uint64_t)dec_avg_cy, cpu_freq);
    double dec_med_ms = cycles_to_ms(dec_med_cy, cpu_freq);
    double dec_min_ms = cycles_to_ms(dec_min_cy, cpu_freq);
    double dec_max_ms = cycles_to_ms(dec_max_cy, cpu_freq);

    // 8. 双表格打印结果（周期数表格 + ms表格）
    printf("=======================================================================\n");
    printf("                      HQC-256 性能测试结果（周期数）          \n");
    printf("=======================================================================\n");
    printf("%-15s | %-12s | %-12s | %-12s | %-12s\n", 
           "测试项", "平均值(周期)", "中位数(周期)", "最小值(周期)", "最大值(周期)");
    printf("-------------------------------------------------------------\n");
    printf("%-15s | %-12.0f | %-12lu | %-12lu | %-12lu\n", 
           "密钥对生成", kp_avg_cy, kp_med_cy, kp_min_cy, kp_max_cy);
    printf("%-15s | %-12.0f | %-12lu | %-12lu | %-12lu\n", 
           "加密", enc_avg_cy, enc_med_cy, enc_min_cy, enc_max_cy);
    printf("%-15s | %-12.0f | %-12lu | %-12lu | %-12lu\n", 
           "解密", dec_avg_cy, dec_med_cy, dec_min_cy, dec_max_cy);
    printf("=======================================================================\n");

    // ms 表格（频率获取失败时显示 N/A）
    printf("=======================================================================\n");

    printf("                      HQC-256 性能测试结果（时间）            \n");
    printf("=======================================================================\n");
    printf("%-15s | %-12s | %-12s | %-12s | %-12s\n", 
           "测试项", "平均值(ms)", "中位数(ms)", "最小值(ms)", "最大值(ms)");
    printf("-----------------------------------------------------------------------\n");
    // 密钥对生成行（处理频率获取失败的情况）
    if (cpu_freq > 0) {
        printf("%-15s | %-12.6f | %-12.6f | %-12.6f | %-12.6f\n", 
               "密钥对生成", kp_avg_ms, kp_med_ms, kp_min_ms, kp_max_ms);
        printf("%-15s | %-12.6f | %-12.6f | %-12.6f | %-12.6f\n", 
               "加密", enc_avg_ms, enc_med_ms, enc_min_ms, enc_max_ms);
        printf("%-15s | %-12.6f | %-12.6f | %-12.6f | %-12.6f\n", 
               "解密", dec_avg_ms, dec_med_ms, dec_min_ms, dec_max_ms);
    } else {
        printf("%-15s | %-12s | %-12s | %-12s | %-12s\n", 
               "密钥对生成", "N/A", "N/A", "N/A", "N/A");
        printf("%-15s | %-12s | %-12s | %-12s | %-12s\n", 
               "加密", "N/A", "N/A", "N/A", "N/A");
        printf("%-15s | %-12s | %-12s | %-12s | %-12s\n", 
               "解密", "N/A", "N/A", "N/A", "N/A");
        printf("\n⚠️  提示：CPU频率获取失败，无法换算时间（ms）\n");
    }
    printf("=======================================================================\n");

    // 9. 算法正确性验证（确保测试数据有效）
    if (memcmp(key1, key2, SHARED_SECRET_BYTES) != 0) {
        printf("\n⚠️  警告：最后一次测试中，加密密钥与解密密钥不匹配！\n");
    } else {
        printf("\n✅ 最后一次测试验证：加密密钥与解密密钥匹配，算法逻辑正常。\n");
    }

    return 0;
}