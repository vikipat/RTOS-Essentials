# RTOS-Essentials

## Updates

### Week 1 – 13/09/2025  
feat: Add RTOS task demos for bare-metal vs RTOS comparison  
- Implemented 3 concurrent tasks (LED blink, high/low priority prints)  
- Demonstrates FreeRTOS priority-based scheduling  
- Complete Arduino Uno setup with detailed comments  

### Week 2 – 20/09/2025  
feat: Add delay handling demo (blocking vs non-blocking)  
- Bare-metal version: `delay()` blocks all tasks, halts system concurrency  
- FreeRTOS version: `vTaskDelay()` allows true multitasking  
- Demonstrates LED toggling every 5s + continuous serial prints every 500ms  
- Clear timestamps in output show difference in task scheduling  

### Week 3 – 27/09/2025  
feat: Add task communication demo (global variable vs FreeRTOS queue)  
- **Global Variable Demo:** Increment + Reset tasks overwrite each other, Print task sees inconsistent values (race condition risk)  
- **Queue Demo:** Increment + Reset tasks push data safely into a queue, Print task retrieves values in FIFO order without corruption  
- Demonstrates how FreeRTOS queues solve the limitations of globals by providing atomic, ordered, and synchronized task communication  
- Clear serial logs highlight the contrast: globals cause overwrites, queues guarantee message delivery  


### Week 4–5 – 11/10/2025  
feat: Add task synchronization and software timer demo  

- **Semaphore Demo:**  
  Two tasks safely share the Serial port using a **binary semaphore**.  
  Demonstrates how FreeRTOS ensures mutual exclusion and avoids data overlap when multiple tasks access the same shared resource.  

- **Software Timer Demo:**  
  Introduces FreeRTOS **software timers** to perform periodic actions without blocking any task.  
  A timer toggles an LED every 1 second, showing how timers offload periodic tasks from the main scheduler.  

- **Combined Concept:**  
  Both features are integrated into a single example while the timer toggles an LED, two concurrent tasks print messages using semaphore protection.  
  This illustrates how synchronization (🔒) and periodic events (⏱️) can coexist smoothly in a multitasking RTOS system.  

- Clear serial logs and LED toggle behavior highlight:  
  • Safe resource access via semaphores  
  • Non-blocking periodic actions via software timers  
  • Deterministic and modular task management under FreeRTOS  


### Week 6 – 18/10/2025  
feat: Add task priorities and preemption demo  

- **Concept Overview:**  
  Demonstrates how FreeRTOS schedules multiple tasks based on their **priority levels**.  
  A higher-priority task always preempts (interrupts) a lower-priority one whenever it becomes ready to run.  
  This behavior is at the heart of FreeRTOS’s real-time deterministic scheduling.  

- **Demo (Arduino Uno):**  
  • **Low Priority Task (Priority 1):** Simulates heavy CPU work to occupy processor time.  
  • **High Priority Task (Priority 2):** Periodically wakes up and interrupts the low-priority task to run first.  
  • Both tasks print messages on the Serial Monitor, making preemption clearly visible.  

- **Key Observations:**  
  • High-priority task takes control whenever it becomes ready, regardless of what the low-priority task is doing.  
  • Scheduler automatically handles context switching between tasks.  
  • Serial output shows the low task’s “heavy work” being interrupted mid-cycle by the high-priority task.  

- **Takeaway:**  
  FreeRTOS preemptive scheduling ensures that time-critical tasks always get CPU access when needed.  
  Task priorities are a powerful mechanism to control system responsiveness and deterministic behavior in embedded systems.  


### Week 7 – 16/11/2025  
feat: Add ESP32 dual-core scheduling demo  

- **Concept Overview:**  
  ESP32 runs FreeRTOS natively on **two cores** (Core 0 and Core 1).  
  This demo shows how tasks can be explicitly assigned to a specific core using  
  `xTaskCreatePinnedToCore()`, enabling true parallel execution.

