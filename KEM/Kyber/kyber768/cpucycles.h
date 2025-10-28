#ifndef CPUCYCLES_H
#define CPUCYCLES_H

#include <stdint.h>


// 2种方法获取CPU周期数
// 1. rdpmc: 需要内核支持，且需要权限（echo 2 > /sys/devices/cpu/rdpmc）
// 2. rdtsc: 不需要权限，但可能会被乱序执行影响精度
#ifdef USE_RDPMC  /* Needs echo 2 > /sys/devices/cpu/rdpmc */

static inline uint64_t cpucycles(void) {
  const uint32_t ecx = (1U << 30) + 1;
  uint64_t result;

  __asm__ volatile ("rdpmc; shlq $32,%%rdx; orq %%rdx,%%rax"
    : "=a" (result) : "c" (ecx) : "rdx");

  return result;
}

#else

static inline uint64_t cpucycles(void) {
  uint64_t result;

  __asm__ volatile ("rdtsc; shlq $32,%%rdx; orq %%rdx,%%rax"
    : "=a" (result) : : "%rdx");

  return result;
}

#endif

uint64_t cpucycles_overhead(void);

#endif
