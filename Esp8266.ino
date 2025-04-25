#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "FbiSurveillanceVan";
const char* password = "fbiopenup";
const char* server = "192.168.158.186";

int countdownTime = 0;
int currentLane = 0;
bool timerRunning = false;

unsigned long prevMillis = 0;
const long interval = 1000;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I2C Pin Definitions for ESP8266 (adjust as needed for ESP32)
#define SDA_1 4  //d2
#define SCL_1 5  //d1
#define SDA_2 0  //d3
#define SCL_2 2  //d4
#define SDA_3 14  //d5
#define SCL_3 12  //d6

// Countdown values
const int initialCountdown1 = 0;
const int initialCountdown2 = 0;
const int initialCountdown3 = 0;

int countdownTime1 = initialCountdown1;
int countdownTime2 = initialCountdown2;
int countdownTime3 = initialCountdown3;

// Timer tracking
unsigned long prevMillis1 = 0, prevMillis2 = 0, prevMillis3 = 0;

void setup() {
  Serial.begin(19200);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP:");
  Serial.println(WiFi.localIP());

  Wire.begin(SDA_1, SCL_1);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED 1 not found!");
    while (1);
  }

  Wire.begin(SDA_2, SCL_2);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED 2 not found!");
    while (1);
  }

  Wire.begin(SDA_3, SCL_3);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED 3 not found!");
    while (1);
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

        if (lane >= 2 && lane <= 4) {  // Only accept lanes 2-4
          currentLane = lane;
          countdownTime = duration;
          timerRunning = true;
          Serial.printf("Lane %d countdown started for %d seconds\n", lane, duration);
        } else {
          Serial.println("Not my lane, waiting...");
        }
      } else {
        Serial.print("JSON Parse Error: ");
        Serial.println(error.c_str());
      }
      unsigned long currentMillis = millis();


      client.stop();
    } else {
      Serial.println("Server connection failed");
    }
  } else {
    Serial.println("WiFi disconnected");
  }
}

void loop() {
  if (!timerRunning) {
    fetchAndStartTimer();
  }

  if (timerRunning) {
    unsigned long currentMillis = millis();
    if (currentMillis - prevMillis >= interval) {
      prevMillis = currentMillis;
      countdownTime--;

      // Print countdown to serial for debugging
      Serial.printf("Lane %d: %d seconds remaining\n", currentLane, countdownTime);

      if (countdownTime <= 0) {
        Serial.println("Timer finished.");
        timerRunning = false;
        Serial.println("DONE");
      }
    }
  }
  if(currentLane==2){
        countdownTime1 = countdownTime;
        unsigned long currentMillis = millis();
        if (currentMillis - prevMillis1 >= interval) {
          prevMillis1 = currentMillis;
          countdownTime1 = (countdownTime1 > 0) ? countdownTime1 - 1 : initialCountdown1;

          Wire.begin(SDA_1, SCL_1);
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(30, 20);
          display.print("Lane 2:");
          display.setCursor(50, 40);
          display.print(countdownTime1);
          display.display();
        }
      }
      
      if(currentLane==3){
        countdownTime2 = countdownTime;
        unsigned long currentMillis = millis();

        if (currentMillis - prevMillis2 >= interval) {
          prevMillis2 = currentMillis;
          countdownTime2 = (countdownTime2 > 0) ? countdownTime2 - 1 : initialCountdown2;

          Wire.begin(SDA_2, SCL_2);
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(30, 20);
          display.print("Lane 3:");
          display.setCursor(50, 40);
          display.print(countdownTime2);
          display.display();
        }
      }
      

      if(currentLane==4){
        countdownTime3 = countdownTime;
        unsigned long currentMillis = millis();

        if (currentMillis - prevMillis3 >= interval) {
          prevMillis3 = currentMillis;
          countdownTime3 = (countdownTime3 > 0) ? countdownTime3 - 1 : initialCountdown3;

          Wire.begin(SDA_3, SCL_3);
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(30, 20);
          display.print("Lane 4:");
          display.setCursor(50, 40);
          display.print(countdownTime3);
          display.display();
        }
      }
}

