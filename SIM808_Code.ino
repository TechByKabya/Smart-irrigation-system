
#include <DFRobot_SIM808.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define pins for SIM808
#define PIN_TX 10
#define PIN_RX 11

// Define pins for Ultrasonic Sensor
#define TRIG_PIN 7
#define ECHO_PIN 8

// Define pins for Pumps and Soil Sensor
#define MAIN_PUMP_PIN 13
#define DRAIN_PUMP_PIN 12
#define SOIL_SENSOR_PIN A0

// Define constants
#define WATER_LEVEL_THRESHOLD 5  // Desired water level in cm
#define PHONE_NUMBER "01406645788"
#define MESSAGE_MAIN_ON "Main Pump is ON"
#define MESSAGE_MAIN_OFF "Main Pump is OFF"
#define MESSAGE_LEVEL_REACHED "Water level reached"
#define MESSAGE_FLOOD "Flood detected! Drain Pump is ON"

// Initialize SIM808, LCD, and serial communication
SoftwareSerial mySerial(PIN_TX, PIN_RX);
DFRobot_SIM808 sim808(&mySerial);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C LCD with address 0x27

// Buffers and variables
char message[160];
char phone[16];
char datetime[24];
String rmsg;

// Pump and flood status
unsigned long mainPumpStartTime = 0;
bool mainPumpIsOn = false;
bool floodDetected = false;

void setup() {
  pinMode(MAIN_PUMP_PIN, OUTPUT);
  pinMode(DRAIN_PUMP_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SOIL_SENSOR_PIN, INPUT);

  // Initial state of pumps (relay logic for pulled-up relays)
  digitalWrite(MAIN_PUMP_PIN, HIGH);  // OFF
  digitalWrite(DRAIN_PUMP_PIN, HIGH);  // OFF

  mySerial.begin(9600);
  Serial.begin(9600);

  // Initialize LCD
  lcd.begin();
  lcd.backlight();
  lcd.print("System Starting...");
  delay(2000);

  // Initialize SIM808 module
  lcd.clear();
  lcd.print("Initializing GSM...");
  while (!sim808.init()) {
    lcd.setCursor(0, 1);
    lcd.print("Retrying...");
    delay(1000);
  }
  lcd.clear();
  lcd.print("GSM Ready");
  delay(1000);
}

void loop() {
  Read_SMS();

  if (mainPumpIsOn) {
    float waterLevel = measureWaterLevel();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Main Pump: ON ");
    lcd.setCursor(0, 1);
    lcd.print("Level: ");
    lcd.print(11 - waterLevel);
    lcd.print(" cm");

    if (waterLevel <= WATER_LEVEL_THRESHOLD) {
      sim808.sendSMS(PHONE_NUMBER, MESSAGE_LEVEL_REACHED);
      turnOffMainPump();
    }
  } else if (floodDetected) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Flood Detected!");
    lcd.setCursor(0, 1);
    lcd.print("Drain Pump ON ");
  } else {
    float waterLevel = measureWaterLevel();
    int waterLevelPercent = calculateWaterLevelPercent(waterLevel);
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Main Pump: OFF");
    lcd.setCursor(0, 1);
    lcd.print("Level: ");
    lcd.print(waterLevelPercent);
    lcd.print("%      "); // Ensures clean display by overwriting old content
  }

  checkFlood();
  delay(2000);
}

void Read_SMS() {
  int messageIndex = sim808.isSMSunread();

  if (messageIndex > 0) {
    sim808.readSMS(messageIndex, message, 160, phone, datetime);
    rmsg = String(message);
    rmsg.trim();

    if (rmsg == "pump on") {
      turnOnMainPump();
    } else if (rmsg == "pump off") {
      turnOffMainPump();
    } else if (rmsg == "pump status") {
      sendPumpStatus();
    }

    sim808.deleteSMS(messageIndex);
  }
}

void turnOnMainPump() {
  digitalWrite(MAIN_PUMP_PIN, LOW);  // Turn ON (relay logic)
  mainPumpIsOn = true;
  mainPumpStartTime = millis();
  sim808.sendSMS(PHONE_NUMBER, MESSAGE_MAIN_ON);
}

void turnOffMainPump() {
  digitalWrite(MAIN_PUMP_PIN, HIGH);  // Turn OFF (relay logic)
  mainPumpIsOn = false;
  sim808.sendSMS(PHONE_NUMBER, MESSAGE_MAIN_OFF);
}

void turnOnDrainPump() {
  digitalWrite(DRAIN_PUMP_PIN, LOW);  // Turn ON (relay logic)
  sim808.sendSMS(PHONE_NUMBER, MESSAGE_FLOOD);
}

void turnOffDrainPump() {
  digitalWrite(DRAIN_PUMP_PIN, HIGH);  // Turn OFF (relay logic)
}

void sendPumpStatus() {
  float waterLevel = measureWaterLevel();
  String mainStatus = mainPumpIsOn ? "ON" : "OFF";
  int pumpDuration = mainPumpIsOn ? (millis() - mainPumpStartTime) / 60000 : 0;
  String message = "Main Pump: " + mainStatus + ", Level: " + String(waterLevel) + " cm, ON Time: " + String(pumpDuration) + " min";

  sim808.sendSMS(PHONE_NUMBER, message.c_str());
}

float measureWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2.0;
  return distance;
}

void checkFlood() {
  int soilValue = digitalRead(SOIL_SENSOR_PIN);
  if (soilValue == 0 && !floodDetected) {  // Flood detected and pump not yet ON
    floodDetected = true;
    turnOnDrainPump();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Flood Detected!");
    lcd.setCursor(0, 1);
    lcd.print("Drain Pump ON ");
  } else if (soilValue == 1 && floodDetected) {  // No flood and pump is ON
    floodDetected = false;
    lcd.clear();
    turnOffDrainPump();
    lcd.setCursor(0, 0);
    lcd.print("Flood Cleared  ");
    lcd.setCursor(0, 1);
    lcd.print("Drain Pump OFF");
    sim808.sendSMS(PHONE_NUMBER, "Flood cleared! Drain pump is OFF.");
  }
}

int calculateWaterLevelPercent(float currentLevel) {
    float maxLevel = 9.0; // Distance in cm corresponding to an empty tank (0%)
    float minLevel = 6.0;  // Distance in cm corresponding to a full tank (100%)
    
    // Calculate percentage based on the sensor reading
    float percent = ((maxLevel - currentLevel) / (maxLevel - minLevel)) * 100.0;

    // Ensure the percentage is within bounds
    if (percent < 0) percent = 0;     // Cap at 0% if below the minimum level
    if (percent > 100) percent = 100; // Cap at 100% if above the maximum level

    return (int)percent;
}



