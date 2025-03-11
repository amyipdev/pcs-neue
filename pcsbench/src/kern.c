// SPDX-License-Identifier: GPL-2.0-or-later

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/sysinfo.h>

#include <glib.h>

#include "common.h"
#include "kern.h"

inline uint64_t udt(struct sysinfo *si, uint64_t ct) {
    return si->freeram + si->bufferram + ct;
}

// TODO: factor this code out into a common function that just calls a method changer
void bench_kern(struct sysinfo *si, uint64_t ct) {
    uint64_t total_free = udt(si, ct);
    printf("Total free memory: %lu MiB\n", total_free >> 20);
    uint64_t apg = (total_free * 9 / 10) >> PS_SHIFT;
    uint64_t ptf = apg >> 3;
    uint64_t hpta = ptf >> 9;
    srand(time(NULL));
    GArray *ptrs = g_array_new(FALSE, FALSE, sizeof(void *));

    //for (size_t i = 0; i <= 90; i += 10) {
        // TODO: set compaction level
        // TODO: time each iteration
        //for (size_t j = 0; j < 10; ++j) {
            // allocate base
            for (size_t _j = 0; _j < apg; ++_j) {
                void *buf = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                assert(buf != NULL);
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
            ptrs = g_array_new(FALSE, FALSE, sizeof(void *));
            sleep(10);
        //}
    //}
}
