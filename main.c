#include <LiquidCrystal.h>
#include <Keypad.h>

// LCD Pins
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Motor Control Pins
const int in1 = 7;  // Motor direction pin 1
const int in2 = 8;  // Motor direction pin 2
const int motorPin = 10;  // PWM pin for motor speed control
const int brakeLightPin = 6; // Brake light LED pin
const int pushbutton = 13;  // Push button pin for brake
#define accelPin A0 // Acceleration pedal input (Potentiometer)

// Keypad Pins
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'D'},
  {'*', '0', '#', 'C'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};  // Adjust pins as per your wiring
byte colPins[COLS] = {5, 4, 3, 2};  // Adjust pins as per your wiring
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Variables
int motorSpeed = 0;      // Current motor speed (PWM)
int temperatureSensorPin = A1;  // LM35 temperature sensor pin
float temperature = 0;      // Measured temperature in Celsius
char unit = 'C';          // Default temperature unit ('C' for Celsius, 'F' for Fahrenheit)
int tempThreshold = 40;   // Default temperature threshold for braking

// Function prototypes
void displaySpeed();
void displayTemperature();
void updateUnit(char key);
void measureTemperature();
void brakeCar();
void accelerate();
void setThreshold();

void setup() {
  // Initialize LCD and motor control pins
  lcd.begin(16, 2);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(brakeLightPin, OUTPUT);
  pinMode(pushbutton, INPUT_PULLUP);  // Set pushbutton with internal pull-up resistor
  Serial.begin(9600);

  // Initial message on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Unit:");
  lcd.setCursor(0, 1);
  lcd.print("1 = C, 2 = F");
}

void loop() {
  // Read keypad input
  char key = customKeypad.getKey();
  if (key) {
    if (key == 'A') {
      setThreshold();  // Enter threshold setup mode
    } else {
      updateUnit(key);  // Update unit based on keypad input
    }
  }

  // Measure temperature and display
  measureTemperature();

  // Push button logic to control the LED
  if (digitalRead(pushbutton) == LOW) { // Button is pressed (LOW with internal pull-up)
    digitalWrite(brakeLightPin, LOW); // Turn on LED
    brakeCar();  // Gradual deceleration
  } else {
    digitalWrite(brakeLightPin, HIGH); // Turn off LED
    accelerate();  // Control acceleration based on pedal input
  }
  
  displayTemperature();  // Show temperature on the LCD
  //delay(500); // Update frequency
}

// Function to handle updating the temperature unit based on keypad input
void updateUnit(char key) {
  if (key == '1') {
    unit = 'C';  // Celsius
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Celsius Selected");
    delay(1000);
  } else if (key == '2') {
    unit = 'F';  // Fahrenheit
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fahrenheit Selected");
    delay(1000);
  }
}

// Function to set the temperature threshold using the keypad
void setThreshold() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Temp Thr:");
  String input = "";  // To collect user input
  
  while (true) {
    char key = customKeypad.getKey();
    if (key) {
      if (key >= '0' && key <= '9') {
        input += key;  // Append digit to input string
        lcd.setCursor(0, 1);
        lcd.print(input);
      } else if (key == '#') {  // Confirm input with '#'
        tempThreshold = input.toInt();  // Convert input to integer
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Threshold Set:");
        lcd.setCursor(0, 1);
        lcd.print(tempThreshold);
        delay(1000);
        break;
      } else if (key == '*') {  // Cancel input with '*'
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Threshold Cancel");
        delay(1000);
        break;
      }
    }
  }
}

// Function to measure temperature from the LM35 sensor
void measureTemperature() {
  int sensorValue = analogRead(temperatureSensorPin);
  temperature = (sensorValue * 5.0 / 1023.0) * 100.0;
  if (unit == 'F') {
    temperature = (temperature * 9.0 / 5.0) + 32;
  }
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println((unit == 'C') ? " °C" : " °F");
}

// Function to brake the car
void brakeCar() {
  for (int i = motorSpeed; i >= 0; i -= 10) {
    analogWrite(motorPin, i);
    delay(100); 
  }
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(motorPin, 0); 
  motorSpeed = 0; 
  Serial.println("Braking: Motor stopped.");
}

// Function to accelerate the car
void accelerate() {
  int accelValue = analogRead(accelPin); 
  motorSpeed = map(accelValue, 0, 1023, 0, 255); 
  digitalWrite(in1, HIGH); 
  digitalWrite(in2, LOW);
  analogWrite(motorPin, motorSpeed); 
  Serial.print("Accelerating: Speed = ");
  Serial.println(motorSpeed);
}

// Function to display the temperature on the LCD
void displayTemperature() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print((unit == 'C') ? " C" : " F");
  lcd.setCursor(0, 1);
  lcd.print("Thr: ");
  lcd.print(tempThreshold);
}
