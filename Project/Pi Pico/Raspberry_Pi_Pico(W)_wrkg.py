import network
import time
from simple import MQTTClient
from machine import Pin, SPI
import ili9341

# WiFi Credentials
WIFI_SSID = "moto g(30)_2775"
WIFI_PASSWORD = "12345677"

# MQTT Broker Details
MQTT_BROKER = "192.168.109.241"  # Change this to your Node-RED broker IP
MQTT_TOPIC = "pico/display"

# Initialize SPI0 for LCD (2.8" TFT 240x320)
spi = SPI(0, baudrate=40000000, sck=Pin(18), mosi=Pin(19))
tft = ili9341.ILI9341(
    spi, 
    cs=Pin(17), 
    dc=Pin(15), 
    rst=Pin(16), 
    w=320,  
    h=320,  
    r=0  # Rotation (0, 90, 180, or 270)
)

# Message storage
messages = []  
MAX_MESSAGES = 10  # Max number of messages to display at once
LINE_HEIGHT = 30  # Space between lines

# Function to connect WiFi
def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(WIFI_SSID, WIFI_PASSWORD)

    print("Connecting to WiFi...")
    while not wlan.isconnected():
        time.sleep(1)
    print("Connected to WiFi:", wlan.ifconfig())

# Function to handle received MQTT messages
def on_message(topic, msg):
    global messages

    text = msg.decode()
    print("Received:", text)

    # Insert new message at the top
    messages.insert(0, text)

    # Limit the number of messages to fit the screen
    if len(messages) > MAX_MESSAGES:
        messages.pop()  # Remove the oldest message at the bottom

    # Refresh the screen
    draw_screen()

# Function to draw the background
def draw_background():
    tft.fill_rectangle(0, 0, 240, 320, ili9341.color565(10, 10, 50))  # Dark blue background

# Function to draw messages on screen
def draw_screen():
    draw_background()
    y_offset = 10  # Start drawing from the top

    for message in messages:
        tft.chars(message, 10, y_offset)
        y_offset += LINE_HEIGHT  # Move to the next line

# Connect to MQTT Broker
def connect_mqtt():
    client = MQTTClient("pico_client", MQTT_BROKER)
    client.set_callback(on_message)
    client.connect()
    client.subscribe(MQTT_TOPIC)
    print("Connected to MQTT Broker, Subscribed to:", MQTT_TOPIC)
    return client

# Main Function
def main():
    connect_wifi()
    client = connect_mqtt()

    draw_background()
    tft.chars("Waiting for data...", 10, 10)

    while True:
        client.check_msg()  # Check for new MQTT messages
        time.sleep(0.1)

# Run the main function
main()
 