//===============================================================================
// Project Name : RTOS Delay Demo
// Author       : Vivek Patel
// Date         : 20/09/2025
// Description  : Demonstration of delay() vs vTaskDelay() in FreeRTOS
//===============================================================================
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

// Task prototypes
void TaskBlink(void *pvParameters);
void TaskPrint(void *pvParameters);
//===============================================================================
void setup() {
  Serial.begin(9600);

  // Create two tasks
  xTaskCreate(TaskBlink, "Blink", 128, NULL, 1, NULL);
  xTaskCreate(TaskPrint, "Print", 128, NULL, 1, NULL);

  // Start FreeRTOS scheduler
  vTaskStartScheduler();
}
//===============================================================================
void loop() {
  // Empty — control is given to the RTOS
}
//===============================================================================
void TaskBlink(void *pvParameters) {
  pinMode(LED_BUILTIN, OUTPUT);
  while (1) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("LED ON  | Time: ");
    Serial.println(millis());
    vTaskDelay(5000 / portTICK_PERIOD_MS);   // ✅ Non-blocking delay

    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("LED OFF | Time: ");
    Serial.println(millis());
    vTaskDelay(5000 / portTICK_PERIOD_MS);   // ✅ Non-blocking delay
  }
}
//===============================================================================
void TaskPrint(void *pvParameters) {
  while (1) {
    Serial.print("Hello | Time: ");
    Serial.println(millis());
    vTaskDelay(500 / portTICK_PERIOD_MS);    // ✅ Keeps running independently
  }
}
//===============================================================================
