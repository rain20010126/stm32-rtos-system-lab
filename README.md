# STM32 RTOS Performance Lab

This project is a system-level performance analysis platform built on STM32 using FreeRTOS.  
It investigates how different synchronization mechanisms and system designs affect performance in an RTOS-based embedded system, and is designed to evolve toward Linux driver development.

---

## Objectives

The main goals of this project are:

- Compare busy-wait (polling) and event-driven (state machine + semaphore) designs  
- Analyze producer-consumer rate mismatch in RTOS systems  
- Build a benchmarking framework to measure throughput, latency, and CPU usage  
- Apply control methods (PI controller) to stabilize system behavior  
- Extend the system toward Linux device driver development  

---

## System Architecture (RTOS)
SensorTask (Producer) 
→ Message Queue 
→ LoggerTask (Consumer)


---

## Task Description

### SensorTask (Producer)

- Reads sensor data via I2C (BME680)  
- Attaches timestamp and pushes data into the queue  
- Controls data generation rate (via `osDelay` or dynamic adjustment)  

---

### LoggerTask (Consumer)

- Retrieves data from the queue  
- Measures throughput (msg/sec)  
- Measures end-to-end latency (enqueue → processing completion)  
- Simulates processing workload  

---

### BenchmarkTask

- Logs system metrics periodically:
  - Throughput  
  - Queue depth  
  - Drop rate  
  - Latency  
  - CPU usage (idle-based estimation)  

---

## Benchmark Metrics

The system measures:

- **Throughput** — messages processed per second  
- **Latency** — time from enqueue to processing completion  
- **Queue depth** — to observe backlog  
- **Drop rate** — queue overflow occurrences  
- **CPU usage** — estimated based on idle time  

---

## Experiment Design

This project compares two synchronization and I/O strategies:

---

### 1. Busy Wait (Polling-based)

- I2C operations use polling / busy wait  
- CPU continuously waits for I/O completion (no yielding)  

**Characteristics:**

- Simple implementation  
- High CPU usage  
- May block the system  

---

### 2. State Machine + Semaphore (Event-driven)

- I2C driver uses interrupt + state machine  
- Completion is signaled via semaphore  
- Task blocks while waiting, allowing CPU to run other tasks  

**Characteristics:**

- Non-blocking  
- More flexible scheduling  
- Higher implementation complexity  

---

## Experimental Observations

### Busy Wait

- CPU usage: high (near saturation)  
- Throughput: stable but limited by CPU  
- Latency: relatively high  
- Queue tends to accumulate backlog  

---

### State Machine + Semaphore

- CPU usage: similar to busy wait  
- Throughput: similar to busy wait  
- Latency: no significant improvement observed  

---

## Key Insights

### 1. Event-driven design does not always reduce CPU usage

In this system:

- I2C latency: on the order of tens of microseconds  
- RTOS scheduling granularity: approximately 1 ms  

Therefore:

> I/O completes much faster than task scheduling resolution

This results in minimal actual blocking, and the expected advantage of semaphore-based design is not fully realized.

---

### 2. The real bottleneck is rate mismatch

When:

- Producer rate exceeds consumer processing capability  

The system exhibits:

- Queue backlog  
- Increased latency  
- Rising drop rate  

Thus, system performance is primarily limited by:

> The mismatch between producer and consumer rates  

---

### 3. PI Controller stabilizes the system

To address queue buildup, a PI controller is introduced:

- Input: queue depth  
- Output: producer sampling delay  

By dynamically adjusting the data generation rate:

- Queue depth is stabilized  
- Drop rate is reduced  
- Latency is controlled  

This converts the system from passive load handling to active control.

---

### 4. System performance depends on overall design

This project demonstrates:

- Event-driven design is not always superior to busy wait  
- Synchronization effectiveness depends on workload and I/O latency  
- RTOS performance must be analyzed at the system level  

---

## Development Notes

Detailed experiments and analysis:

👉 https://hackmd.io/@10530417/ryWL9oLsZx  

---

## Future Work (Linux Driver)

The next stage of this project extends to Linux-based systems:

### 1. Linux Device Driver

- Implement I2C sensor driver in kernel space  
- Provide a device interface for user applications  
- Support standard driver operations (read / write / control)  

---

### 2. System Integration
User Space 
-> Linux Driver
-> I2C
-> Sensor


---

### 3. Extended Analysis

- Compare RTOS and Linux driver architectures  
- Analyze user space and kernel space interaction  
- Evaluate the impact of driver design on system performance  

---

## Summary

This project demonstrates:

- RTOS-based producer-consumer system design  
- Comparison of busy wait and event-driven approaches  
- System bottleneck identification and analysis  
- Modeling and solving rate mismatch problems  
- Applying control theory (PI controller) to system optimization  
- Extending from embedded RTOS to Linux driver development  