import network
import time

SSID = "moto g(30)_2775"
PASSWORD = "12345677"

wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(SSID, PASSWORD)

print("Connecting to WiFi...")

timeout = 10  # 10 seconds timeout
while not wlan.isconnected() and timeout > 0:
    time.sleep(1)
    timeout -= 1
    print("Trying to connect...")

if wlan.isconnected():
    print("Connected to WiFi! IP Address:", wlan.ifconfig()[0])
else:
    print("Failed to connect. Check WiFi credentials and signal strength.")
