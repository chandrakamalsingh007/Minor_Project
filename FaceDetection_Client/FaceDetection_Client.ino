#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Camera model and configuration
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// ===========================
// WiFi Configuration
// ===========================
const char *ssid = "**********";
const char *password = "**********";
const char* serverURL = "http://YOUR_SERVER_IP:5000/verify";
const char* statusURL = "http://YOUR_SERVER_IP:5000/status";

// ===========================
// Camera Configuration
// ===========================
void setupCamera() {
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
  config.frame_size = FRAMESIZE_SVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x", err);
    ESP.restart();
  }

  // Adjust sensor settings
  sensor_t *s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_SVGA);  // 800x600
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(500);
  
  setupCamera();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  
  Serial.print("Connecting to WiFi");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect!");
    ESP.restart();
  }
  
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check for system alerts from Arduino
  if (Serial.available()) {
    String alert = Serial.readStringUntil('\n');
    if (alert == "LOW_BATTERY") {
      sendStatusAlert("Low Battery Warning!");
    }
  }

  // Main recognition cycle
  if (WiFi.status() == WL_CONNECTED) {
    captureAndProcessImage();
  } else {
    Serial.println("WiFi disconnected!");
    delay(1000);
  }
}

void captureAndProcessImage() {
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "image/jpeg");
  
  int httpCode = http.POST(fb->buf, fb->len);
  if (httpCode > 0) {
    String response = http.getString();
    if (response.startsWith("ACCESS_GRANTED")) {
      Serial.println("GRANTED:" + response.substring(15));
    } 
    else if (response.startsWith("OTP:")) {
      Serial.println(response);
    }
    else {
      Serial.println("Unexpected response: " + response);
    }
  } else {
    Serial.printf("HTTP error: %s\n", http.errorToString(httpCode).c_str());
  }

  esp_camera_fb_return(fb);
  http.end();
  delay(3000);  // Reduced delay for faster response
}

void sendStatusAlert(String message) {
  HTTPClient http;
  http.begin(statusURL);
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.POST(message);
  
  if (httpCode <= 0) {
    Serial.printf("Status alert failed: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}