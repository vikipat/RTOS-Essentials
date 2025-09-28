//===============================================================================
// Project Name : RTOS Essentials Learning Series
// Author       : Vivek Patel
// Date         : 27/09/2025
// Description  : Global Variable Demonstration
//                This example shows how tasks communicate using a shared global
//                variable. Increment and Reset tasks update the same variable,
//                while the Print task reads it. As there is no protection,
//                the variable can be overwritten or inconsistent — demonstrating
//                the limitation of global variables in multitasking systems.
//===============================================================================
#include <Arduino_FreeRTOS.h>
//===============================================================================
// Shared global variable (not thread-safe)
volatile unsigned long counter = 0;
//===============================================================================
// Increment Task – increments the global counter
void vIncrementTask(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    counter++;
    Serial.print("Increment Task: counter = ");
    Serial.println(counter);
    vTaskDelay(500 / portTICK_PERIOD_MS);  // every 500 ms
  }
}
//===============================================================================
// Reset Task – resets the counter back to 0
void vResetTask(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    counter = 0;
    Serial.println("Reset Task: counter reset to 0");
    vTaskDelay(1000 / portTICK_PERIOD_MS); // every 1 sec
  }
}
//===============================================================================
// Print Task – prints the current counter value
void vPrintTask(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    Serial.print("Print Task: counter = ");
    Serial.println(counter);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // every 1 sec
  }
}
//===============================================================================
void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  // Create tasks
  xTaskCreate(vIncrementTask, "Inc", 128, NULL, 1, NULL);
  xTaskCreate(vResetTask, "Rst", 128, NULL, 1, NULL);
  xTaskCreate(vPrintTask, "Prt", 128, NULL, 1, NULL);

  // Start scheduler
  vTaskStartScheduler();
}
//===============================================================================
void loop() {
  // Not used when running FreeRTOS
}
//===============================================================================