# SENTINEL FIREWALL: Automated Home Design 🏡

**Team UltraPulse** presents a comprehensive IoT-based Smart Home Security and Automation system. This project integrates multiple hardware modules into a unified, responsive dark-mode web dashboard hosted locally on an ESP32.

> "Secured by design: From code to comfort !!"

## 🚀 Core Features
* **RFID Door Access:** Secure entry using MFRC522 RFID reader and Servo-controlled door mechanism.
* **Automatic Solar Tracking:** Dual-LDR logic that rotates a solar panel towards the strongest light source for maximum efficiency.
* **Smart Entry/Exit Counter:** IR-based sequential logic to track the number of people currently inside the house.
* **Intelligent Lighting:** Room-specific LDR sensors that automatically trigger LED lighting based on ambient brightness.
* **Environmental Monitoring:** Real-time tracking of Gas levels (MQ4), Temperature, and Humidity (DHT11).
* **Live Web Dashboard:** A professional, dark-themed interactive interface to monitor all systems from any device on the network.

## 🛠️ Hardware Components
* **Microcontroller:** ESP32 (DevKit V1)
* **Sensors:** DHT11, MQ4 Gas Sensor, IR Sensors (x2), LDRs (x4)
* **Actuators:** MG90S Servos (x2), LEDs (x4)
* **Security:** MFRC522 RFID Module
* **Display:** Local Web Server (AsyncWebServer)

## 📡 Web Dashboard
The dashboard provides real-time JSON updates every 400ms, featuring:
* Live Temperature & Humidity gauges.
* Gas concentration alerts.
* Occupancy counter.
* Dynamic "System Log" mirroring the Serial Monitor.
* ON/OFF status indicators for room lighting.

## 👥 Team UltraPulse
* **Anurag Verma**
* **Anvi Sharma**
* **Malika Parveen**
* **Trisha Pandey**

---
Built with ❤️ by Team UltraPulse.
