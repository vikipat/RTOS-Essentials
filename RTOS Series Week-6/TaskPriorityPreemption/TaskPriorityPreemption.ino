//===============================================================================
// Project Name : RTOS Essentials Learning Series
// Author       : Vivek Patel
// Date         : 18/10/2025
// Description  : Task Priorities and Preemption Demonstration (Enhanced)
//                This version makes preemption visibly clear by simulating
//                heavy work in the low-priority task.
//===============================================================================
#include <Arduino_FreeRTOS.h>
//======================Function Prototypes======================================
void vHighPriorityTask(void *pvParameters);
void vLowPriorityTask(void *pvParameters);
//==============================Setup============================================
void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Serial.println("RTOS Week 6: Task Priorities & Preemption (Enhanced)");
  Serial.println("==============================================================");

  // Low priority = 1
  xTaskCreate(vLowPriorityTask, "LowTask", 128, NULL, 1, NULL);

  // High priority = 2
  xTaskCreate(vHighPriorityTask, "HighTask", 128, NULL, 2, NULL);

  vTaskStartScheduler();
}
//================================================================================
void loop() {
  // Not used when running FreeRTOS
}
//=================================Tasks==========================================
// High Priority Task - prints periodically
void vHighPriorityTask(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    Serial.println("🔺 High Priority Task INTERRUPT!");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
//===============================================================================
// Low Priority Task - simulates heavy processing
void vLowPriorityTask(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    Serial.println("⬇️ Low Priority Task starting heavy work...");
    // Simulate CPU-intensive work (~300ms loop)
    for (volatile uint32_t i = 0; i < 600000; i++) {
      // dummy math to consume time
      uint8_t dummy = i % 10;
      (void)dummy;
    }
    Serial.println("⬇️ Low Priority Task finished cycle.");
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
//================================================================================