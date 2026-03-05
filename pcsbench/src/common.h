#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

#include <sys/sysinfo.h>
#include <sys/time.h>

#include <stdbool.h>
#include <stdatomic.h>

#define PRGM_NAME   "pcsbench"
#define PRGM_VERS   "v0.1.0"
// TODO: define based on system values
#define PS_SHIFT    (12)
#define HPS_SHIFT   (21)
#define PAGE_SIZE   (1 << PS_SHIFT)
#define HPAGE_SIZE  (1 << HPS_SHIFT)
#define HP_ORDER    (HPS_SHIFT - PS_SHIFT)

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define perr()      perror(PRGM_NAME);

typedef void (*bench_fp)(struct sysinfo *si, uint64_t ct);

uint64_t cachedmem();
struct timeval *start_timer();
uint64_t end_timer(struct timeval *timer);

extern atomic_bool *completed;

#endif // COMMON_H_
