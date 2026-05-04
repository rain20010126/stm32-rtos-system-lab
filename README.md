# STM32 RTOS Performance Lab

This project is a system-level performance analysis platform built on STM32 using FreeRTOS.  
It investigates how different synchronization mechanisms and system designs affect performance in an RTOS-based embedded system, and is designed to evolve toward Linux driver development.

---

## Objectives

The main goals of this project are:

- Compare busy-wait (polling) and event-driven (state machine + semaphore) I2C driver designs
- Design a centralized I2C Manager Task to serialize bus access and prevent multi-task race conditions
- Analyze queue behavior and producer-consumer balance in RTOS systems
- Build a benchmarking framework to measure throughput, sensor read latency, and CPU usage
- Extend the system toward Linux device driver development

---

## System Architecture (RTOS)

> Architecture diagram will be added later.

The benchmark uses a producer-consumer model. `SensorTask` reads BME680 data through I2C and pushes sensor samples into a message queue. `LoggerTask` consumes queued data and updates throughput statistics. `BenchmarkTask` periodically reports throughput, sensor read latency, queue depth, drop count, and CPU usage.

Runtime behavior can be controlled through the UART command interface. The project also introduces an I2C Manager Task design to centralize I2C bus access. Instead of allowing multiple tasks to access the I2C driver directly, I2C requests are serialized through a manager task, reducing race conditions and making the bus access pattern easier to reason about in an RTOS multi-task environment.

---

## Task Description

### SensorTask (Producer)

- Reads BME680 data through the I2C driver
- Attaches timestamp and pushes data into the queue  
- Controls data generation rate (via `osDelay` or dynamic adjustment)  

---

### LoggerTask (Consumer)

- Retrieves data from the queue  
- Updates throughput statistics  
- Supports optional synthetic workload for stress testing

---

### BenchmarkTask

- Logs system metrics periodically:
  - Throughput  
  - Queue depth  
  - Drop rate  
  - Sensor read latency  
  - CPU usage (idle-based estimation)  

---

### UART Command Interface

The system provides a UART-based command interface to switch runtime modes without recompiling the firmware.

Supported commands include:

| Command | Description |
|---|---|
| `temp` | Print temperature data periodically |
| `pressure` | Print pressure data periodically |
| `bench` | Enable benchmark-oriented logging |
| `silent` | Consume queue data without printing every message |

### I2C Manager Task

The project explores a centralized I2C Manager Task design to prevent race conditions when multiple tasks need to access the same I2C bus.

Instead of allowing application tasks to call the I2C driver directly, tasks send I2C requests to the manager through a request queue. The manager processes one request at a time, ensuring that I2C transactions are serialized.

Each request may include a completion semaphore, allowing the caller to block until its transaction is completed without busy-waiting.

This design provides:

- Centralized I2C bus ownership
- Serialized access to avoid bus-level race conditions
- Clear request/response synchronization using per-request semaphores
- Better scalability when more sensor or application tasks are added

## Benchmark Metrics

The system measures:

- **Throughput** — messages processed per second  
- **Sensor read latency** — wall time of `sensor_read()`, measured using the DWT cycle counter 
- **Queue depth** — to observe backlog  
- **Drop rate** — queue overflow occurrences  
- **CPU usage** — estimated based on idle time  

---

## Experiment Branches

The driver comparison experiments are maintained in separate branches:

- Polling implementation: [`exp/i2c-polling`](https://github.com/rain20010126/stm32-rtos-system-lab/tree/exp/i2c-polling)
- FSM + Semaphore implementation: [`exp/i2c-state-machine-semaphore`](https://github.com/rain20010126/stm32-rtos-system-lab/tree/exp/i2c-state-machine-semaphore)

These branches are used to compare two I2C driver implementations under the same RTOS benchmark setup.
Detailed experiment logs and analysis are documented in the HackMD note.

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

- Avoids busy-waiting in the calling task
- Uses semaphore-based blocking wait for I/O completion
- Allows CPU to execute other tasks or enter idle during I/O waiting
- More flexible scheduling
- Higher implementation complexity

---

## Experimental Observations

The experimental results show that the observed performance depends heavily on the producer rate.

### Low producer rate

When the SensorTask delay is relatively long, the throughput is mainly limited by the task delay rather than the I2C driver implementation. Under this condition, polling and FSM + Semaphore show similar throughput and CPU usage.

### High producer rate

When the SensorTask delay is very short, the SensorTask issues I2C transactions almost back-to-back.

In this condition:

- Polling can achieve higher raw throughput
- Polling consumes nearly all CPU time due to busy-waiting
- FSM + Semaphore avoids continuous busy-waiting by blocking the task while the I2C transaction is handled by interrupts
- FSM + Semaphore is more suitable for RTOS multi-task systems where CPU availability and task responsiveness are important

## Note on Previous Experimental Results

Earlier experiment results were treated as development-stage observations because the experimental conditions were not fully controlled across branches.

For the final comparison, the polling and FSM + Semaphore branches were re-tested with aligned benchmark conditions, including queue configuration, workload setting, SensorTask delay settings, and latency measurement method.

Therefore, the README summarizes the controlled comparison, while detailed logs and historical observations are documented in the HackMD note.

## Key Insights

### 1. FSM + Semaphore is not mainly for higher raw throughput

The FSM + Semaphore implementation does not always produce higher raw throughput than polling.

Polling can be faster in a single-task benchmark because it continuously checks I2C status flags and immediately continues once the hardware is ready. However, this also keeps the CPU busy during the waiting period.

FSM + Semaphore trades some raw throughput for better RTOS behavior. The task can block while waiting for I2C completion, allowing the CPU to execute other tasks or enter idle.

---

### 2. I2C Manager Task prevents bus-level race conditions

In an RTOS system, multiple tasks may need to access the same I2C bus. If each task calls the I2C driver directly, transactions may overlap or interfere with each other.

The I2C Manager Task centralizes bus ownership by receiving I2C requests through a queue and processing them one at a time. This serializes I2C access and reduces race conditions at the bus and driver level.

Using a per-request semaphore also allows the caller to wait for completion without busy-waiting.

---

### 3. Queue behavior verifies producer-consumer balance

Queue depth and drop count are used to verify whether the consumer can keep up with the producer.

In the controlled driver comparison, no message drops were observed, and the maximum queue depth remained low. This indicates that the benchmark mainly reflects I2C driver behavior and CPU usage rather than queue overload.

If the producer rate exceeds the consumer processing capability in a heavier workload, the system may show queue backlog, increased latency, or message drops.

---

### 4. System performance depends on overall design

This project demonstrates:

- Event-driven design is not always superior in raw throughput
- Driver design should be evaluated together with CPU availability and task responsiveness
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
- Centralized I2C Manager Task design for bus serialization and race-condition prevention
- Comparison of polling and interrupt-driven FSM + Semaphore I2C driver approaches
- Benchmarking of throughput, sensor read latency, queue behavior, drop count, and CPU usage
- System-level analysis of driver design, CPU availability, and task responsiveness
- Extending from embedded RTOS toward Linux driver development