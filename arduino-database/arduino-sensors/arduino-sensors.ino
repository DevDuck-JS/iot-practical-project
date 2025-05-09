// Include Arduino Wire library for I2C
#include <Wire.h> 
// Include LCD display library for I2C
#include <LiquidCrystal_I2C.h>
// Include Keypad library
#include <Keypad.h>


// Ultrasonic initial trigger
int triggerThreshold = 20;  // default threshold


// Length of password + 1 for null character
#define Password_Length 8
// Character to hold password input
char Data[Password_Length];

// Character to hold key input
char customKey;

// Counter for character entries
byte data_count = 0;

// Active Buzzer
const int buzzerPin = A2;

// Ultrasonics
const int trigPin = A0;  // Trigger is on A0
const int echoPin = A1;  // Echo is on A1
long duration;
int distance;

// RGB
const int ledRed = 11;
const int ledGreen = 10;
const int ledBlue = 12;


// Constants for row and column sizes
const byte ROWS = 4;
const byte COLS = 4;

// Array to represent keys on keypad
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Connections to Arduino
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

// Create keypad object
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Create LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2);  

void setup(){

  // Setup LCD with backlight and initialize
  lcd.init(); 
  lcd.backlight();

  // Setup Buzzer
  pinMode(buzzerPin, OUTPUT);
  
  // Setup Ultrasonics
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Setup LED
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledGreen, OUTPUT);

  Serial.begin(9600);

}

void loop(){

  // ✅ Listen for commands from Flask
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');

    if (command == "BUZZER:ON") {
      digitalWrite(buzzerPin, HIGH);
      delay(1000);
      digitalWrite(buzzerPin, LOW);
    }

    if (command.startsWith("THRESHOLD:")) {
      String value = command.substring(10);
      triggerThreshold = value.toInt();
      Serial.print("Threshold updated to: ");
      Serial.println(triggerThreshold);
    }
  }


  // Ultrasobnix sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.0344 / 2;

  Serial.print("DISTANCE:");
  Serial.println(distance);

  if (distance > triggerThreshold) {
    digitalWrite(buzzerPin, LOW);        // Buzzer Silence
    digitalWrite(ledBlue, 0);             // LED OFF: Blue 
    lcd.setCursor(0, 0);
    lcd.print("Approach to activate");
    delay(100);
  } else {
    digitalWrite(ledBlue, 1);             // LED ON: Blue 

    // Initialize LCD and print
    lcd.setCursor(0, 0);
    lcd.print("Enter 7-Digit Code:");
  
    // Get key value if pressed
    char customKey = customKeypad.getKey();
    
    if (customKey) {
      // Enter keypress into array and increment counter
      Data[data_count] = customKey;
      lcd.setCursor(data_count, 4);
      lcd.print(Data[data_count]);
      data_count++;
    }

    // See if we have reached the password length
  
    if (data_count == Password_Length - 1) {
      lcd.clear();
      Serial.print("PASSWORD:");
      Serial.println(Data);         // Send password to Python
      delay(1000);                  // Wait for Python to respond

      while (!Serial.available()); // Wait until response
      String userType = Serial.readStringUntil('\n');

      if (userType == "Admin" || userType == "Residence") {
        lcd.print("Welcome, " + userType);
        digitalWrite(buzzerPin, HIGH);            // Buzzer beeps
        digitalWrite(ledRed, 0);                  // LED ON: Red 
        digitalWrite(ledGreen, 1);                // LED ON: Green 
        digitalWrite(ledBlue, 0);                 // LED OFF: Blue 
        delay(2000);
        digitalWrite(buzzerPin, LOW);             // Buzzer silence
        lcd.clear();
        

        // Ask if they want to change password
        lcd.print("Change code?");
        lcd.setCursor(0, 1);
        lcd.print("1=Yes 0=No");

        bool decisionMade = false;
        char changeChoice;

        while (!decisionMade) {
          changeChoice = customKeypad.getKey();
          if (changeChoice == '1' || changeChoice == '0') {
            decisionMade = true;
          }
        }

        if (changeChoice == '1') {
          lcd.clear();
          lcd.print("New 7-Digit Code:");
          char newCode[Password_Length];
          byte newCodeCount = 0;

          while (newCodeCount < Password_Length - 1) {
            char key = customKeypad.getKey();
            if (key) {
              newCode[newCodeCount] = key;
              lcd.setCursor(newCodeCount, 1);
              lcd.print(key);
              newCodeCount++;
            }
          }

          newCode[newCodeCount] = '\0';       // Null-terminate the string

          // Send change request to Python
          Serial.print("CHANGE:");
          Serial.print(userType);             // Admin or Residence
          Serial.print(":");
          Serial.println(newCode);
          lcd.clear();
          lcd.print("Code Updated");
          digitalWrite(buzzerPin, HIGH);         // Buzzer beeps

          digitalWrite(ledRed, 0);                // LED ON: Red 
          digitalWrite(ledGreen, 1);             // LED ON: Green 
          digitalWrite(ledBlue, 0);               // LED OFF: Blue 
          delay(2000);
        }

        else if (changeChoice == '0'){
          lcd.clear();
          lcd.print("You're logged out");
          delay(2000);
        }
      }
      else {
        lcd.print("Access Denied");
        digitalWrite(buzzerPin, HIGH);          // Buzzer beeps
        delay(200);
        digitalWrite(buzzerPin, LOW);           // Buzzer beeps
        delay(200);
        digitalWrite(buzzerPin, HIGH);          // Buzzer beeps
        delay(200);
        digitalWrite(buzzerPin, LOW);           // Buzzer beeps
        delay(200);
        digitalWrite(ledRed, 1);                // LED ON: Red 
        digitalWrite(ledGreen, 0);              // LED OFF: Green 
        digitalWrite(ledBlue, 0);               // LED OFF: Blue 
        delay(2000);
      }

      delay(2000);

      // Reset everything
      lcd.clear();
      clearData();
      digitalWrite(buzzerPin, LOW);           // Buzzer beeps
      digitalWrite(ledRed, 0);                // LED OFF: Red 
      digitalWrite(ledGreen, 0);              // LED OFF: Green 
      digitalWrite(ledBlue, 0);               // LED OFF: Blue 
    }
  }
  delay(100);
}

void clearData() {
  // Go through array and clear data
  while (data_count != 0) {
    Data[data_count--] = 0;
  }
  return;
}