#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoQueue.h>
#include <esp_task_wdt.h>

// ===========================
// Configuration
// ===========================
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Network Configuration
const char *ssid = "**********";
const char *password = "**********";
const char* serverURL = "https://YOUR_SERVER:5000/verify";
const char* statusURL = "https://YOUR_SERVER:5000/status";

// System Parameters
const int MAX_RETRIES = 3;
const int CAMERA_TIMEOUT = 5000;
const unsigned long STATUS_CHECK_INTERVAL = 60000;
const size_t MAX_QUEUE_MEMORY = 2048;  // 2KB
const size_t MAX_FRAME_SIZE = 150000;   // 150KB

// ===========================
// Global Variables
// ===========================
ArduinoQueue<String> alertQueue(5);
size_t currentQueueMemory = 0;
bool systemActive = false;
unsigned long lastStatusCheck = 0;
bool faceRecognitionActive = false;
unsigned long frameCounter = 0;
WiFiClientSecure client;

// ===========================
// Camera Initialization
// ===========================
bool initializeCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_VGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("CAM_ERROR:0x%x\n", err);
    return false;
  }
  
  sensor_t *s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_VGA);
  return true;
}

// ===========================
// WiFi Management
// ===========================
bool connectWiFi(int maxRetries = 3) {
  if(WiFi.status() == WL_CONNECTED) return true;
  
  int retries = 0;
  while(retries < maxRetries) {
    WiFi.begin(ssid, password);
    WiFi.setSleep(true);
    
    Serial.print("WIFI_CONNECTING:");
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && 
          (millis() - startTime < 20000)) {
      delay(500);
      Serial.print(".");
    }
    
    if(WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWIFI_CONNECTED");
      return true;
    }
    retries++;
    Serial.printf("\nWIFI_FAILED:%d/%d\n", retries, maxRetries);
  }
  return false;
}

// ===========================
// Main Setup
// ===========================
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);
  esp_task_wdt_init(10, true);  // 10-second watchdog

  if(!initializeCamera()) {
    Serial.println("RESTARTING");
    delay(1000);
    ESP.restart();
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  Serial.println("SYSTEM_READY");
}

// ===========================
// Main Loop
// ===========================
void loop() {
  esp_task_wdt_reset();
  
  if(Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if(command == "START_FACE_RECOG") {
      systemActive = true;
      faceRecognitionActive = true;
    }
  }

  if(systemActive && faceRecognitionActive) {
    if(performFaceRecognition()) {
      systemActive = false;
    }
    faceRecognitionActive = false;
  }

  if(millis() - lastStatusCheck > STATUS_CHECK_INTERVAL) {
    lastStatusCheck = millis();
  }

  if(!systemActive) {
    delay(100);
  }
}

// ===========================
// Face Recognition Flow
// ===========================
bool performFaceRecognition() {
  Serial.println("SCANNING_FACE");
  frameCounter++;
  
  camera_fb_t *fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("CAM_CAPTURE_FAIL");
    return false;
  }

  if(fb->len > MAX_FRAME_SIZE) {
    Serial.println("FRAME_SIZE_ERROR");
    esp_camera_fb_return(fb);
    return false;
  }

  bool result = false;
  if(connectWiFi()) {
    HTTPClient http;
    http.begin(client, serverURL);
    http.addHeader("Content-Type", "image/jpeg");

    int httpCode = http.POST(fb->buf, fb->len);
    if(httpCode > 0) {
      String response = http.getString();
      if(response.startsWith("ACCESS_GRANTED")) {
        Serial.println("GRANTED");
        result = true;
      }
      else {
        Serial.print("RECOG_FAIL:");
        Serial.println(response);
      }
    } else {
      Serial.printf("SERVER_ERROR:%d\n", httpCode);
    }
    http.end();
  } else {
    Serial.println("WIFI_UNAVAILABLE");
  }

  esp_camera_fb_return(fb);
  return result;
}
