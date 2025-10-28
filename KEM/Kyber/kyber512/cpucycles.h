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
#error "cpucycles.h only supports x86 architectures (x86_64/i386)"
#endif
uint64_t cpucycles_overhead(void);
#endif