# Hardware Wiring Diagram 🔌

| Component | Pin Type | ESP32 Pin | Note |
| :--- | :--- | :--- | :--- |
| **DHT11 (Temp/Hum)** | Digital | GPIO 4 | |
| **MQ4 Gas Sensor** | Analog | GPIO 39 (VN) | ADC1 |
| **IR Sensor 1 (Entry)** | Digital | GPIO 25 | |
| **IR Sensor 2 (Exit)** | Digital | GPIO 26 | |
| **Room LDR 1** | Analog | GPIO 34 | ADC1 |
| **Room LDR 2** | Analog | GPIO 35 | ADC1 |
| **Solar LDR Left** | Analog | GPIO 36 (VP) | ADC1 |
| **Solar LDR Right** | Analog | GPIO 33 | ADC1 |
| **Room 1 LEDs** | Output | GPIO 13, 12 | |
| **Room 2 LEDs** | Output | GPIO 14, 27 | |
| **Door Servo** | PWM | GPIO 15 | |
| **Solar Servo** | PWM | GPIO 22 | |

### RFID (SPI Connection)
| RFID Pin | ESP32 Pin |
| :--- | :--- |
| **SDA (SS)** | GPIO 5 |
| **SCK** | GPIO 18 |
| **MOSI** | GPIO 23 |
| **MISO** | GPIO 19 |
| **RST** | GPIO 2 |
| **3.3V** | 3.3V |
| **GND** | GND |

**Important Note:** Ensure all sensors and the ESP32 share a Common Ground (GND).
