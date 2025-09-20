//===============================================================================
// Project Name : RTOS Task Basics
// Author       : Vivek Patel
// File         : Rtos_Blinky
// Date         : 13/09/2025
// Description  : Demonstrates RTOS fundamentals using 3 tasks.
//               Shows how primitive scheduling works compared to 
//               the traditional while(1) loop approach in bare-metal programming.
//================================================================================

//===============================================================================
#include <Arduino_FreeRTOS.h>
//===============================================================================
void setup() {

  // Initialize pins
  pinMode(LED_BUILTIN, OUTPUT);   // Onboard LED (pin 13)
  Serial.begin(9600);
  while (!Serial) ;               // Wait for Serial Monitor (important on Uno)

  // Create tasks
  // Create tasks
  xTaskCreate(TaskBlink, "Blink", 128, NULL, 1, NULL);
  xTaskCreate(TaskHighPrint, "HighPrint", 128, NULL, 3, NULL);  // Higher priority
  xTaskCreate(TaskLowPrint, "LowPrint", 128, NULL, 1, NULL);   // Lower priority

  // No code in setup() after this, scheduler takes control
}
//===============================================================================
void loop() {
  // Empty - RTOS tasks are running
}
//===============================================================================
// Task 1: Blink LED
void TaskBlink(void *pvParameters) {
  pinMode(LED_BUILTIN, OUTPUT);
  while (1) {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
//===============================================================================
// Task 2: High priority print
void TaskHighPrint(void *pvParameters) {
  while (1) {
    Serial.println(" HIGH priority task is running");
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
//===============================================================================
// Task 3: Low priority print
void TaskLowPrint(void *pvParameters) {
  while (1) {
    Serial.println("... low priority task is running");
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}
//===============================================================================