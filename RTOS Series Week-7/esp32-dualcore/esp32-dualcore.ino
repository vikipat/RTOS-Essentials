//===============================================================================
// Project Name : RTOS Essentials Learning Series
// Author       : Vivek Patel
// Date         : 09/11/2025
// Description  : Dual-Core Scheduling Demonstration (ESP32)
//                Demonstrates true parallel execution:
//                - Core 0: LED blink task
//                - Core 1: CPU-intensive compute task
//===============================================================================

#include <Arduino.h>

// Define LED pin (built-in LED on ESP32 Dev boards is usually GPIO 2)
#define LED_PIN 2

TaskHandle_t TaskLED_Handle;
TaskHandle_t TaskCompute_Handle;

//--------------------------------------------------------------------
// Task 1 - LED blink (Core 0)
//--------------------------------------------------------------------
void TaskLED(void *pvParameters) {
  pinMode(LED_PIN, OUTPUT);

  for (;;) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // toggle LED
    Serial.print("💡 LED toggled on Core ");
    Serial.println(xPortGetCoreID());
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

//--------------------------------------------------------------------
// Task 2 - CPU Intensive work (Core 1)
//--------------------------------------------------------------------
void TaskCompute(void *pvParameters) {
  uint32_t n1 = 0, n2 = 1, n3;

  for (;;) {
    // Compute a small Fibonacci sequence (simulate heavy work)
    for (int i = 0; i < 10; i++) {
      n3 = n1 + n2;
      n1 = n2;
      n2 = n3;
    }

    Serial.print("⚙️ Compute Task running on Core ");
    Serial.print(xPortGetCoreID());
    Serial.print(" | Fibonacci value: ");
    Serial.println(n3);

    vTaskDelay(700 / portTICK_PERIOD_MS);
  }
}

//--------------------------------------------------------------------
// Setup
//--------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("==================================================");
  Serial.println("RTOS Series – Week 7: Dual-Core Scheduling Demo");
  Serial.println("==================================================");

  Serial.printf("ESP32 FreeRTOS running on %d cores\n\n", portNUM_PROCESSORS);

  // Create LED task pinned to Core 0
  xTaskCreatePinnedToCore(
    TaskLED,             // Function
    "TaskLED",           // Name
    2048,                // Stack size
    NULL,                // Parameters
    1,                   // Priority
    &TaskLED_Handle,     // Task handle
    0                    // Core 0
  );

  // Create Compute task pinned to Core 1
  xTaskCreatePinnedToCore(
    TaskCompute,         // Function
    "TaskCompute",       // Name
    4096,                // Stack size (bigger for computation)
    NULL,                // Parameters
    1,                   // Priority
    &TaskCompute_Handle, // Task handle
    1                    // Core 1
  );
}

void loop() {
  // Empty - tasks are managed by FreeRTOS scheduler
}
