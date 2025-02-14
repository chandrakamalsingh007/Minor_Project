#include <Keypad.h>
#include <Servo.h>

// Keypad configuration
const byte ROW_NUM    = 4; // Four rows
const byte COL_NUM    = 4; // Four columns
char keys[ROW_NUM][COL_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pin_rows[ROW_NUM] = {9, 8, 7, 6};
byte pin_column[COL_NUM] = {5, 4, 3, 2};

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COL_NUM);

// Servo configuration
Servo doorServo;

String correctOTP = "123456";  // The correct OTP sent by the ESP32 (this should be updated dynamically)
String enteredOTP = "";  // To store the OTP entered by the user

void setup() {
  Serial.begin(115200);
  doorServo.attach(9);  // Pin for Servo motor
  doorServo.write(0);   // Lock the door initially
  delay(1000);

  // Ask user to enter OTP
  Serial.println("Enter OTP to unlock the door.");
}

void loop() {
  char key = keypad.getKey();
  
  if (key) {
    if (key == '#') {  // Verify OTP when '#' is pressed
      verifyOTP();
    } else {
      enteredOTP += key;  // Add the pressed key to OTP
    }
  }
}

// Function to verify OTP entered by user
void verifyOTP() {
  Serial.print("Entered OTP: ");
  Serial.println(enteredOTP);

  if (enteredOTP == correctOTP) {
    Serial.println("OTP Verified. Unlocking the door...");
    doorServo.write(90);  // Unlock the door (servo turns to 90 degrees)
    delay(5000);  // Keep the door unlocked for 5 seconds
    doorServo.write(0);  // Lock the door again
  } else {
    Serial.println("Incorrect OTP. Try again.");
  }

  enteredOTP = "";  // Clear entered OTP
}
