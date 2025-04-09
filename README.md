# 🚀 ESP32 Ethernet OSC Trigger with Dual TFmini LiDAR

This project is designed for the **LILYGO T-Eth Lite ESP32** board and features:

- 📡 Ethernet communication using OSC over UDP
- 🔦 Dual TFmini LiDAR sensors for proximity-based triggering
- 🔧 Bluetooth Serial interface for live configuration
- 💾 Persistent settings via `Preferences.h` (IP, Ports, device ID, threshold, etc.)

---

## 📁 Project Structure

```
.
├── main.cpp              # Main application logic
├── eth_properties.h      # Ethernet and SD pin macros
├── platformio.ini        # PlatformIO build configuration
```

---

## 🧠 Features

- Dual TFmini LIDAR sensor support
- OSC message broadcasting over Ethernet
- Runtime configuration via Bluetooth Serial (IP, Ports, Threshold, deviceID)
- Persistent storage of all settings using Preferences
- Ethernet PHY configuration (RTL8201) with RMII interface
- Debug logging option
- Configurable input/output ports and distance thresholds

---

## 🛠️ Hardware Setup

| Peripheral     | ESP32 Pin Mapping |
|----------------|-------------------|
| TFmini-1       | RX: 32, TX: 13     |
| TFmini-2       | RX: 34, TX: 14     |
| Ethernet PHY   | RTL8201 (RMII)     |
| SD Card (opt.) | MISO: 34, MOSI: 13, SCLK: 14, CS: 5 |

Pin mappings are configured in [`eth_properties.h`](eth_properties.h).

---

## ⚙️ PlatformIO Configuration

**platformio.ini**
```ini
[env:T-ETH-Lite-ESP32]
platform = espressif32
board = esp32-poe
framework = arduino

board_build.flash_size = 16MB
board_build.psram = enabled
board_build.partitions = huge_app.csv

monitor_speed = 115200
upload_speed = 921600

build_flags = 
  -DBOARD_HAS_PSRAM
  -DLILYGO_T_ETH_LITE_ESP32
  -DUSER_SETUP_LOADED
```

---

## 🌐 Default Network Settings

| Setting          | Default Value          |
|------------------|------------------------|
| IP Address       | 10.255.250.150         |
| Subnet Mask      | 255.255.254.0          |
| Gateway          | 10.255.250.1           |
| Destination IP   | 10.255.250.129         |
| Input Port       | 7001                   |
| Output Port      | 7000                   |
| Threshold (cm)   | 100                    |
| Device ID        | 1                      |

---

## 🔧 Bluetooth Commands

Connect to `ESP32-ETH` using any Bluetooth terminal app (e.g., Serial Bluetooth Terminal). The following commands are available:

| Command                | Description                         |
|------------------------|-------------------------------------|
| `SET_IP <x.x.x.x>`     | Set static IP address               |
| `SET_SUBNET <x.x.x.x>` | Set subnet mask                     |
| `SET_GATEWAY <x.x.x.x>`| Set gateway IP                      |
| `SET_OUTIP <x.x.x.x>`  | Set destination IP for OSC          |
| `SET_INPORT <port>`    | Set local input port for UDP        |
| `SET_OUTPORT <port>`   | Set remote output port for OSC      |
| `SET_THRESHOLD <val>`  | Set LIDAR threshold distance (cm)   |
| `SET_ID <0-255>`       | Set device ID for OSC address       |
| `GET_CONFIG`           | Display all current config values   |
| `IP`                   | Show current Ethernet IP            |
| `MAC`                  | Show Ethernet MAC address           |

---

## 📏 LIDAR Trigger Logic

Each TFmini sensor checks whether an object is within `threshold_distance`.

If yes:
- OSC message is sent: `/composition/columns/1/connect 1`

If the object moves out:
- OSC message is sent: `/composition/columns/21/connect 1`

Both sensors are checked independently and send the same OSC address format with device ID and sensor ID.

---

## 📡 OSC Messaging

- Sent via UDP to the configured `outIp:outPort`
- OSC message format:
  ```
  Address: /composition/columns/<column>/connect
  Payload: [int] 1
  ```

---

## 🧪 Debugging

To enable debug output:
```cpp
#define DEBUG 1
```

This will show LIDAR readings and trigger states over Serial.

---

## 🔌 Setup Instructions

1. Open this project in [PlatformIO](https://platformio.org/)
2. Connect the LILYGO T-Eth Lite ESP32 via USB
3. Upload firmware to the board
4. Connect Ethernet cable
5. Use Serial Monitor @ 115200 baud to view logs
6. Use a Bluetooth terminal to configure network or device ID

---

## 🧠 Future Enhancements

- Web-based configuration page via ESPAsyncWebServer
- Save/load config presets
- OLED display for live values
- Trigger debounce and averaging filters

---

## 📜 License

This project is licensed under the MIT License. Free to use, modify, and distribute.
