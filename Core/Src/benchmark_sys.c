#include "benchmark_sys.h"
#include <stdio.h>
#include "cmsis_os2.h"

static uint32_t max_queue_depth = 0;
static uint32_t queue_drop_count = 0;

static uint32_t queue_drop_last = 0; 

static uint32_t max_latency = 0;

static uint32_t msg_count = 0;
static uint32_t last_tick = 0;

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

void benchmark_sys_log(void)
{
    uint32_t now = osKernelGetTickCount();

    if (now - last_tick >= 1000)
    {   
        uint32_t latency_us = max_latency / (CPU_FREQ / 1000000);
        
        // drop per second
        uint32_t drop_per_sec = queue_drop_count - queue_drop_last;
        queue_drop_last = queue_drop_count;

        printf("[SYS] tp=%lu | maxQ=%lu | drop=%lu (%lu/s) | maxLat=%lu us\r\n",
               msg_count,
               max_queue_depth,
               queue_drop_count,
               drop_per_sec,
               latency_us);

        msg_count = 0;
        last_tick = now;
        max_latency = 0; 
        max_queue_depth = 0;
    }
}