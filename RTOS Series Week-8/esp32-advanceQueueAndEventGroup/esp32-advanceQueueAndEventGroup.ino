//===============================================================================
// RTOS Essentials - Week 8
// Advanced Queues + Event Groups (ESP32)
// Author : Vivek Patel 
// Date   : 22/11/2025
//===============================================================================
//
// Features:
// - Sensor producer pushes SensorData_t into a FreeRTOS queue (struct-based)
// - WiFi task signals WiFi readiness via Event Group
// - Fusion task waits for BIT_SENSOR_READY + BIT_WIFI_READY then processes queue
// - Optional DHT22 sensor; simulation fallback if sensor not available
// - Tasks pinned to specific cores to show multi-core behavior
//
// IMPORTANT:
// - Do NOT commit WiFi credentials to git. Use config.h (ignored via .gitignore).
//===============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"

// DHT support (optional)
//#include "DHT.h"

// -------- CONFIG (edit locally, don't commit) --------
// Create a file named config.h in the same folder with the following two lines:
// #define WIFI_SSID "your-ssid"
// #define WIFI_PASS "your-password"
// and add config.h to .gitignore before committing.
#include "config.h"

// Set to 1 to use a real DHT22 on DHT_PIN; set to 0 to use synthetic sensor values
#define USE_REAL_SENSOR 0
#define DHT_PIN 4
#define DHT_TYPE DHT22

// Queue size
#define SENSOR_QUEUE_LENGTH 6

// Event bits
#define BIT_SENSOR_READY (1 << 0)
#define BIT_WIFI_READY   (1 << 1)



#if USE_REAL_SENSOR
// DHT support (optional)
#include "DHT.h"
#endif


// Struct passed via queue
typedef struct {
  float temperature;
  float humidity;
  uint32_t timestamp; // millis()
} SensorData_t;

// FreeRTOS objects
static EventGroupHandle_t eventGroup;
static QueueHandle_t sensorQueue;

// Task handles
TaskHandle_t TaskSensorHandle = NULL;
TaskHandle_t TaskWiFiHandle   = NULL;
TaskHandle_t TaskFusionHandle = NULL;

// DHT instance (if used)
#if USE_REAL_SENSOR
DHT dht(DHT_PIN, DHT_TYPE);
#endif

// -------------------- Task: Sensor (Producer) --------------------
void TaskSensor(void *pvParameters) {
  (void) pvParameters;
  Serial.print("TaskSensor started on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    SensorData_t data;
    data.timestamp = millis();

#if USE_REAL_SENSOR
    // Read DHT22 (slow sensor; allow time between samples)
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("DHT read failed, skipping sample");
      // Wait and continue
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      continue;
    }
    data.temperature = t;
    data.humidity = h;
#else
    // Simulation: generate plausible values
    static float simTemp = 24.5;
    static float simHum  = 45.0;
    simTemp += (rand() % 21 - 10) * 0.05; // small variation ±0.5
    simHum  += (rand() % 21 - 10) * 0.1;  // small variation ±1.0
    data.temperature = simTemp;
    data.humidity = simHum;
#endif

    // Try to send to queue (block up to 100ms if queue full)
    if (xQueueSend(sensorQueue, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
      Serial.printf("TaskSensor: queued sample (t=%.2fC h=%.2f%%) @%u on core %d\n",
                    data.temperature, data.humidity, data.timestamp, xPortGetCoreID());
      // Signal sensor ready
      xEventGroupSetBits(eventGroup, BIT_SENSOR_READY);
    } else {
      Serial.println("TaskSensor: queue full, sample dropped");
    }

    // Sample every 2 seconds (DHT22 recommended slow rate)
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

// -------------------- Task: WiFi (connect and signal) --------------------
void TaskWiFi(void *pvParameters) {
  (void) pvParameters;
  Serial.print("TaskWiFi started on core ");
  Serial.println(xPortGetCoreID());

  WiFi.mode(WIFI_STA);
  WiFi.setHostname("rtos-esp32-demo");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.printf("TaskWiFi: connecting to SSID '%s' ...\n", WIFI_SSID);

  // Wait (non-busy) for connection with timeout
  const uint32_t timeoutMs = 15000;
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
    vTaskDelay(200 / portTICK_PERIOD_MS);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("TaskWiFi: connected, IP=");
    Serial.println(WiFi.localIP());
    // Set WiFi ready bit
    xEventGroupSetBits(eventGroup, BIT_WIFI_READY);
  } else {
    Serial.println();
    Serial.println("TaskWiFi: initial connection failed - entering retry loop");
    // Retry loop (retry every 5 seconds) - simple demo logic
    for (;;) {
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        Serial.println("TaskWiFi: retrying connect...");
      } else {
        Serial.println("TaskWiFi: reconnected!");
        xEventGroupSetBits(eventGroup, BIT_WIFI_READY);
        break;
      }
    }
  }

  // If you want TaskWiFi to remain active, you can vTaskDelay here
  // For demo we delete the task to save resources
  vTaskDelete(NULL);
}

