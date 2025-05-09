#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_PN532.h>

// Wi-Fi credentials
const char* ssid = "moto g(30)_2775";
const char* password = "12345677";

// MQTT Broker details
const char* mqtt_server = "192.168.109.241";  // Use the Raspberry Pi's IP address
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32/data";

WiFiClient espClient;
PubSubClient client(espClient);

// Define I2C pins
#define SDA_PIN 18  // GPIO 18
#define SCL_PIN 19  // GPIO 19
#define IRQ_PIN 23  // GPIO 23 (Interrupt Pin for PN532)
#define RESET_PIN 5 // GPIO 5 (Reset Pin for PN532)

// Keypad TTP229
#define KEYPAD_SCL 27
#define KEYPAD_SDO 26
String keypadInput = "";
const int maxKeypadDigits = 5;

// Initialize NFC module
Adafruit_PN532 nfc(IRQ_PIN, RESET_PIN);

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize I2C
    Wire.begin(SDA_PIN, SCL_PIN);
    
    // Connect to Wi-Fi
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected!");

    // Connect to MQTT Broker
    Serial.println("Connecting to MQTT Broker...");
    client.setServer(mqtt_server, mqtt_port);
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client")) {
            Serial.println("Connected to MQTT Broker!");
        } else {
            Serial.print("MQTT connection failed. Error: ");
            Serial.println(client.state());
            delay(1000);
        }
    }

    // Initialize NFC module
    Serial.println("Initializing NFC module...");
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        Serial.println("ERROR: PN532 not detected! Check wiring.");
        while (1);
    }
    Serial.println("PN532 NFC module found!");
    nfc.SAMConfig();
    Serial.println("Scan an NFC card...");

    // Initialize keypad
    pinMode(KEYPAD_SCL, OUTPUT);
    pinMode(KEYPAD_SDO, INPUT);
}

void loop() {
    readNFC();
    readKeypad();
    delay(200);
}

void readNFC() {
    uint8_t uid[7];  // Buffer to store UID
    uint8_t uidLength;

    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        Serial.print("NFC Tag UID: ");
        String tagData = "NFC:";
        for (uint8_t i = 0; i < uidLength; i++) {
            Serial.print(uid[i], HEX);
            Serial.print(" ");
            tagData += String(uid[i], HEX);
        }
        Serial.println();
        sendData(tagData);
    }
}

void readKeypad() {
    digitalWrite(KEYPAD_SCL, LOW);
    delayMicroseconds(2);
    digitalWrite(KEYPAD_SCL, HIGH);
    delayMicroseconds(2);
    
    int key = 0;
    for (int i = 0; i < 16; i++) {
        if (!digitalRead(KEYPAD_SDO)) {
            key = i + 1;
            break;
        }
        digitalWrite(KEYPAD_SCL, LOW);
        delayMicroseconds(2);
        digitalWrite(KEYPAD_SCL, HIGH);
        delayMicroseconds(2);
    }
    
    if (key > 0) {
        Serial.print("Key Pressed: ");
        Serial.println(key);
        keypadInput += String(key);
        if (keypadInput.length() >= maxKeypadDigits) {
            sendData("Keypad:" + keypadInput);
            keypadInput = "";
        }
    }
}

void sendData(String data) {
    if (client.connected()) {
        Serial.println("Sending data to MQTT...");
        client.publish(mqtt_topic, data.c_str());
        Serial.println("Data Sent: " + data);
    } else {
        Serial.println("MQTT Disconnected! Reconnecting...");
        reconnectMQTT();
    }
}

void reconnectMQTT() {
    while (!client.connected()) {
        Serial.print("Attempting to reconnect to MQTT...");
        if (client.connect("ESP32Client")) {
            Serial.println("Reconnected to MQTT Broker!");
        } else {
            Serial.print("MQTT reconnection failed. Error: ");
            Serial.println(client.state());
            delay(1000);
        }
    }
}
