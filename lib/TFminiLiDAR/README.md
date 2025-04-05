# TFminiLiDAR Arduino Library

A lightweight library for interfacing the TFmini LiDAR sensor via UART using SoftwareSerial.

## Features
- Reads distance and signal strength from the TFmini sensor.
- Designed for easy integration in Arduino projects.

## Usage

```cpp
#include <TFminiLiDAR.h>

TFminiLiDAR lidar(2, 3);

void setup() {
  Serial.begin(9600);
  lidar.begin(115200);
}

void loop() {
  if (lidar.readData()) {
    Serial.print("Distance: ");
    Serial.print(lidar.getDistance());
    Serial.print(" cm\tStrength: ");
    Serial.print(tfmini.getStrength());
    Serial.print("\tTemperature: ");
    Serial.print(tfmini.getTemperature());
    Serial.print(" °C\tFirmware Version: ");
    Serial.println(tfmini.getFirmwareVersion());
  }
}
```

## License
MIT
