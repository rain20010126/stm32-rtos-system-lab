#ifndef BENCHMARK_SYS_H
#define BENCHMARK_SYS_H

#define CPU_FREQ 180000000

#include <stdint.h>

void benchmark_sys_init(void);

// queue
void benchmark_queue_update(uint32_t depth);
void benchmark_queue_drop(void);

// latency
void benchmark_latency_record(uint32_t latency);

// throughput
void benchmark_throughput_inc(void);

// print all system stats
void benchmark_sys_log(void);

#endif