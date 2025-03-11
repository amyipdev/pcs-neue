// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

#include "common.h"

#define PROC_BUFFER_SIZE 255
#define CACHED_LITERAL   "Cached:"

uint64_t cachedmem() {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) {
        perr();
        return 0;
    }

    char line[PROC_BUFFER_SIZE + 1];
    line[PROC_BUFFER_SIZE] = 0;
    uint64_t val = 0;
    while (fgets(line, PROC_BUFFER_SIZE, f)) {
        if (strncmp(line, CACHED_LITERAL, strlen(CACHED_LITERAL)) == 0) {
            sscanf(line + strlen(CACHED_LITERAL), " %lu ", &val);
            break;
        }
    }
    fclose(f);
    return val << 10;
}

struct timeval *start_timer() {
    struct timeval *tv = (struct timeval *)malloc(sizeof(struct timeval));
    if (!tv)
        perr();
    gettimeofday(tv, NULL);
    return tv;
}

uint64_t end_timer(struct timeval *timer) {
    if (!timer)
        perr();
    struct timeval *tv = (struct timeval *)malloc(sizeof(struct timeval));
    if (!tv) {
        perr();
    }
    gettimeofday(tv, NULL);
    uint64_t ems = (uint64_t)(tv->tv_sec - timer->tv_sec) * 1000000LL + (uint64_t)(tv->tv_usec - timer->tv_usec);
    free(tv);
    return ems;
}
