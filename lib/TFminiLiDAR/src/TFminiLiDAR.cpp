#include "TFminiLiDAR.h"

TFminiLiDAR::TFminiLiDAR(uint8_t rxPin, uint8_t txPin)
    : _serial(rxPin, txPin), _distance(0), _strength(0) {}

void TFminiLiDAR::begin(long baudRate) {
    _serial.begin(baudRate); // User-defined baud rate
}

bool TFminiLiDAR::readData() {
    if (_serial.available() && _serial.read() == HEADER) {
        if (_serial.read() == HEADER) {
            int uart[9];
            uart[0] = HEADER;
            uart[1] = HEADER;

            for (int i = 2; i < 9; i++) {
                while (!_serial.available());
                uart[i] = _serial.read();
            }

            int check = 0;
            for (int i = 0; i < 8; i++) check += uart[i];
            if ((check & 0xFF) == uart[8]) {
                _distance = uart[2] + uart[3] * 256;
                _strength = uart[4] + uart[5] * 256;
                _temperature = uart[6] + uart[7] * 256;
                _firmwareVersion = uart[8];
                return true;
            }
        }
    }
    return false;
}

int TFminiLiDAR::getDistance() const {
    return _distance;
}

int TFminiLiDAR::getStrength() const {
    return _strength;
}
int TFminiLiDAR::getTemperature() const {
    return _temperature;
}
int TFminiLiDAR::getFirmwareVersion() const {
    return _firmwareVersion;
}