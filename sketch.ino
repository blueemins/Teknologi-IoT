#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Fuzzy.h> // Library Fuzzy Logic

// WiFi Configuration
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ThingsBoard Configuration
const char* MQTT_SERVER = "demo.thingsboard.io"; // Replace with your server
const char* TOKEN = "YOUR_ACCESS_TOKEN"; // Replace with your ThingsBoard token

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Pin Configuration
#define DHTPIN 14       // DHT22 pin
#define DHTTYPE DHT22   // DHT 22 (AM2302)
const int trigPin1 = 5;  // Ultrasonic Trigger Pin 1
const int echoPin1 = 18; // Ultrasonic Echo Pin 1
const int trigPin2 = 25; // Ultrasonic Trigger Pin 2
const int echoPin2 = 26; // Ultrasonic Echo Pin 2
const int ledRed = 2;    // Red LED
const int ledGreen = 4;  // Green LED
const int relayPin1 = 32; // Relay pin 1
const int relayPin2 = 33; // Relay pin 2
const int relayPin3 = 27; // Relay pin 3
const int servoPin = 23;  // Servo motor pin
const int turbidityPin = 34; // Turbidity sensor pin
const int potPin = 35;    // Potentiometer pin

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensors
Servo myservo;
DHT dht(DHTPIN, DHTTYPE);

// Fuzzy Logic Variables
Fuzzy* fuzzy = new Fuzzy();
float fuzzyOutput = 0;

// Function Prototypes
void setupWiFi();
void reconnect();
void sendDataToThingsBoard();
void setupFuzzyLogic();

void setup() {
  Serial.begin(115200);

  // Pin Modes
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);

  myservo.attach(servoPin);

  // Start WiFi and MQTT
  setupWiFi();
  client.setServer(THINGSBOARD_SERVER, 1883);

  // Initialize DHT Sensor
  dht.begin();

  // Initialize OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Failed to initialize OLED"));
    while (true);
  }
  oled.clearDisplay();
  oled.display();

  // Initialize Fuzzy Logic
  setupFuzzyLogic();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sendDataToThingsBoard();
  delay(2000); // Delay between readings
}

void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    if (client.connect("ESP32", TOKEN, NULL)) {
      Serial.println(" Connected!");
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setupFuzzyLogic() {
  // Input: Water Level
  FuzzyInput* waterLevel = new FuzzyInput(1);
  waterLevel->addFuzzySet(new FuzzySet("Low", 0, 10, 10, 20));
  waterLevel->addFuzzySet(new FuzzySet("Medium", 10, 20, 20, 30));
  waterLevel->addFuzzySet(new FuzzySet("High", 20, 30, 30, 40));
  fuzzy->addFuzzyInput(waterLevel);

  // Input: Turbidity
  FuzzyInput* turbidity = new FuzzyInput(2);
  turbidity->addFuzzySet(new FuzzySet("Clear", 0, 30, 30, 60));
  turbidity->addFuzzySet(new FuzzySet("Cloudy", 30, 60, 60, 90));
  turbidity->addFuzzySet(new FuzzySet("Dirty", 60, 90, 90, 100));
  fuzzy->addFuzzyInput(turbidity);

  // Output: Control Action
  FuzzyOutput* action = new FuzzyOutput(1);
  action->addFuzzySet(new FuzzySet("Low", 0, 10, 10, 20));
  action->addFuzzySet(new FuzzySet("Medium", 10, 20, 20, 30));
  action->addFuzzySet(new FuzzySet("High", 20, 30, 30, 40));
  fuzzy->addFuzzyOutput(action);

  // Rule Base
  fuzzy->addFuzzyRule(new FuzzyRule(1, new FuzzyRuleAntecedent("WaterLevel IS Low AND Turbidity IS Clear"), new FuzzyRuleConsequent("Action IS Low")));
  fuzzy->addFuzzyRule(new FuzzyRule(2, new FuzzyRuleAntecedent("WaterLevel IS High AND Turbidity IS Dirty"), new FuzzyRuleConsequent("Action IS High")));
}

void sendDataToThingsBoard() {
  // Turbidity Reading
  int turbidityValue = analogRead(turbidityPin);
  int turbidityPercentage = map(turbidityValue, 0, 4095, 0, 100);
  Serial.print("Turbidity: ");
  Serial.println(turbidityPercentage);

  // Ultrasonic Reading 1
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  float duration1 = pulseIn(echoPin1, HIGH);
  float distance1 = duration1 * 0.034 / 2;
  Serial.print("Water Level 1: ");
  Serial.println(distance1);

  // Ultrasonic Reading 2
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  float duration2 = pulseIn(echoPin2, HIGH);
  float distance2 = duration2 * 0.034 / 2;
  Serial.print("Water Level 2: ");
  Serial.println(distance2);

  // DHT Sensor Readings
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);

  // Apply Fuzzy Logic
  fuzzy->setInput(1, distance1);
  fuzzy->setInput(2, turbidityPercentage);
  fuzzy->fuzzify();
  fuzzyOutput = fuzzy->defuzzify(1);

  // Control Logic
  if (fuzzyOutput > 20) {
    digitalWrite(relayPin1, HIGH);
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
  } else {
    digitalWrite(relayPin1, LOW);
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
  }

  // Publish to ThingsBoard
  char telemetry[256];
  snprintf(telemetry, sizeof(telemetry), "{\"turbidity\":%d,\"water_level1\":%.2f,\"water_level2\":%.2f,\"temperature\":%.2f,\"humidity\":%.2f}", turbidityPercentage, distance1, distance2, temperature, humidity);
  client.publish("v1/devices/me/telemetry", telemetry);
}
