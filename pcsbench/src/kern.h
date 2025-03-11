// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef KERN_H_
#define KERN_H_

#include <stdint.h>
#include <sys/sysinfo.h>

void bench_kern(struct sysinfo *si, uint64_t ct);

#endif // KERN_H_
