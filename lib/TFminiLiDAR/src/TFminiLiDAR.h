#ifndef TFMINILIDAR_H
#define TFMINILIDAR_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class TFminiLiDAR {
public:
    TFminiLiDAR(uint8_t rxPin, uint8_t txPin);
    void begin(long baudRate = 115200); // Allow setting baud rate in sketch
    bool readData();
    int getDistance() const;
    int getStrength() const;
    int getTemperature() const;
    int getFirmwareVersion() const;


private:
    SoftwareSerial _serial;
    int _distance;
    int _strength;
    int _temperature;
    int _firmwareVersion;
    
    const int HEADER = 0x59;
};

#endif
