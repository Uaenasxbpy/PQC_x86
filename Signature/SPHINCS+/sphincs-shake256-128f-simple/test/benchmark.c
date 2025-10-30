#define _POSIX_C_SOURCE 199309L
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/utsname.h>  // 系统内核信息
#include <sys/sysinfo.h>  // 内存信息
#include <string.h>       // 字符串处理
#include <stdint.h>       // uint64_t 类型

#include "../api.h"
#include "../fors.h"
#include "../wots.h"
#include "../params.h"
#include "../rng.h"

#define SPX_MLEN 32
#define NTESTS 10
#define TEST_ROUNDS 20

#if defined(__i386__) || defined(__x86_64__)
static inline uint64_t rdtsc(void) {
    unsigned int lo, hi;
    // RDTSC指令将64位计数器读入 EDX:EAX
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
#else

#warning "RDTSC (CPU cycle counter) is not supported on this architecture. Cycle counts will be 0."
static inline uint64_t rdtsc(void) {
    return 0;
}
#endif


// 新增：获取当前CPU时间（纳秒）
static uint64_t get_current_time_ns(void) {
    struct timespec ts;
    // 使用 CLOCK_PROCESS_CPUTIME_ID 来获取进程花费的CPU时间
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int compare_uint64(const void *a, const void *b) {
    uint64_t va = *(const uint64_t *)a, vb = *(const uint64_t *)b;
    return (va > vb) - (va < vb);
}

static double get_cpu_freq(void) {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) return -1.0;
    char buf[256];
    double freq = -1.0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, "cpu MHz") != NULL) {
            sscanf(buf, "cpu MHz\t: %lf", &freq);
            break;
        }
    }
    fclose(fp);
    return freq;
}

static double get_total_memory(void) {
    struct sysinfo info;
    if (sysinfo(&info) != 0) return -1.0;
    return (double)info.totalram * info.mem_unit / (1024.0 * 1024.0 * 1024.0);
}

static const char *get_compiler_name(void) {
#ifdef __GNUC__
    return "GCC";
#elif defined(__clang__)
    return "Clang";
#elif defined(_MSC_VER)
    return "MSVC";
#else
    return "Unknown";
#endif
}

static void print_platform_info(double *cpu_freq_out) {
    struct utsname un;
    if (uname(&un) != 0) {
        *cpu_freq_out = -1.0;
        return;
    }
    double cpu_freq = get_cpu_freq();
    double total_mem = get_total_memory();
    *cpu_freq_out = cpu_freq;
    printf("=============================================================\n");
    printf("                           测试平台信息                      \n");
    printf("=============================================================\n");
    printf("操作系统内核版本: %s %s\n", un.sysname, un.release);
    printf("编译器          : %s %s\n", get_compiler_name(), __VERSION__);
    printf("CPU基础频率     : %.2f MHz\n", (cpu_freq >= 0) ? cpu_freq : -1.0);
    printf("系统总内存      : %.2f GB\n", (total_mem >= 0) ? total_mem : -1.0);
    printf("测试次数        : %d 次\n", TEST_ROUNDS);
    printf("=============================================================\n\n");
}

static void calc_stats(uint64_t *data, int len, double *avg, uint64_t *median, uint64_t *min, uint64_t *max) {
    if (len == 0 || data == NULL) return;
    *min = data[0];
    *max = data[0];
    uint64_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data[i];
        if (data[i] < *min) *min = data[i];
        if (data[i] > *max) *max = data[i];
    }
    *avg = (double)sum / len;
    qsort(data, len, sizeof(uint64_t), compare_uint64);
    if (len % 2 == 0) *median = (data[len/2 - 1] + data[len/2]) / 2;
    else *median = data[len/2];
}

static int cmp_llu(const void *a, const void*b) {
    if(*(unsigned long long *)a < *(unsigned long long *)b) return -1;
    if(*(unsigned long long *)a > *(unsigned long long *)b) return 1;
    return 0;
}

