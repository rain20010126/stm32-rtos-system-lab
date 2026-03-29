# STM32 RTOS Benchmark

This project implements a performance benchmark framework on STM32 using FreeRTOS.

## Architecture

SensorTask (Producer) → Queue → LoggerTask (Consumer)

## Metrics

- Throughput (msg/sec)
- Latency (enqueue → dequeue)
- Queue depth
- Drop rate
- CPU usage (idle-based)

## Baseline (baseline-v1)

Configuration:
- Producer rate: 100 Hz
- Consumer: intentionally slowed down
- Queue size: 10

Observed:
- Queue saturation
- High latency (~500 ms)
- Continuous data drops
- CPU usage: 60–100%

This baseline is used for further optimization experiments.
