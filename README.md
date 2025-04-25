# Smart-Traffic-Light-System
An intelligent traffic control system using ESP32, ESP8266, and ESP32-CAM to optimize green light durations based on real-time vehicle density. The system uses a Flask-based machine learning server to analyze images from traffic lanes and coordinates multiple microcontrollers to manage traffic lights and countdown displays for up to four lanes.
🔧 Features
Real-time image capture using ESP32-CAM for vehicle detection

Flask server with ML model to calculate dynamic green light durations

Master-slave communication between ESP32 and ESP8266 via UART

OLED displays show live countdown timers for each lane

Automatic traffic light control with feedback acknowledgment system

🧰 Tech Stack
Microcontrollers: ESP32, ESP8266

Communication: UART, I²C

Displays: 0.96" I2C OLED

Backend: Python, Flask, OpenCV

IDE/Tools: Arduino IDE, PlatformIO, Git
