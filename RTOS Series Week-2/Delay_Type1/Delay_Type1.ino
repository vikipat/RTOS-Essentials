//===============================================================================
// Project Name : Baremetal Delay Demo
// Author       : Vivek Patel
// Date         : 20/09/2025
// Description  : Demonstration of blocking delay() in Arduino
//===============================================================================
#include <Arduino.h>
//===============================================================================
void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}
//===============================================================================
void loop() {
  // Toggle LED
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("LED ON  | Time: ");
  Serial.println(millis());
  delay(2000);   // ❌ BLOCKS everything for 2 seconds

  digitalWrite(LED_BUILTIN, LOW);
  Serial.print("LED OFF | Time: ");
  Serial.println(millis());
  delay(2000);   // ❌ BLOCKS everything for 2 seconds

  // Try to print something else
  Serial.print("Hello | Time: ");
  Serial.println(millis());
  delay(500);    // ❌ This will never run independently
}
//===============================================================================