static unsigned long long median(unsigned long long *l, size_t llen) {
    qsort(l, llen, sizeof(unsigned long long), cmp_llu);
    if(llen%2) return l[llen/2];
    else return (l[llen/2-1]+l[llen/2])/2;
}

static void delta(unsigned long long *l, size_t llen) {
    unsigned int i;
    for(i = 0; i < llen - 1; i++) {
        l[i] = l[i+1] - l[i];
    }
}

static void printfcomma(unsigned long long n) {
    if (n < 1000) {
        printf("%llu", n);
        return;
    }
    printfcomma(n / 1000);
    printf(",%03llu", n % 1000);
}

static void printfalignedcomma(unsigned long long n, int len) {
    unsigned long long ncopy = n;
    int i = 0;

    while (ncopy > 9) {
        len -= 1;
        ncopy /= 10;
        i += 1;
    }
    i = i/3 - 1;
    for (; i < len; i++) {
        printf(" ");
    }
    printfcomma(n);
}

static void display_result(double result, unsigned long long *l, size_t llen, unsigned long long mul) {
    unsigned long long med;

    result /= NTESTS;
    delta(l, NTESTS + 1);
    med = median(l, llen);
    printf("avg. %11.2lf us (%2.2lf sec); median ", result / 1000.0, result / 1e9);
    printfalignedcomma(med, 12);
    printf(" ns,  %5llux: ", mul);
    printfalignedcomma(mul*med, 12);
    printf(" ns\n");
}

#define MEASURE(TEXT, MUL, FNCALL)\
    printf(TEXT);\
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);\
    for(i = 0; i < NTESTS; i++) {\
        t[i] = get_current_time_ns();\
        FNCALL;\
    }\
    t[NTESTS] = get_current_time_ns();\
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);\
    result = (double)((stop.tv_sec - start.tv_sec) * 1000000000ULL + (stop.tv_nsec - start.tv_nsec));\
    display_result(result, t, NTESTS, MUL);
// ... (以上部分保持不变) ...


