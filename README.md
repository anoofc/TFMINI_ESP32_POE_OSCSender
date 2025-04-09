# 🚀 ESP32 Ethernet OSC Trigger with Dual TFmini LiDAR

This project is built for the **LILYGO T-Eth Lite ESP32** board and integrates:

- 📡 Ethernet communication (OSC over UDP)
- 🔦 Dual TFmini LiDAR sensors for distance-based triggering
- 🔧 Bluetooth Serial interface for dynamic network configuration
- 💾 Persistent IP settings using `Preferences.h` (replacing EEPROM)

---

## 📁 Project Structure

```
.
├── main.cpp              # Main application logic
├── eth_properties.h      # Ethernet and SD pin definitions
├── platformio.ini        # PlatformIO build config
```

---

## 🧠 Features

- ✅ Dual TFmini LIDAR sensor support
- ✅ OSC message sending over Ethernet (UDP)
- ✅ Bluetooth Serial configuration (SET/GET IP)
- ✅ Persistent network settings using Preferences
- ✅ Debug logging via Serial Monitor
- ✅ 16MB Flash + 8MB PSRAM enabled via PlatformIO

---

## 🛠️ Hardware Connections

| Component     | Pin                          |
|---------------|------------------------------|
| TFmini 1      | RX: 32, TX: 13               |
| TFmini 2      | RX: 34, TX: 14               |
| Ethernet PHY  | RTL8201 via RMII             |
| SD Card       | MISO: 34, MOSI: 13, SCLK: 14, CS: 5 |

All these pin mappings are defined in [`eth_properties.h`](eth_properties.h).

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

## 🌐 Default Network Configuration

| Parameter      | Value               |
|----------------|---------------------|
| IP Address     | `10.255.250.150`    |
| Subnet         | `255.255.254.0`     |
| Gateway        | `10.255.250.1`      |
| Destination IP | `10.255.250.129`    |
| UDP In Port    | `7001`              |
| UDP Out Port   | `7000`              |

These values are **saved to flash memory** using `Preferences.h` and persist across reboots.

---

## 🔧 Bluetooth Commands

Connect to `ESP32-ETH` via any Bluetooth serial terminal (e.g., Serial Bluetooth Terminal app). You can send the following commands:

| Command            | Description                    |
|--------------------|--------------------------------|
| `SET_IP <ip>`      | Set device IP address          |
| `SET_SUBNET <ip>`  | Set subnet mask                |
| `SET_GATEWAY <ip>` | Set gateway IP                 |
| `SET_OUTIP <ip>`   | Set destination IP             |
| `GET_CONFIG`       | Print current IP configuration |
| `i`                | Print current ETH IP           |
| `m`                | Print MAC address              |

---

## 📏 Trigger Logic

Both LIDAR sensors check for objects within a `THRESHOLD_DISTANCE` (default: `100 cm`).

If an object is detected:
- A trigger OSC message is sent with value `1`
- When object moves out of range, a reset message (`0`) is sent

**OSC Address Format:**
```
/device<ID>/  [value: 1 or 0]
```

> `deviceID` is defined in the code (default: `1`).

---

## 📡 OSC Messaging

- Protocol: UDP
- Destination: Configurable via Bluetooth
- Format: `/device1/` with integer payload (1 or 0)

---

## 🧪 Debug Mode

To enable verbose output:
```cpp
#define DEBUG 1
```

This will print:
- Sensor distance readings
- Trigger/reset actions
- IP/MAC info when `i` or `m` is received via serial

---

## 🧵 Setup Instructions

1. Clone the repo and open it in [PlatformIO](https://platformio.org/)
2. Connect the LILYGO T-Eth Lite ESP32 via USB
3. Build & upload the firmware
4. Open Serial Monitor @ `115200 baud`
5. Power the board with Ethernet cable connected
6. Configure IP via Bluetooth (optional)

---

## 🧠 Future Ideas

- Add web-based config interface via AsyncWebServer
- Integrate OLED/LED feedback for trigger state
- Multi-device OSC broadcasting with unique IDs

---

## 📜 License

This project is open-source and free to use under MIT License.
