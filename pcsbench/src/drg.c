// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/sysinfo.h>
#include <sys/param.h>
#include <unistd.h>
#include <sys/prctl.h>

#include <time.h>

#include "common.h"
#include "drg.h"

#define P1   (0.1L)
#define P2   (0.9L)
#define Q1   (0.2L)
#define Q2   (0.04L)

#define P(Q) ((unsigned long long)(MIN(100.0L,MAX(0.0L,(((Q1*Q2*(P1-P2)/(Q))+(P2*Q2-P1*Q1))/(Q2-Q1))*100.0L))))

// TODO: for full paper, refactor this (and a lot of other things)
// that create unnecessarily deep blocks
void bench_drg(struct sysinfo *_si, uint64_t ct) {
    if (!ct) {
        int pid = fork();
        if (!pid) {
            prctl(PR_SET_NAME, "pcs-drg", 0, 0, 0);
            FILE *f = fopen("/proc/buddyinfo", "r");
            char buf[4096];
            const struct timespec dur = {0, 10'000'000};

            while (likely(!*completed)) {
                fseek(f, 0, SEEK_SET);
                const ssize_t n = fread(buf, sizeof(char), sizeof(buf)-1, f);
                buf[n] = '\0';
                char *line = strtok(buf, "\n");
                while (line) {
                    if (!unlikely(strncmp(line, "Node 0, zone   Normal", 21))) {
                        char *nums = line + 22;
                        char *end;
                        size_t fp = 0, hp = 0;
                        for (size_t i = 0; likely(i < 16 && nums); ++i) {
                            const size_t q = strtol(nums, &end, 10);
                            if (nums == end)
                                break;
                            const size_t pc = q << i;
                            fp += pc;
                            if (i >= HP_ORDER)
                                hp += pc;
                            nums = end;
                        }
                        FILE *fcp = fopen("/proc/sys/vm/compaction_proactiveness", "w");
                        if (unlikely(!f))
                            perr();
                        long double fpd = (long double)fp;
                        long double hpd = (long double)hp;
                        fprintf(fcp, "%llu", P(hpd/fpd));
                        fclose(fcp);
                        break;
                    }
                    line = strtok(NULL, "\n");
                }
                nanosleep(&dur, NULL);
            }
            exit(0);
        }
    }
}
