//=============================================================================
// RTOS Series – Week 11
// Memory Management Demo (ESP32)
// Static vs Dynamic Task Allocation
// Author: Vivek Patel
// Date: 17/01/2026
//=============================================================================

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ---------------- Config ----------------
#define SERIAL_BAUD 115200

// Stack sizes (in words, not bytes)
#define DYNAMIC_TASK_STACK_SIZE 2048
#define STATIC_TASK_STACK_SIZE  2048

// ---------------- Handles ----------------
TaskHandle_t dynamicTaskHandle = NULL;
TaskHandle_t staticTaskHandle  = NULL;

// ---------------- Static Allocation Buffers ----------------
// These are allocated at compile time (global memory)
StaticTask_t staticTaskTCB;
StackType_t  staticTaskStack[STATIC_TASK_STACK_SIZE];

// ---------------- Dynamic Task ----------------
void DynamicTask(void *pvParameters) {
  (void) pvParameters;

  Serial.printf("DynamicTask started on core %d\n", xPortGetCoreID());

  // Print stack high water mark
  UBaseType_t stackHW = uxTaskGetStackHighWaterMark(NULL);
  Serial.printf("DynamicTask stack high-water mark: %u words\n", stackHW);

  for (;;) {
    Serial.println("DynamicTask running...");
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// ---------------- Static Task ----------------
void StaticTask(void *pvParameters) {
  (void) pvParameters;

  Serial.printf("StaticTask started on core %d\n", xPortGetCoreID());

  // Print stack high water mark
  UBaseType_t stackHW = uxTaskGetStackHighWaterMark(NULL);
  Serial.printf("StaticTask stack high-water mark: %u words\n", stackHW);

  for (;;) {
    Serial.println("StaticTask running...");
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);

  Serial.println();
  Serial.println("===========================================");
  Serial.println("RTOS Week 11: Memory Management Demo");
  Serial.println("Static vs Dynamic Task Allocation");
  Serial.println("===========================================");
  delay(1000)

  // -------- Create Dynamic Task --------
  Serial.println("Creating DynamicTask using xTaskCreate()");
  xTaskCreatePinnedToCore(
    DynamicTask,
    "DynamicTask",
    DYNAMIC_TASK_STACK_SIZE,
    NULL,
    1,
    &dynamicTaskHandle,
    0
  );

  // -------- Create Static Task --------
  Serial.println("Creating StaticTask using xTaskCreateStatic()");
  staticTaskHandle = xTaskCreateStatic(
    StaticTask,
    "StaticTask",
    STATIC_TASK_STACK_SIZE,
    NULL,
    1,
    staticTaskStack,
    &staticTaskTCB
  );

  // Print static memory addresses
  Serial.printf("StaticTask stack address: 0x%08X\n", (uint32_t)staticTaskStack);
  Serial.printf("StaticTask TCB   address: 0x%08X\n", (uint32_t)&staticTaskTCB);
}

void loop() {
  // Nothing here — FreeRTOS tasks run independently
}