int main() {
    double cpu_freq;
    print_platform_info(&cpu_freq);
    setbuf(stdout, NULL);

    unsigned char pk[SPX_PK_BYTES];
    unsigned char sk[SPX_SK_BYTES];
    unsigned char *m = malloc(SPX_MLEN);
    unsigned char *sm = malloc(SPX_BYTES + SPX_MLEN);
    unsigned char *mout = malloc(SPX_BYTES + SPX_MLEN);

    unsigned char fors_pk[SPX_FORS_PK_BYTES];
    unsigned char fors_m[SPX_FORS_MSG_BYTES];
    unsigned char fors_sig[SPX_FORS_BYTES];
    unsigned char addr[SPX_ADDR_BYTES];

    unsigned char wots_sig[SPX_WOTS_BYTES];
    unsigned char wots_m[SPX_N];
    unsigned char wots_pk[SPX_WOTS_PK_BYTES];

    unsigned long long smlen;
    unsigned long long mlen;
    unsigned long long t[NTESTS+1];

    // 修改：重命名数组以区分纳秒和周期
    uint64_t keypair_ns[TEST_ROUNDS] = {0};   // 存储纳秒时间
    uint64_t sign_ns[TEST_ROUNDS] = {0};      // 存储纳秒时间
    uint64_t verify_ns[TEST_ROUNDS] = {0};    // 存储纳秒时间

    // 新增：用于存储CPU周期的数组
    uint64_t keypair_cycles[TEST_ROUNDS] = {0};
    uint64_t sign_cycles[TEST_ROUNDS] = {0};
    uint64_t verify_cycles[TEST_ROUNDS] = {0};

    struct timespec start, stop;
    double result;
    int i;

    randombytes(m, SPX_MLEN);
    randombytes(addr, SPX_ADDR_BYTES);

    printf("Parameters: n = %d, h = %d, d = %d, b = %d, k = %d, w = %d\n",
           SPX_N, SPX_FULL_HEIGHT, SPX_D, SPX_FORS_HEIGHT, SPX_FORS_TREES,
           SPX_WOTS_W);

    printf("Running %d iterations.\n", NTESTS);

    // (MEASURE 宏部分保持不变，它提供了快速的初步测试)
    MEASURE("Generating keypair.. ", 1, crypto_sign_keypair(pk, sk));
    MEASURE("  - WOTS pk gen..    ", (1 << SPX_TREE_HEIGHT), wots_gen_pk(wots_pk, sk, pk, (uint32_t *) addr));
    MEASURE("Signing..            ", 1, crypto_sign(sm, &smlen, m, SPX_MLEN, sk));
    MEASURE("  - FORS signing..   ", 1, fors_sign(fors_sig, fors_pk, fors_m, sk, pk, (uint32_t *) addr));
    MEASURE("  - WOTS signing..   ", SPX_D, wots_sign(wots_sig, wots_m, sk, pk, (uint32_t *) addr));
    MEASURE("  - WOTS pk gen..    ", SPX_D * (1 << SPX_TREE_HEIGHT), wots_gen_pk(wots_pk, sk, pk, (uint32_t *) addr));
    MEASURE("Verifying..          ", 1, crypto_sign_open(mout, &mlen, sm, smlen, pk));

    printf("\n正在执行 %d 次详细统计测试 (纳秒 + 周期)...\n", TEST_ROUNDS);
    for (int j = 0; j < TEST_ROUNDS; j++) {
        uint64_t start_ns, end_ns;
        // 新增：周期变量
        uint64_t start_cy, end_cy;

        // --- 密钥对生成 ---
        start_ns = get_current_time_ns(); // 1. 开始计时
        start_cy = rdtsc();               // 2. 开始计周期

        crypto_sign_keypair(pk, sk);      // 3. 执行函数

        end_cy = rdtsc();                 // 4. 结束计周期
        end_ns = get_current_time_ns();   // 5. 结束计时

        keypair_ns[j] = end_ns - start_ns;
        keypair_cycles[j] = end_cy - start_cy; // 存储周期

        // --- 签名 ---
        start_ns = get_current_time_ns();
        start_cy = rdtsc();

        crypto_sign(sm, &smlen, m, SPX_MLEN, sk);

        end_cy = rdtsc();
        end_ns = get_current_time_ns();

        sign_ns[j] = end_ns - start_ns;
        sign_cycles[j] = end_cy - start_cy; // 存储周期

        // --- 验证 ---
        start_ns = get_current_time_ns();
        start_cy = rdtsc();

        crypto_sign_open(mout, &mlen, sm, smlen, pk);

        end_cy = rdtsc();
        end_ns = get_current_time_ns();

        verify_ns[j] = end_ns - start_ns;
        verify_cycles[j] = end_cy - start_cy; // 存储周期
    }

    // (时间统计变量)
    double kp_avg_ns, sign_avg_ns, verify_avg_ns;
    uint64_t kp_med_ns, sign_med_ns, verify_med_ns;
    uint64_t kp_min_ns, sign_min_ns, verify_min_ns;
    uint64_t kp_max_ns, sign_max_ns, verify_max_ns;

    // 新增：周期的统计变量
    double kp_avg_cy, sign_avg_cy, verify_avg_cy;
    uint64_t kp_med_cy, sign_med_cy, verify_med_cy;
    uint64_t kp_min_cy, sign_min_cy, verify_min_cy;
    uint64_t kp_max_cy, sign_max_cy, verify_max_cy;


    // 统计纳秒级时间 (修改：使用 ..._ns 数组)
    calc_stats(keypair_ns, TEST_ROUNDS, &kp_avg_ns, &kp_med_ns, &kp_min_ns, &kp_max_ns);
    calc_stats(sign_ns, TEST_ROUNDS, &sign_avg_ns, &sign_med_ns, &sign_min_ns, &sign_max_ns);
    calc_stats(verify_ns, TEST_ROUNDS, &verify_avg_ns, &verify_med_ns, &verify_min_ns, &verify_max_ns);

    // 新增：统计CPU周期
    calc_stats(keypair_cycles, TEST_ROUNDS, &kp_avg_cy, &kp_med_cy, &kp_min_cy, &kp_max_cy);
    calc_stats(sign_cycles, TEST_ROUNDS, &sign_avg_cy, &sign_med_cy, &sign_min_cy, &sign_max_cy);
    calc_stats(verify_cycles, TEST_ROUNDS, &verify_avg_cy, &verify_med_cy, &verify_min_cy, &verify_max_cy);


    // 纳秒→毫秒换算 (保持不变)
    double kp_avg_ms = kp_avg_ns / 1e6;
    double kp_med_ms = kp_med_ns / 1e6;
    double kp_min_ms = kp_min_ns / 1e6;
    double kp_max_ms = kp_max_ns / 1e6;
    double sign_avg_ms = sign_avg_ns / 1e6;
    double sign_med_ms = sign_med_ns / 1e6;
    double sign_min_ms = sign_min_ns / 1e6;
    double sign_max_ms = sign_max_ns / 1e6;
    double verify_avg_ms = verify_avg_ns / 1e6;
    double verify_med_ms = verify_med_ns / 1e6;
    double verify_min_ms = verify_min_ns / 1e6;
    double verify_max_ms = verify_max_ns / 1e6;


    // (毫秒时间输出表格，保持不变)
    printf("=======================================================================\n");
    printf("                sphincs-shake256-128f 性能测试结果（时间：毫秒）               \n");
    printf("=======================================================================\n");
    printf("%-15s | %-12s | %-12s | %-12s | %-12s\n",
           "测试项", "平均值(ms)", "中位数(ms)", "最小值(ms)", "最大值(ms)");
    printf("-----------------------------------------------------------------------\n");
    printf("%-15s | %-12.6f | %-12.6f | %-12.6f | %-12.6f\n",
           "密钥", kp_avg_ms, kp_med_ms, kp_min_ms, kp_max_ms);
    printf("%-15s | %-12.6f | %-12.6f | %-12.6f | %-12.6f\n",
           "签名", sign_avg_ms, sign_med_ms, sign_min_ms, sign_max_ms);
    printf("%-15s | %-12.6f | %-12.6f | %-12.6f | %-12.6f\n",
           "验证", verify_avg_ms, verify_med_ms, verify_min_ms, verify_max_ms);
    printf("=======================================================================\n");

    // 新增：CPU 周期数统计表
    printf("=======================================================================\n");
    printf("                sphincs-shake256-128f 性能测试结果（CPU 周期数）             \n");
    printf("=======================================================================\n");
    printf("%-15s | %-15s | %-15s | %-15s | %-15s\n",
           "测试项", "平均值(cy)", "中位数(cy)", "最小值(cy)", "最大值(cy)");
    printf("-----------------------------------------------------------------------\n");
    // 注意：周期数是非常大的数字，我们使用 PRIu64 来打印
    printf("%-15s | %-15.0f | %-15" PRIu64 " | %-15" PRIu64 " | %-15" PRIu64 "\n",
           "密钥", kp_avg_cy, kp_med_cy, kp_min_cy, kp_max_cy);
    printf("%-15s | %-15.0f | %-15" PRIu64 " | %-15" PRIu64 " | %-15" PRIu64 "\n",
           "签名", sign_avg_cy, sign_med_cy, sign_min_cy, sign_max_cy);
    printf("%-15s | %-15.0f | %-15" PRIu64 " | %-15" PRIu64 " | %-15" PRIu64 "\n",
           "验证", verify_avg_cy, verify_med_cy, verify_min_cy, verify_max_cy);
    printf("=======================================================================\n");


    printf("Signature size: %d (%.2f KiB)\n", SPX_BYTES, SPX_BYTES / 1024.0);
    printf("Public key size: %d (%.2f KiB)\n", SPX_PK_BYTES, SPX_PK_BYTES / 1024.0);
    printf("Secret key size: %d (%.2f KiB)\n", SPX_SK_BYTES, SPX_SK_BYTES / 1024.0);

    free(m);
    free(sm);
    free(mout);

    return 0;
}