#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// WiFi credentials
const char* ssid = "yourSSID";
const char* password = "yourPassword";

// Twilio credentials
String twilioSID = "yourTwilioSID";
String twilioAuthToken = "yourTwilioAuthToken";
String twilioPhoneNumber = "yourTwilioPhoneNumber"; // From Twilio
String ownerPhone = "+1234567890"; // Owner's phone number


String generatedOTP = "";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Wait for Wi-Fi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");

  generateOTP();      // Generate OTP
  sendOTP();          // Send OTP via SMS
}

void loop() {
  // You can add other functions here if needed
}

// Generate a random OTP
void generateOTP() {
  generatedOTP = String(random(100000, 999999)); // Generate a 6-digit OTP
  Serial.println("Generated OTP: " + generatedOTP);
}

// Send OTP via Twilio SMS API
void sendOTP() {
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();  // Disable SSL verification for simplicity (in production, use certificate)

  String url = "https://api.twilio.com/2010-04-01/Accounts/" + twilioSID + "/Messages.json";
  String message = "OTP: " + generatedOTP;

  http.begin(client, url);
  http.addHeader("Authorization", "Basic " + base64::encode(twilioSID + ":" + twilioAuthToken));
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String payload = "To=" + ownerPhone + "&From=" + twilioPhoneNumber + "&Body=" + message;

  int httpResponseCode = http.POST(payload);
  if (httpResponseCode == 200) {
    Serial.println("OTP Sent successfully.");
  } else {
    Serial.println("Error sending OTP: " + String(httpResponseCode));
  }
  http.end();
}

