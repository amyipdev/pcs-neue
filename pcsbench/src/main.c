// SPDX-License-Identifier: GPL-2.0-or-later

#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <linux/version.h>

#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>

#include <argp.h>
#include <glib.h>

#include "common.h"
#include "drg.h"
#include "kern.h"

#ifndef __linux__
#error PCSBench must be built for Linux.
#endif

#ifndef LINUX_VERSION_CODE
#warning Cannot check Linux version. Proactive compaction may not be supported.
#elifndef KERNEL_VERSION
#warning Compiler does not support Linux kernel version checking. Proactive compaction may not be supported.
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,9,0)
#warning Your kernel version does not support proactive compaction. PCSBench will fail.
#endif
#endif

// BEGIN ARGP

const char *argp_program_version = PRGM_NAME " " PRGM_VERS;
const char *argp_program_bug_address = "<amy@amyip.net>";
const char doc[] = "test Linux proactive memory compaction";
const char args_doc[] = "";

static struct argp_option options[] = {
    {"bench", 128, "num", OPTION_ARG_OPTIONAL, "Select a proactive compaction model"},
    {0}
};

enum Benchmarks { BENCH_DEFAULT = 0, BENCH_DRG = 1 };
const char *BENCH_NAMES[] = {
    "kernel-default",
    "drg-unnamed"
};

struct arguments {
    enum Benchmarks bench;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
        case 128:
            if (!strcmp("drg", arg))
                arguments->bench = BENCH_DRG;
            else
                arguments->bench = BENCH_DEFAULT;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

// END ARGP

bench_fp BENCHES[] = {
    &bench_kern,
    &bench_drg
};

int main(int argc, char **argv) {
    struct sysinfo si;
    struct arguments args = { .bench = BENCH_DEFAULT };

    argp_parse(&argp, argc, argv, 0, 0, &args);

    if (sysinfo(&si))
        perr();

    printf("%s\n", argp_program_version);
    printf("Executing benchmark %s\n", BENCH_NAMES[args.bench]);

    int _wpid, _st;
    uint64_t total_free = si.freeram + si.bufferram + cachedmem();
    printf("Total free memory: %lu MiB\n", total_free >> 20);
    uint64_t apg = (total_free * 9 / 10) >> PS_SHIFT;
    uint64_t ptf = apg >> 3;
    uint64_t hpta = ptf >> 9;
    srand(time(NULL));
    GArray *ptrs = g_array_new(FALSE, FALSE, sizeof(void *));
    for (size_t i = 0; i < 10; ++i) {
        // Change compaction level
        for (size_t j = 0; j < 10; ++j) {
            int pid = fork();
            if (pid == 0) {
                for (size_t _j = 0; _j < apg; ++_j) {
                    void *buf = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                    if (!buf)
                        perr();
                    g_array_append_vals(ptrs, &buf, 1);
                    memset(buf, 0x7a, PAGE_SIZE);
                }
                // shuffle
                for (size_t _k = ptrs->len - 1; _k > 0; --_k) {
                    size_t l = rand() % (_k + 1);
                    void *tmp = g_array_index(ptrs, void *, _k);
                    g_array_index(ptrs, gpointer, _k) = g_array_index(ptrs, gpointer, l);
                    g_array_index(ptrs, gpointer, l) = tmp;
                }
                // drop ptf pages
                size_t edrop = ptrs->len - ptf;
                for (size_t _k = ptrs->len - 1; _k > edrop; --_k) {
                    void *b = g_array_index(ptrs, void *, _k);
                    munmap(b, PAGE_SIZE);
                    g_array_remove_index(ptrs, _k);
                }
                // time reallocation as hugepages
                struct timeval *t0 = start_timer();
                for (size_t _k = 0; _k < hpta; ++_k) {
                    void *buf = mmap(NULL, HPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                    madvise(buf, HPAGE_SIZE, MADV_HUGEPAGE);
                    g_array_append_vals(ptrs, &buf, 1);
                    memset(buf, 0x83, HPAGE_SIZE);
                }
                printf("iter: %lu us\n", end_timer(t0));
                fflush(stdout);
                free(t0);
                // free everything and we start over
                // need to do the HPs first
                printf("a");
                fflush(stdout);
                size_t hdrop = ptrs->len - hpta;
                for (size_t _k = ptrs->len - 1; _k > hdrop; --_k) {
                    void *b = g_array_index(ptrs, void *, _k);
                    madvise(b, HPAGE_SIZE, MADV_NOHUGEPAGE);
                    munmap(b, HPAGE_SIZE);
                    g_array_remove_index(ptrs, _k);
                }
                printf("b");
                fflush(stdout);
                for (ssize_t _k = ptrs->len - 1; _k >= 0; --_k) {
                    void *b = g_array_index(ptrs, void *, _k);
                    munmap(b, PAGE_SIZE);
                    g_array_remove_index(ptrs, _k);
                }
                printf("c");
                fflush(stdout);
                // TODO: configurable sleep
                g_array_free(ptrs, TRUE);
                return 0;
            } else {
                while ((_wpid = wait(&_st)) > 0)
                    sleep(1);
            }
        }
    }

    return 0;
}
