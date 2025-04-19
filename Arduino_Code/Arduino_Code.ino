#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change 0x27 if needed

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Servo motor setup
Servo doorServo;

// Variables
String otp = "0987";
unsigned long doorOpenMillis = 0;
const unsigned long doorCloseDelay = 22000; // 22 seconds
int failedAttempts = 0;
const int maxAttempts = 3;
bool systemShutdown = false;

String lastMsg1 = "", lastMsg2 = "";

// Function declarations
String getOtpFromKeypad();
void openDoor();
void closeDoor();
void showMessage(const String &msg1, const String &msg2);
void shutdownSystem();

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  showMessage("Waiting for", "data...");

  doorServo.attach(10);
  doorServo.write(0);
}

void loop() {
  if (systemShutdown) return;

  if (Serial.available()) {
    String receivedData = Serial.readStringUntil('\n');
    receivedData.trim();

    if (receivedData.startsWith("Signal:")) {
      int signal = receivedData.substring(8).toInt();

      if (signal == 0) {
        showMessage("Unknown!!", "Person....");
        delay(1000);

        String otpData = Serial.readStringUntil('\n');
        otpData.trim();
        if (otpData.startsWith("otp: ")) {
          otp = otpData.substring(5);
          Serial.println("New OTP set: " + otp);
        }

        // OTP Entry loop with retry
        bool otpSuccess = false;
        while (failedAttempts < maxAttempts && !otpSuccess) {
          showMessage("Enter OTP:", "");
          String enteredOtp = getOtpFromKeypad();

          if (enteredOtp == otp) {
            openDoor();
            showMessage("Welcome!", "Home (^-^)");
            delay(2000);  // Show "Welcome Home" message for 2 seconds
            failedAttempts = 0;
            otpSuccess = true;
          } else {
            failedAttempts++;
            showMessage("OTP Failed", "Attempts left: " + String(maxAttempts - failedAttempts));
            delay(1000); // Display message before retry
          }
        }

        if (!otpSuccess && failedAttempts >= maxAttempts) {
          showMessage("Intruder", "Alert!");
          delay(1000);
          shutdownSystem();
        }

      } else if (signal == 1) {
        openDoor();
        showMessage("Welcome!", "Home (^-^)");
        delay(2000);  // Show "Welcome Home" message for 2 seconds
      }
    }
  }

  // Show door close countdown after door is opened
  if (doorOpenMillis > 0 && millis() - doorOpenMillis < doorCloseDelay) {
    int remainingTime = (doorCloseDelay - (millis() - doorOpenMillis)) / 1000;
    showMessage("Door closing in", String(remainingTime) + "s");
  }

  // Close door after timeout
  if (doorOpenMillis > 0 && millis() - doorOpenMillis > doorCloseDelay) {
    closeDoor();
    shutdownSystem();
  }
}

// Show message on LCD
void showMessage(const String &msg1, const String &msg2) {
  if (msg1 != lastMsg1 || msg2 != lastMsg2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(msg1);
    lcd.setCursor(0, 1);
    lcd.print(msg2);
    Serial.println(msg1 + " | " + msg2);
    lastMsg1 = msg1;
    lastMsg2 = msg2;
  }
}

// Get 4-digit OTP with *
String getOtpFromKeypad() {
  String enteredOtp = "";
  while (enteredOtp.length() < 4) {
    char key = keypad.getKey();
    if (key && isDigit(key)) {
      enteredOtp += key;
      lcd.setCursor(enteredOtp.length() - 1, 1);
      lcd.print("*");
      Serial.print("*");
    }
  }
  Serial.println();
  return enteredOtp;
}

// Open the door
void openDoor() {
  doorServo.write(90);
  doorOpenMillis = millis();
  Serial.println("Door opened.");
}

// Close the door
void closeDoor() {
  doorServo.write(0);
  doorOpenMillis = 0;
  Serial.println("Door closed.");
}

// Shutdown system and turn off LCD
void shutdownSystem() {
  systemShutdown = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Shutting down");
  lcd.setCursor(0, 1);

  for (int i = 0; i < 10; i++) {
    lcd.print(".");
    delay(300);
  }

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("System Off");
  lcd.setCursor(1, 1);
  lcd.print("Press RESET btn");
  Serial.println("System is now off.");

  delay(3000);
  lcd.noBacklight();  // Turn off LCD screen
}
