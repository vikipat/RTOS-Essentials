//===============================================================================
// Project Name : RTOS Essentials Learning Series
// Author       : Vivek Patel
// Date         : 27/09/2025
// Description  : RTOS Queue Demonstration
//                This example illustrates how tasks can communicate safely
//                using FreeRTOS queues. The producer task sends data into
//                the queue, and the consumer task retrieves it in FIFO order.
//===============================================================================
#include <Arduino_FreeRTOS.h>
#include <queue.h>
//===============================================================================
// Queue handle
QueueHandle_t xQueue;
//===============================================================================
// Incrementer Task – produces counter values
void vIncrementTask(void *pvParameters) {
  int counter = 0;
  for (;;) {
    counter++;  // fake sensor value

    // Send to queue
    if (xQueueSend(xQueue, &counter, (TickType_t)50) == pdPASS) {
      Serial.print("Increment Task: sent = ");
      Serial.println(counter);
    } else {
      Serial.println("Increment Task: queue full!");
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);  // every 500 ms
  }
}
//===============================================================================
// Resetter Task – injects zero
void vResetTask(void *pvParameters) {
  int resetValue = 0;
  for (;;) {
    if (xQueueSend(xQueue, &resetValue, (TickType_t)50) == pdPASS) {
      Serial.println("Reset Task: sent = 0");
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);  // every 1 sec
  }
}
//===============================================================================
// Printer Task – consumes from queue
void vPrintTask(void *pvParameters) {
  int receivedValue;
  for (;;) {
    if (xQueueReceive(xQueue, &receivedValue, portMAX_DELAY) == pdPASS) {
      Serial.print("Print Task: received = ");
      Serial.println(receivedValue);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);  // every 1 sec
  }
}
//===============================================================================
void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  // Create a queue with capacity for 10 integers
  xQueue = xQueueCreate(10, sizeof(int));

  if (xQueue != NULL) {
    xTaskCreate(vIncrementTask, "Inc", 128, NULL, 1, NULL);
    xTaskCreate(vResetTask, "Rst", 128, NULL, 1, NULL);
    xTaskCreate(vPrintTask, "Prt", 128, NULL, 1, NULL);

    vTaskStartScheduler();
  } else {
    Serial.println("Queue creation failed!");
  }
}
//===============================================================================
void loop() {
  // Empty. FreeRTOS scheduler runs tasks.
}
//===============================================================================
