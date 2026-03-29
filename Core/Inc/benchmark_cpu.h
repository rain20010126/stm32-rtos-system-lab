#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>

// init benchmark system
void benchmark_cpu_init(void);

// calibrate idle baseline (run once)
void benchmark_calibrate_idle(void);

// print CPU usage
void benchmark_cpu_log(void);

// DWT cycle measurement
uint32_t benchmark_start(void);
uint32_t benchmark_end(uint32_t start);

// CPU usage getter
uint32_t benchmark_get_cpu_usage(void);

#endif