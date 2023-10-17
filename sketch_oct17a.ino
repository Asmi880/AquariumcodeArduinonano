#include <LiquidCrystal.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// LCD Setup
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

// Sensors Setup
OneWire oneWire(7); // DS18B20 Temperature sensor on pin D7
DallasTemperature sensors(&oneWire);
Servo servo;

// Variables for sensor data
float temperature; // Store temperature value
int tdsSensorPin = A0;
int ldrSensorPin = A1;
int servoPin = 3;

// LED Pins
int ledPin1 = 5; // PWM-capable LED pin
int ledPin2 = 6; // PWM-capable LED pin

// Thresholds and motor delay settings
int sensorThresholds[3];
int motorDelay;
int ldrThreshold;
int Flag = 1; // Flag for serial data reception
long int t1, t2; // Variables for tracking time

void setup() {
  // Serial Communication Setup
  Serial.begin(9600); // Initialize serial communication
  while (!Serial); // Wait for the serial connection to be established

  // LCD setup
  lcd.begin(16, 2);

  // Sensors Setup
  sensors.begin();
  pinMode(tdsSensorPin, INPUT);
  pinMode(ldrSensorPin, INPUT);

  servo.attach(servoPin);
  servo.write(90); // Set the servo to the initial position

  // LED Setup
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  analogWrite(ledPin1, 10); // Set the LED brightness to a low value
  digitalWrite(ledPin2, LOW);

  // Initial LCD Message
  lcd.setCursor(0, 0);
  lcd.print("Aquarium Health");
  lcd.setCursor(0, 1);
  lcd.print("Tracking System");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting....");
  delay(1000);
}

void loop() {
  if (Flag == 1) {
    serialRec(); // Call the serial data reception function
    Flag = 2;
    t1 = millis(); // Record the current time
  }

  // Read sensor data
  temperature = readTemperature();
  int tdsValue = analogRead(tdsSensorPin);
  int ldrValue = analogRead(ldrSensorPin);

  // Display sensor data and messages on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:" + String(temperature) + "C");
  lcd.print(" Td:" + String(tdsValue));
  lcd.setCursor(0, 1);
  lcd.print("Lux:" + String(ldrValue);

  // Check sensor thresholds and display messages
  if (temperature > sensorThresholds[0]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp Rise");
  }

  if (tdsValue > sensorThresholds[1]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bad Quality");
    lcd.setCursor(0, 1);
    lcd.print("Water");
  } else if (tdsValue > 50 && tdsValue < sensorThresholds[1] - 10) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Good Quality");
    lcd.setCursor(0, 1);
    lcd.print("Water");
  }

  // Check light level and adjust LED
  if (ldrValue < ldrThreshold) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Light is Low ");
    analogWrite(ledPin1, 255); // Set the LED to full intensity
  } else {
    analogWrite(ledPin1, 10); // Set the LED to low intensity using PWM
  }

  // Perform servo action based on motor delay
  t2 = millis();
  if ((t2 - t1) >= motorDelay) {
    servo.write(40); // Open servo to 90 degrees
    delay(3000); // Keep it open for 3 seconds
    servo.write(90); // Close servo to 0 degrees
    delay(3000); // Keep it closed for 3 seconds
    t1 = millis();
  }

  // Send sensor data via serial
  String sensorData = "*";
  sensorData += String(temperature) + ",";
  sensorData += String(ldrValue) + ",";
  sensorData += String(tdsValue) + "#";
  Serial.println(sensorData);
  delay(1000);
}

float readTemperature() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  return sensors.getTempCByIndex(0); // Index 0 is the only sensor
}

void serialRec() {
  String packet = ""; // Store the received packet
  while (true) {
    if (Serial.available() > 0) {
      packet = Serial.readStringUntil('#'); // Read serial data until '#' is encountered
      processPacket(packet); // Process the received data
      packet = ""; // Clear the packet for the next reception
      break;
    }
  }
}

void processPacket(String packet) {
  // Parse the packet and extract values
  int values[4]; // Assuming there are 4 values in the packet
  int currentIndex = 0;
  String temp = "";
  packet += ","; // Add a ',' to ensure the last value is processed

  // Extract values from the packet
  for (int i = 1; i < packet.length(); i++) {
    char c = packet[i];
    if (c == ',') {
      int val = temp.toInt(); // Convert the temporary string to an integer
      values[currentIndex] = val; // Store the value in the array
      currentIndex++;
      temp = ""; // Clear the temporary string for the next value
    } else {
      temp += c; // Build the temporary string character by character
    }
  }

  // Check if the received values match your expected format
  if (currentIndex == 4) {
    sensorThresholds[0] = values[0]; // Set temperature threshold
    sensorThresholds[1] = values[1]; // Set TDS threshold
    ldrThreshold = values[2]; // Set LDR threshold
    motorDelay = values[3] * 1000; // Set motor delay in milliseconds
  }
}
