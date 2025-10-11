//===============================================================================
// Project Name : RTOS Essentials Learning Series
// Author       : Vivek Patel
// Date         : 11/10/2025
// Description  : Combined Demo – Task Synchronization + Software Timer
//                Two tasks share Serial safely using a semaphore.
//                A software timer toggles an LED every 1 second.
//===============================================================================
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <timers.h>
//===============================================================================
#define LED_PIN 13
//===============================================================================
SemaphoreHandle_t xSerialSemaphore;
TimerHandle_t xLedTimer;
//===============================================================================
void vTaskA(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    if (xSemaphoreTake(xSerialSemaphore, (TickType_t)10) == pdTRUE) {
      Serial.println("Task A running...");
      xSemaphoreGive(xSerialSemaphore);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
//===============================================================================
void vTaskB(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    if (xSemaphoreTake(xSerialSemaphore, (TickType_t)10) == pdTRUE) {
      Serial.println("Task B running...");
      xSemaphoreGive(xSerialSemaphore);
    }
    vTaskDelay(700 / portTICK_PERIOD_MS);
  }
}
//===============================================================================
void vLedTimerCallback(TimerHandle_t xTimer) {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}
//===============================================================================
void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  // Create semaphore for Serial access
  xSerialSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xSerialSemaphore);

  // Create LED timer – 1 s auto-reload
  xLedTimer = xTimerCreate("LEDTimer",
                           1000 / portTICK_PERIOD_MS,
                           pdTRUE,
                           (void*)0,
                           vLedTimerCallback);
  if (xLedTimer != NULL) xTimerStart(xLedTimer, 0);

  // Create tasks
  xTaskCreate(vTaskA, "TaskA", 128, NULL, 1, NULL);
  xTaskCreate(vTaskB, "TaskB", 128, NULL, 1, NULL);

  vTaskStartScheduler();
}
//===============================================================================
void loop() {}
//===============================================================================
