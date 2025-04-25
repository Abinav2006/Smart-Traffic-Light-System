#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define SDA_1 21
#define SCL_1 22

const char* ssid = "FbiSurveillanceVan";
const char* password = "fbiopenup";
const char* server = "192.168.158.186";

int redPins[4]    = {4, 14, 18, 26};
int yellowPins[4] = {5, 27, 19, 33};
int greenPins[4]  = {16, 25, 23, 32};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int countdownTime = 0;
bool timerRunning = false;
int currentLane = 0;

unsigned long prevMillis = 0;
const long interval = 1000;

void setup() {
  Serial.begin(115200);
  

  Wire.begin(SDA_1, SCL_1);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    while (true);
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  for (int i = 0; i < 4; i++) {
    pinMode(redPins[i], OUTPUT);
    pinMode(yellowPins[i], OUTPUT);
    pinMode(greenPins[i], OUTPUT);
    digitalWrite(redPins[i], HIGH);
    digitalWrite(yellowPins[i], LOW);
    digitalWrite(greenPins[i], LOW);
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (timerRunning) {
    if (currentMillis - prevMillis >= interval) {
      prevMillis = currentMillis;

      if (countdownTime > 0) {
        countdownTime--;
        
        // Update display only for lane 1
        if (currentLane == 1) {
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(30, 20);
          display.print("Lane 1:");
          display.setCursor(50, 40);
          display.print(countdownTime);
          display.display();
        }
      } else {
        // Countdown finished for current lane
        endCurrentLane();
      }
    }
  } else {
    // No timer running, fetch new data
    fetchAndStartTimer();
  }
}

void endCurrentLane() {
  timerRunning = false;
  // Turn current lane back to red
  digitalWrite(redPins[currentLane - 1], HIGH);
  digitalWrite(greenPins[currentLane - 1], LOW);
  digitalWrite(yellowPins[currentLane - 1], LOW);
  
  // Clear display if it was showing lane 1
  if (currentLane == 1) {
    display.clearDisplay();
    display.display();
  }
}

void fetchAndStartTimer() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;

    Serial.println("Connecting to server...");
    if (client.connect(server, 5000)) {
      client.println("GET /get_data HTTP/1.1");
      client.println("Host: " + String(server));
      client.println("Connection: close");
      client.println();

      while (client.connected() && !client.available()) delay(10);
      while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") break;
      }

      String payload = client.readString();
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        int lane = doc["lane"];
        int duration = doc["value"];
        currentLane = lane;

        Serial.printf("Starting Lane %d | Duration: %d\n", lane, duration);

        // Set all lanes to red
        for (int i = 0; i < 4; i++) {
          digitalWrite(redPins[i], HIGH);
          digitalWrite(greenPins[i], LOW);
          digitalWrite(yellowPins[i], LOW);
        }

        // Yellow blink
        for (int i = 0; i < 4; i++) {
          digitalWrite(yellowPins[lane - 1], HIGH);
          delay(250);
          digitalWrite(yellowPins[lane - 1], LOW);
          delay(250);
        }

        digitalWrite(redPins[lane - 1], LOW);
        digitalWrite(greenPins[lane - 1], HIGH);

        // Send lane and duration to ESP8266
        String sendData = String(lane) + "," + String(duration) + "\n";
        Serial1.print(sendData);

        countdownTime = duration;
        timerRunning = true;
        prevMillis = millis(); // Reset the timer

      } else {
        Serial.print("JSON Parse Error: ");
        Serial.println(error.c_str());
      }

      client.stop();
    } else {
      Serial.println("Failed to connect to Flask server");
      delay(5000); // Wait before retrying
    }
  } else {
    Serial.println("WiFi not connected!");
    // Attempt to reconnect to WiFi
    WiFi.begin(ssid, password);
    delay(5000); // Wait before retrying
  }
}
