# ESP32 NFC & Keypad Access Control with Node-RED and Raspberry Pi LCD Pico-Display

This project implements an **IoT Access Control System** using an **ESP32**, **NFC reader**, and **4x4 Keypad**, with data sent over **MQTT** to **Node-RED** on a **Raspberry Pi**. The access result ("Access Granted" / "Access Denied") is then displayed on an **LCD screen** connected to the Raspberry Pi.

---

## 📦 Components Used

- ESP32-WROOM-32
- NFC Module (e.g., ELEC Freaks V3 or V1.1)
- 4x4 Keypad (Analog or digital)
- Raspberry Pi (any model with Wi-Fi)
- 2.4" or 2.8" TFT LCD (SPI)
- Node-RED
- Mosquitto MQTT Broker
- Jumper wires, breadboard

---

## ⚙️ Features

- Read NFC UID or keypad input on ESP32.
- Publish the input data via MQTT.
- Node-RED receives the data and validates it.
- Based on validation, sends response ("Access Granted" or "Access Denied") back via MQTT.
- Raspberry Pi receives response and displays it on the LCD screen.

---

## 🧭 System Architecture

```plaintext
[NFC / Keypad] --> ESP32 --> MQTT --> Node-RED (Raspberry Pi)
                                        ↓
                                 Validation Logic
                                        ↓
                               MQTT Response ("Granted"/"Denied")
                                        ↓
                           Raspberry Pi --> LCD Display
