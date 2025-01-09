# IoT_Project

An Internet of Things (IoT) project designed to enhance parking safety, especially for older vehicles. This system provides real-time distance measurement, audio and visual feedback, and cloud-based data logging using an Arduino Uno R4 WiFi and ThingSpeak.

## Features
- **Real-Time Feedback**: Measures distance using an HC-SR04 ultrasonic sensor and displays it on a 16x2 LCD.
- **Audio Alerts**: Provides audible feedback with a buzzer based on proximity to obstacles.
- **Cloud Integration**: Logs parking session data (minimum, maximum, and parked distances) to ThingSpeak.
- **Web Interface**: Hosts a simple web server for live distance monitoring.
- **Data Analytics**: Analyzes parking patterns and provides insights using ThingSpeak's MATLAB tools.

## Components
- Arduino Uno R4 WiFi
- HC-SR04 Ultrasonic Sensor
- 16x2 LCD Screen (Non-I2C)
- Potentiometer
- Buzzer
- 2kΩ Resistor

## Getting Started
1. **Clone this repository**:
   ```bash
   git clone https://github.com/YourUsername/IoT-Parking-Sensor.git