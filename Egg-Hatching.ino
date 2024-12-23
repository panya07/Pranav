#define BLYNK_TEMPLATE_ID "TMPL37-zQNbaD"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "DpySLqlpVo7TfnsOfGE-iMncPw9qLpsn"

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Define GPIO pin numbers
#define DHTPIN 13            // GPIO14 (D7 on NodeMCU)
#define DHTTYPE DHT11
#define MOTION_SENSOR_PIN 2  // GPIO12 (D4 on NodeMCU)
#define HUMIDIFIER_PIN 14    // GPIO13 (D5 on NodeMCU)
#define RELAY_PIN 0          // Use A0 pin for relay control

#define VPIN_RELAY_CONTROL V1

// Initialize components
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_PCF8574 lcd(0x27);

BlynkTimer timer;

bool manualControl = false;  // Flag for manual override
int relayState = LOW;        // Relay state

// Blynk control: Manual override
BLYNK_WRITE(VPIN_RELAY_CONTROL) {
  relayState = param.asInt();     // Read Blynk virtual pin value
  manualControl = true;           // Enable manual control
  digitalWrite(RELAY_PIN, relayState);
  Serial.print("Manual Mode: ");
  Serial.println(relayState == HIGH ? "ON" : "OFF");
}

// Blynk credentials
char auth[] = "DpySLqlpVo7TfnsOfGE-iMncPw9qLpsn";
char ssid[] = "1234";
char pass[] = "12345678";

// Variables
float temp = 0.0;
float humidity = 0.0;
bool motionDetected = false;

void setup() {
  Serial.begin(115200);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.print("Egg Hatcher");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);

  dht.begin();
  pinMode(MOTION_SENSOR_PIN, INPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);  // Set A0 as output for relay

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  if (WiFi.status() == WL_CONNECTED) Serial.println("\nWiFi Connected.");
  
  Blynk.config(auth);
  Serial.println("Blynk Initialized");

  lcd.clear();
  lcd.print("System Ready");
  delay(2000);

  // Periodic tasks
  timer.setInterval(2000L, readSensors); // Read sensors every 2 seconds
  timer.setInterval(1000L, updateLCD);   // Update LCD every 1 second
}

// Function to read sensors
void readSensors() {
  temp = dht.readTemperature();
  humidity = dht.readHumidity();
  motionDetected = digitalRead(MOTION_SENSOR_PIN);

  // Debug print sensor data
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C | Hum: ");
  Serial.print(humidity);
  Serial.print(" % | Motion: ");
  Serial.println(motionDetected ? "Yes" : "No");

  // Control humidifier
  digitalWrite(HUMIDIFIER_PIN, humidity > 60.0 ? HIGH : LOW);

  // Automatic control for relay based on temperature
  if (!manualControl) { // Only apply automatic control if manual mode is off
    if (temp > 37.5) {
      relayState = HIGH;
    } else {
      relayState = LOW;
    }
    digitalWrite(RELAY_PIN, relayState);
  }

  // Update Blynk virtual pins
  Blynk.virtualWrite(V4, temp);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V0, motionDetected);
  Blynk.virtualWrite(V1, relayState); // Update relay state on Blynk
}

// Function to update LCD
void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Temp | Hum |Born");

  lcd.setCursor(0, 1);
  if (!isnan(temp)) lcd.print(temp, 1); else lcd.print("Err");
  lcd.print("C|");
  if (!isnan(humidity)) lcd.print(humidity, 1); else lcd.print("Err");
  lcd.print("%|");
  lcd.print(motionDetected ? "Yes" : "No ");
}

void loop() {
  Blynk.run();
  timer.run();
}
