#include <Keypad.h>
#include <Servo.h>

// LCD setup

// Keypad setup
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};  // Connect to row pinouts
byte colPins[COLS] = {5, 4, 3, 2};  // Connect to column pinouts
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Servo motor setup
Servo doorServo;

// Variables
String otp = "0987"; // Pre-set correct OTP
unsigned long doorOpenMillis = 0;
const unsigned long doorCloseDelay = 20000;

// Function declarations
String getOtpFromKeypad();
void openDoor();
void closeDoor();


void setup() {
  Serial.begin(115200); // Initialize communication with ESP32-CAM
  Serial.println("Waiting for the data..");
  doorServo.attach(10); // Servo on pin 10
  doorServo.write(0); // Keep door closed initially
}

void loop() {
  if (Serial.available()) { 
    Serial.println("Data received ....");
    String receivedData = Serial.readStringUntil('\n'); // Read the incoming data
    receivedData.trim(); // Remove any leading/trailing spaces or newline characters

    if (receivedData.startsWith("Signal:")) {
      int signal = receivedData.substring(8).toInt(); // Extract the signal value
      Serial.println("Signal: " + String(signal));    // Print the signal for debugging

      if (signal == 0) {
        // Get the OTP
        String otpData = Serial.readStringUntil('\n'); // Read the next line for OTP
        otpData.trim(); // Clean the data
        if (otpData.startsWith("otp: ")) {
          otp = otpData.substring(5); // Extract the OTP value
          Serial.println("otp: " + otp); // Print the OTP for debugging
        }

        Serial.println("Enter OTP:");
        String enteredOtp = getOtpFromKeypad(); // Get OTP from keypad
        Serial.print("Entered OTP: ");
        Serial.println(enteredOtp); // Display entered OTP on Serial Monitor

        if (enteredOtp == otp) {
          openDoor();
          Serial.println("Welcome Home.");
        } else {
          Serial.println("OTP Failed. Try Again...");
        }

      } else if (signal == 1) {
        openDoor();
        Serial.println("Welcome Home.");
      }
    }
  }else if(!Serial.available()){
    Serial.println("No serial Connection.");
    Serial.println("Enter Master Code:");

    String enteredOtp = getOtpFromKeypad();
    if(enteredOtp == otp){
      openDoor();
      Serial.println("Welcome Home.");
    }else{
      Serial.println("GOOD BYE!!")
    }
  }

  
  // Close the door after 2 minutes if open
  if (doorOpenMillis > 0 && millis() - doorOpenMillis > doorCloseDelay) {
    closeDoor();
  }
}


// Function to get OTP from keypad
String getOtpFromKeypad() {
  String enteredOtp = "";
  while (enteredOtp.length() < 4) {
    char key = keypad.getKey();
    if (key && isDigit(key)) {
      enteredOtp += key;
      Serial.println(key);
    }
  }
  Serial.println(enteredOtp);
  return enteredOtp;
}

// Function to open the door
void openDoor() {
  doorServo.write(90); // Unlock position
  doorOpenMillis = millis(); // Record the time door was opened
  Serial.println("Door opened.");
}

// Function to close the door
void closeDoor() {
  doorServo.write(0); // Locked position
  doorOpenMillis = 0; // Reset the timer
  Serial.println("Door Closed.");
}

