#include <Servo.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// Configure hardware pins
#define SERVO_PIN 10
#define BATTERY_PIN A0

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo doorLock;

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

String receivedOTP = "";
String enteredOTP = "";
String emergencyCode = "778899";  // Change this code
int failedAttempts = 0;
unsigned long lastActivity = 0;

void setup() {
  Serial.begin(115200);
  doorLock.attach(SERVO_PIN);
  lcd.init();
  lcd.backlight();
  doorLock.write(0);
  EEPROM.begin(512);
  
  // Initialize emergency code in EEPROM
  if(EEPROM.read(0) == 255){
    for(int i=0; i<emergencyCode.length(); i++){
      EEPROM.write(i, emergencyCode[i]);
    }
  }
}

void loop() {
  // Check serial messages
  if(Serial.available()){
    String command = Serial.readStringUntil('\n');
    if(command.startsWith("OTP:")){
      receivedOTP = command.substring(4);
      lcd.clear();
      lcd.print("Enter OTP:");
      failedAttempts = 0;
    }
    else if(command.startsWith("GRANTED:")){
      String user = command.substring(8);
      accessGranted(user);
    }
  }

  // Handle keypad input
  char key = keypad.getKey();
  if(key){
    lastActivity = millis();
    
    // Emergency override
    if(key == '*'){
      checkEmergencyCode();
    }
    // OTP entry
    else if(key == '#'){
      validateOTP();
    }
    else {
      enteredOTP += key;
      updateDisplay();
    }
  }

  // Battery monitoring
  static unsigned long lastBatteryCheck = 0;
  if(millis() - lastBatteryCheck > 60000){
    checkBattery();
    lastBatteryCheck = millis();
  }

  // Auto-lock after 30 seconds
  if(millis() - lastActivity > 30000){
    lcd.clear();
    lcd.print("System Ready");
  }
}

void checkEmergencyCode(){
  String code = "";
  lcd.clear();
  lcd.print("Emergency Code:");
  
  while(true){
    char key = keypad.getKey();
    if(key){
      if(key == '#') break;
      code += key;
      lcd.setCursor(code.length()-1, 1);
      lcd.print("*");
      
      if(code.length() == 6){
        String storedCode;
        for(int i=0; i<6; i++){
          storedCode += char(EEPROM.read(i));
        }
        if(code == storedCode){
          emergencyOpen();
          return;
        }
        else {
          lcd.clear();
          lcd.print("Invalid Code!");
          delay(2000);
          lcd.print("System Ready");
          return;
        }
      }
    }
  }
}

void validateOTP(){
  lcd.clear();
  if(enteredOTP == receivedOTP){
    accessGranted("OTP User");
  }
  else {
    failedAttempts++;
    lcd.print("Wrong OTP!");
    lcd.setCursor(0,1);
    lcd.print("Attempts: " + String(failedAttempts));
    delay(2000);
    
    if(failedAttempts >= 3){
      lcd.clear();
      lcd.print("System Locked!");
      Serial.println("LOCKED");
      delay(10000);
      failedAttempts = 0;
    }
  }
  enteredOTP = "";
  receivedOTP = "";
  lcd.clear();
  lcd.print("System Ready");
}

void accessGranted(String user){
  lcd.clear();
  lcd.print("Welcome ");
  lcd.setCursor(0,1);
  lcd.print(user);
  doorLock.write(90);
  delay(5000);
  doorLock.write(0);
  lcd.clear();
  lcd.print("System Ready");
}

void emergencyOpen(){
  lcd.clear();
  lcd.print("EMERGENCY OPEN!");
  doorLock.write(90);
  Serial.println("EMERGENCY_USED");
  delay(5000);
  doorLock.write(0);
  lcd.clear();
  lcd.print("System Ready");
}

void checkBattery(){
  int raw = analogRead(BATTERY_PIN);
  float voltage = raw * (5.0 / 1023.0) * 2;
  float percentage = (voltage - 3.2) / (4.2 - 3.2) * 100;
  
  if(percentage < 20){
    Serial.println("LOW_BATTERY");
    lcd.clear();
    lcd.print("Low Battery!");
    lcd.setCursor(0,1);
    lcd.print(String(percentage) + "%");
    delay(2000);
  }
}

void updateDisplay(){
  lcd.clear();
  lcd.print("Enter OTP:");
  lcd.setCursor(0,1);
  for(int i=0; i<enteredOTP.length(); i++){
    lcd.print("*");
  }
}