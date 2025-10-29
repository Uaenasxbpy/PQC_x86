#ifndef CPUCYCLES_H
#define CPUCYCLES_H
#include <stdint.h>
#include <time.h>
#if defined(__x86_64__) || defined(__i386__)
static inline uint64_t cpucycles(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
#else
#error "only x86 can do!"
#endif
uint64_t cpucycles_overhead(void);
#endif