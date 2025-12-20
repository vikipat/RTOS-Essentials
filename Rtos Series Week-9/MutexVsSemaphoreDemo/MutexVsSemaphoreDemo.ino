//===============================================================================
// RTOS Series - Week 9
// Mutex vs Binary Semaphore Demo (ESP32 - Arduino)
// Author: Vivek Patel
// Date: 20-12-2025
//===============================================================================
//
// - Demonstrates priority inversion and how Mutex (with priority inheritance)
//   solves it while a Binary Semaphore does not.
// - LowTask acquires lock and holds it (simulated long critical section).
// - MediumTask runs CPU-ish periodic work to preempt LowTask if possible.
// - HighTask tries to acquire lock and blocks until released.
// - Switch USE_MUTEX to 0 (binary semaphore) or 1 (mutex) to compare behaviours.
//===============================================================================

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// ---------------- CONFIG ----------------
// 1 = use Mutex (with priority inheritance). 0 = use Binary Semaphore (no PI).
#define USE_MUTEX 1

// delay values (ms)
#define LOW_HOLD_TIME_MS    3000   // LowTask holds the lock for 3s (simulate long critical)
#define HIGH_PERIOD_MS      2000   // HighTask wakes every 2s and tries to get the lock
#define MEDIUM_PERIOD_MS     200   // MediumTask runs often (preemption pressure)
#define LOW_LOOP_PERIOD_MS   500   // LowTask loops between critical sections

// priorities (FreeRTOS higher number = higher priority)
// We'll show: Low = 1, Medium = 2, High = 3
#define PRIO_LOW     1
#define PRIO_MEDIUM  2
#define PRIO_HIGH    3

// Stack sizes
#define STACK_SMALL  3072

// ---------------- Globals ----------------
SemaphoreHandle_t lockHandle = NULL; // mutex or binary semaphore

TaskHandle_t lowHandle = NULL;
TaskHandle_t medHandle = NULL;
TaskHandle_t highHandle = NULL;

// Utility for timestamp
static inline uint32_t now() { 
  return (uint32_t) millis(); 
}

// ---------------- Tasks ----------------

// Low priority task: obtains lock and HOLD it (simulate long critical section)
void LowTask(void *pv) {
  (void) pv;
  Serial.printf("[%04u] LowTask started on core %d\n", millis(), xPortGetCoreID());
  for (;;) {
    Serial.printf("[%04u] LowTask: attempt to TAKE lock\n", millis());
    if (xSemaphoreTake(lockHandle, portMAX_DELAY) == pdTRUE) {
      // print priority immediately after acquire
      UBaseType_t prio_after_acquire = uxTaskPriorityGet(NULL);
      Serial.printf("[%04u] LowTask: ACQUIRED lock -> priority=%u; entering BUSY critical section\n",
                    millis(), (unsigned)prio_after_acquire);

      // Busy-wait simulation (CPU-bound) so Low remains runnable (PI can help)
      const uint32_t busyLoops = 350000UL; // tune to approximate 2.5-3s wall time
      for (volatile uint32_t i = 0; i < busyLoops; ++i) {
        volatile uint32_t tmp = i * 17;
        (void)tmp;
      }

      // print priority right before release to observe any PI boost
      UBaseType_t prio_before_release = uxTaskPriorityGet(NULL);
      Serial.printf("[%04u] LowTask: finished BUSY work -> priority=%u; RELEASING lock\n",
                    millis(), (unsigned)prio_before_release);

      xSemaphoreGive(lockHandle);
    } else {
      Serial.printf("[%04u] LowTask: failed to take lock\n", millis());
    }

    // small sleep outside critical region
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}


// Medium priority task: does frequent work (creates preemption opportunity)
void MediumTask(void *pv) {
  (void) pv;
  Serial.printf("[%.4u] MediumTask started on core %d\n", now(), xPortGetCoreID());
  for (;;) {
    // Work simulation: print and short busy loop
    Serial.printf("[%.4u] MediumTask: running (priority %d)\n", now(), PRIO_MEDIUM);
    // Simulate small CPU work (non-blocking)
    for (volatile uint32_t i = 0; i < 20000; ++i) { volatile uint32_t x = i * 2; (void)x; }
    vTaskDelay(pdMS_TO_TICKS(MEDIUM_PERIOD_MS));
  }
}

void HighTask(void *pvParameters) {
  (void) pvParameters;

  Serial.printf("[%05u] HighTask started on core %d\n", millis(), xPortGetCoreID());

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(HIGH_PERIOD_MS));

    Serial.printf("[%05u] HighTask: attempting to TAKE lock\n", millis());

    // 🔍 Priority BEFORE blocking
    if (lowHandle != NULL) {
      Serial.printf("[%05u] (DBG-BEFORE) LowTask priority = %u\n",
                    millis(),
                    (unsigned)uxTaskPriorityGet(lowHandle));
    }

    uint32_t tStart = millis();

    // High blocks here if Low owns the mutex
    xSemaphoreTake(lockHandle, portMAX_DELAY);

    uint32_t tGot = millis();

    // 🔍 Priority AFTER High was blocked (PI should be applied now)
    if (lowHandle != NULL) {
      Serial.printf("[%05u] (DBG-AFTER) LowTask priority = %u\n",
                    millis(),
                    (unsigned)uxTaskPriorityGet(lowHandle));
    }

    Serial.printf("[%05u] HighTask: ACQUIRED lock after %u ms wait\n",
                  millis(),
                  (unsigned)(tGot - tStart));

    xSemaphoreGive(lockHandle);
  }
}


// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("===========================================");
  Serial.println("RTOS Week 9: Mutex vs Binary Semaphore Demo");
  Serial.println("===========================================");
  Serial.printf("Mode: %s\n", (USE_MUTEX ? "MUTEX (with PI)" : "BINARY SEMAPHORE (no PI)"));
  Serial.printf("ESP32 cores: %d\n", portNUM_PROCESSORS);

  // Create lock: either mutex or binary semaphore
  #if USE_MUTEX
    lockHandle = xSemaphoreCreateMutex();
    if (lockHandle == NULL) {
      Serial.println("Failed to create mutex");
      while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
  #else
    // Binary semaphore used as a lock (give it initially so first take succeeds)
    lockHandle = xSemaphoreCreateBinary();
    if (lockHandle == NULL) {
      Serial.println("Failed to create binary semaphore");
      while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    xSemaphoreGive(lockHandle); // make it available
  #endif

  // Create tasks pinned to cores to make scheduling clear
  // LowTask on core 0, Medium on core 1, High on core 1 (or mix as you like)
  BaseType_t r;
  r = xTaskCreatePinnedToCore(LowTask, "LowTask", STACK_SMALL, NULL, PRIO_LOW, &lowHandle, 0);
  if (r != pdPASS) Serial.println("Failed to create LowTask");

  r = xTaskCreatePinnedToCore(MediumTask, "MediumTask", STACK_SMALL, NULL, PRIO_MEDIUM, &medHandle, 1);
  if (r != pdPASS) Serial.println("Failed to create MediumTask");

  r = xTaskCreatePinnedToCore(HighTask, "HighTask", STACK_SMALL, NULL, PRIO_HIGH, &highHandle, 1);
  if (r != pdPASS) Serial.println("Failed to create HighTask");
}

void loop() {
  // not used
}