// -------------------- Task: Fusion/Logger (Waits on events, consumes queue) --------------------
void TaskFusion(void *pvParameters) {
  (void) pvParameters;
  Serial.print("TaskFusion started on core ");
  Serial.println(xPortGetCoreID());

  const EventBits_t waitBits = BIT_SENSOR_READY | BIT_WIFI_READY;

  for (;;) {
    // Wait for both bits (AND). Clear bits when returning.
    EventBits_t bits = xEventGroupWaitBits(
        eventGroup,
        waitBits,
        pdTRUE,    // clear bits on exit
        pdTRUE,    // wait for ALL bits (AND)
        portMAX_DELAY);

    if ((bits & waitBits) == waitBits) {
      Serial.println("TaskFusion: Both Sensor & WiFi ready. Processing queued data...");

      // Drain queue: process all available samples
      SensorData_t item;
      while (xQueueReceive(sensorQueue, &item, 0) == pdTRUE) {
        Serial.printf("TaskFusion: Processing sample -> Time:%u ms | Temp: %.2f C | Hum: %.2f %% (core %d)\n",
                      item.timestamp, item.temperature, item.humidity, xPortGetCoreID());
        // Simulate processing time per packet
        vTaskDelay(200 / portTICK_PERIOD_MS);
      }

      Serial.println("TaskFusion: queue empty. Waiting for next readiness signals...");
    }
  }
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  delay(400);
  Serial.println();
  Serial.println("==============================================");
  Serial.println("RTOS Series – Week 8: Advanced Queues + Event Groups");
  Serial.println("==============================================");
  Serial.printf("ESP32 cores: %d\n", portNUM_PROCESSORS);

#if USE_REAL_SENSOR
  dht.begin();
  Serial.println("DHT sensor enabled.");
#else
  Serial.println("DHT sensor NOT used -> running with simulated sensor data.");
#endif

  // Create event group & queue
  eventGroup = xEventGroupCreate();
  if (eventGroup == NULL) {
    Serial.println("Failed to create event group");
    while (1) delay(1000);
  }
  sensorQueue = xQueueCreate(SENSOR_QUEUE_LENGTH, sizeof(SensorData_t));
  if (sensorQueue == NULL) {
    Serial.println("Failed to create sensor queue");
    while (1) delay(1000);
  }

  // Seed random for simulated sensor variation
  srand((unsigned long)millis());

  // Create pinned tasks
  BaseType_t r;

  r = xTaskCreatePinnedToCore(
      TaskSensor,
      "TaskSensor",
      3072,
      NULL,
      1,
      &TaskSensorHandle,
      0); // pin to core 0
  if (r != pdPASS) Serial.println("Failed to create TaskSensor");

  r = xTaskCreatePinnedToCore(
      TaskWiFi,
      "TaskWiFi",
      4096,
      NULL,
      2,
      &TaskWiFiHandle,
      1); // pin to core 1
  if (r != pdPASS) Serial.println("Failed to create TaskWiFi");

  r = xTaskCreatePinnedToCore(
      TaskFusion,
      "TaskFusion",
      4096,
      NULL,
      3,
      &TaskFusionHandle,
      1); // pin to core 1 (fusion/logging on core 1 with WiFi)
  if (r != pdPASS) Serial.println("Failed to create TaskFusion");
}

// -------------------- Loop (unused) --------------------
void loop() {
  // Empty - tasks run under FreeRTOS
}
