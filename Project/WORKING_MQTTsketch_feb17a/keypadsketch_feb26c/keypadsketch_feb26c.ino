#include <WiFi.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* ssid = "moto g(30)_2775";
const char* password = "12345677";

// MQTT Broker details
const char* mqtt_server = "192.168.109.241";  // Raspberry Pi IP
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32/keypad";

WiFiClient espClient;
PubSubClient client(espClient);

// Keypad TTP229 Pins
#define KEYPAD_SCL 27  // Clock pin
#define KEYPAD_SDO 26  // Data pin

String keypadInput = "";
const int maxKeypadDigits = 5;

void setup() {
    Serial.begin(115200);
    delay(1000);

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

    // Initialize keypad
    pinMode(KEYPAD_SCL, OUTPUT);
    pinMode(KEYPAD_SDO, INPUT_PULLUP);

    // Enable 16-key mode using software method
    configureKeypad();
}

void loop() {
    readKeypad();
    delay(200);
}

// **Software method to enable 16-key mode**
void configureKeypad() {
    Serial.println("Configuring TTP229 for 16-key mode...");

    // Reset the keypad
    digitalWrite(KEYPAD_SCL, LOW);
    delay(50);
    digitalWrite(KEYPAD_SCL, HIGH);
    delay(50);

    // Send 8 additional clock pulses to enable 16-key mode
    for (int i = 0; i < 9; i++) {
        digitalWrite(KEYPAD_SCL, HIGH);
        delayMicroseconds(100);
        digitalWrite(KEYPAD_SCL, LOW);
        delayMicroseconds(100);
    }

    Serial.println("TTP229 set to 16-key mode!");
}

void readKeypad() {
    uint16_t keys = 0;

    // Read 16 bits from the keypad
    for (int i = 0; i < 16; i++) {
        digitalWrite(KEYPAD_SCL, LOW);
        delayMicroseconds(10);
        digitalWrite(KEYPAD_SCL, HIGH);
        delayMicroseconds(10);

        if (digitalRead(KEYPAD_SDO) == LOW) {
            keys |= (1 << i);
        }
    }

    // Identify the pressed key and map 10 to 0
    for (int i = 0; i < 16; i++) {
        if (keys & (1 << i)) {
            String keyPressed;
            if (i + 1 == 10) {
                keyPressed = "0";  // Convert 10 to 0
            } else {
                keyPressed = String(i + 1);  // Normal keys 1-16
            }

            Serial.print("Key Pressed: ");
            Serial.println(keyPressed);
            keypadInput += keyPressed;

            // Send data after collecting `maxKeypadDigits`
            if (keypadInput.length() >= maxKeypadDigits) {
                sendData("Keypad:" + keypadInput);
                keypadInput = "";
            }
            break;  // Only register the first detected key
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
