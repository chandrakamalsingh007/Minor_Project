#include <WiFi.h>
#include "esp_camera.h"
#include <HTTPClient.h>
#include<camera_pins.h>

//select camera model
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Python server URL
const char* serverUrl = "http://YOUR_PYTHON_SERVER_IP:5000/recognize";



void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize Camera
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Select lower framesize for better performance
  config.frame_size = FRAMESIZE_QVGA; 
  config.jpeg_quality = 12; 
  config.fb_count = 1;

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera initialization failed: 0x%x", err);
    return;
  }
  Serial.println("ESP32-CAM initialized");
}

void loop() {
  // Wait for START signal from Arduino
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command == "START") {
      // Capture image
      camera_fb_t* fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        return;
      }

      // Send image to Python server
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "image/jpeg");
      int httpResponseCode = http.POST(fb->buf, fb->len);

      if (httpResponseCode == 200) {
        String response = http.getString();
        if (response.startsWith("NAME:")) {
          Serial.println(response); // Send recognized name to Arduino
        } else if (response == "INTRUDER") {
          Serial.println("INTRUDER"); // Send intruder alert to Arduino
          sendIntruderAlert(fb->buf, fb->len);
        }
      } else {
        Serial.println("Error sending image to server");
      }

      http.end();
      esp_camera_fb_return(fb); // Free memory
    }
  }
}

void sendIntruderAlert(const uint8_t* image, size_t len) {
  // Send image to Telegram or WhatsApp (Handled by Python server)
  HTTPClient http;
  http.begin("http://YOUR_PYTHON_SERVER_IP:5000/alert");
  http.addHeader("Content-Type", "image/jpeg");
  http.POST(image, len);
  http.end();
}
