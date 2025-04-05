#include <TFminiLiDAR.h>

TFminiLiDAR tfmini(25, 26); // RX, TX

void setup() {
  Serial.begin(115200);
  tfmini.begin(115200); // 👈 Set baud rate here based on your TFmini config
}

void loop() {
  if (tfmini.readData()) {
    Serial.print("Distance: ");
    Serial.print(tfmini.getDistance());
    Serial.print(" cm\tStrength: ");
    Serial.print(tfmini.getStrength());
    Serial.print("\tTemperature: ");
    Serial.print(tfmini.getTemperature());
    Serial.print(" °C\tFirmware Version: ");
    Serial.println(tfmini.getFirmwareVersion());
  }
}