- **Demo (ESP32):**  
  • **TaskLED (Core 0):** Toggles the onboard LED every 500ms  
  • **TaskCompute (Core 1):** Performs Fibonacci calculations and prints results  
  • Both tasks run independently on separate CPU cores

- **Key Observations:**  
  • LED blinks smoothly on Core 0 while heavy computations run on Core 1  
  • Serial output shows each task’s core using `xPortGetCoreID()`  
  • Demonstrates *real parallelism*, not just time-sliced multitasking  
  • Validates dual-core FreeRTOS scheduling and core affinity on ESP32

- **Takeaway:**  
  ESP32 provides a powerful upgrade for RTOS development — dual-core scheduling  
  allows CPU-intensive tasks and hardware I/O to run seamlessly in parallel,  
  significantly improving responsiveness and system throughput.
  
  
### Week 8 – 22/11/2025  
feat: Add Advanced Queues + Event Groups demo (ESP32)

- **Concept Overview:**  
  This week combines two essential RTOS primitives: **advanced queues** (structured data passing) and **event groups** (multi-bit signaling). The pattern solves a common real-world need: *data readiness* (queue) + *system readiness* (events).

- **Demo (ESP32):**  
  • **TaskSensor (Core 0):** Reads DHT22 (or simulates values), populates `SensorData_t` structs and pushes them to a queue. Sets `BIT_SENSOR_READY`.  
  • **TaskWiFi (Core 1):** Connects to Wi-Fi and sets `BIT_WIFI_READY` when connected.  
  • **TaskFusion (Core 1):** Waits for **BIT_SENSOR_READY + BIT_WIFI_READY** using an Event Group, then drains/processes the sensor queue and logs formatted entries.  
  • Optional simulation fallback included so demo runs without real sensor.

- **Key Observations:**  
  ✅ Queues safely pass structured sensor data between tasks/cores (no globals).  
  ✅ Event Groups allow compact AND/OR synchronization for multi-source readiness.  
  ✅ Combining queues + events creates a robust startup/processing pipeline: *send data* + *signal readiness* → *process*.  
  ✅ Multi-core pins show inter-core communication and real parallel operation on ESP32.

- **Takeaway:**  
  Use **queues** when you need to move data reliably. Use **event groups** when you need to coordinate multiple readiness signals. Together they form a production-ready RTOS pattern used in IoT, robotics, and embedded systems.

### Week 9 – 29/11/2025  
feat: Add Mutex vs Binary Semaphore demo (ESP32)

- **Concept Overview:**  
  This week focuses on **shared resource protection** in FreeRTOS by comparing **Binary Semaphores** and **Mutexes**, with emphasis on **priority inversion** and **priority inheritance** in priority-based schedulers.

- **Demo (ESP32):**  
  • **LowTask (Low Priority):** Acquires a shared resource and performs work while holding it.  
  • **MediumTask (Medium Priority):** Runs periodically to create preemption pressure.  
  • **HighTask (High Priority):** Attempts to acquire the same resource and may block.  
  • The shared resource is protected using either:
    - **Binary Semaphore** (no priority inheritance), or  
    - **Mutex** (supports priority inheritance).  
  • Runtime diagnostics log task priorities using `uxTaskPriorityGet()`.

- **Key Observations:**  
  ✅ Binary semaphores are suitable for signaling but **not** for protecting shared resources.  
  ✅ Mutexes are designed for resource protection and support **priority inheritance**.  
  ✅ Priority inheritance activates only when a high-priority task blocks on a mutex held by a lower-priority task.  
  ✅ On ESP32 (dual-core, high-speed CPU), priority inheritance may not always be visually obvious due to short critical sections and scheduling effects.

- **Takeaway:**  
  Use **mutexes** whenever multiple tasks share a resource in a priority-based system.  
  Use **binary semaphores** strictly for signaling events.  
  Understanding priority inversion and inheritance is essential for building reliable, real-world RTOS applications.

