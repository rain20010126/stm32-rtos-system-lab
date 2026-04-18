#include "benchmark_sys.h"
#include <stdio.h>
#include "cmsis_os2.h"

#define CYCLES_TO_US(cycles) ((cycles) * 1000000UL / CPU_FREQ)

static uint32_t max_queue_depth = 0;
static uint32_t queue_drop_count = 0;

static uint32_t queue_drop_last = 0; 

static uint32_t max_latency = 0;

static uint32_t msg_count = 0;
static uint32_t last_tick = 0;

// additiona; lattency
static uint32_t sensor_latency_max = 0;
static uint32_t i2c_latency_max = 0;

void benchmark_sys_init(void)
{
    max_queue_depth = 0;
    queue_drop_count = 0;
    queue_drop_last = 0; 
    max_latency = 0;
    msg_count = 0;
    last_tick = osKernelGetTickCount();
}

void benchmark_queue_update(uint32_t depth)
{
    if (depth > max_queue_depth)
        max_queue_depth = depth;
}

void benchmark_queue_drop(void)
{
    queue_drop_count++;
}

void benchmark_latency_record(uint32_t latency)
{
    if (latency > max_latency)
        max_latency = latency;
}

void benchmark_throughput_inc(void)
{
    msg_count++;
}

void benchmark_sensor_latency_record(uint32_t cycles)
{
    if (cycles > sensor_latency_max)
        sensor_latency_max = cycles;
}

void benchmark_i2c_latency_record(uint32_t cycles)
{
    if (cycles > i2c_latency_max)
        i2c_latency_max = cycles;
    // printf("cycles = %lu\n", i2c_latency_max);
}

void benchmark_sys_log(void)
{
    uint32_t now = osKernelGetTickCount();

    if (now - last_tick >= 1000)
    {           
        // drop per second
        uint32_t drop_per_sec = queue_drop_count - queue_drop_last;
        queue_drop_last = queue_drop_count;

        printf("[SYS] tp=%lu | maxQ=%lu | drop=%lu (%lu/s) | sensor=%lu us\r\n",
                msg_count,
                max_queue_depth,
                queue_drop_count,
                drop_per_sec,
                CYCLES_TO_US(sensor_latency_max));
    
        
        msg_count = 0;
        // additional latency reset
        sensor_latency_max = 0;
        i2c_latency_max = 0;
    }
}