#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

#include <sys/sysinfo.h>
#include <sys/time.h>

#define PRGM_NAME   "pcsbench"
#define PRGM_VERS   "v0.1.0"
#define PS_SHIFT    (12)
#define HPS_SHIFT   (21)
#define PAGE_SIZE   (1 << PS_SHIFT)
#define HPAGE_SIZE  (1 << HPS_SHIFT)

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define perr()      perror(PRGM_NAME);

typedef void (*bench_fp)(struct sysinfo *si, uint64_t ct);

uint64_t cachedmem();
struct timeval *start_timer();
uint64_t end_timer(struct timeval *timer);

#endif // COMMON_H_
