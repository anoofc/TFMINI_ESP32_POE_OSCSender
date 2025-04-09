#define DEBUG              0 // Enable debug messages

#define deviceID           1
#define THRESHOLD_DISTANCE 100 // Threshold distance in cm
#define DELAY              10 // Delay between readings in milliseconds

#include <Arduino.h>
#include <TFminiLiDAR.h>
#include <OSCMessage.h>
#include <ETH.h>
#include <WiFiUdp.h>
#include <BluetoothSerial.h>
#include <Preferences.h>
#include "eth_properties.h"

BluetoothSerial SerialBT;
Preferences preferences;

TFminiLiDAR tfMini_1(32, 13); // RX, TX
TFminiLiDAR tfMini_2(34, 14); // RX, TX

WiFiUDP Udp;


IPAddress ip, subnet, gateway, outIp;
const unsigned int inPort = 7001;
const unsigned int outPort = 7000;

const char* namespaceName = "network";

void saveIPAddress(const char* keyPrefix, IPAddress address) {
  for (int i = 0; i < 4; i++) {
    String key = String(keyPrefix) + i;
    preferences.putUInt(key.c_str(), address[i]);
  }
}

IPAddress loadIPAddress(const char* keyPrefix, IPAddress defaultIP) {
  IPAddress result;
  for (int i = 0; i < 4; i++) {
    String key = String(keyPrefix) + i;
    result[i] = preferences.getUInt(key.c_str(), defaultIP[i]);
  }
  return result;
}

void saveNetworkConfig() {
  preferences.begin(namespaceName, false);
  saveIPAddress("ip", ip);
  saveIPAddress("sub", subnet);
  saveIPAddress("gw", gateway);
  saveIPAddress("out", outIp);
  preferences.end();
}

void loadNetworkConfig() {
  preferences.begin(namespaceName, true);
  ip = loadIPAddress("ip", IPAddress(10, 255, 250, 150));
  subnet = loadIPAddress("sub", IPAddress(255, 255, 254, 0));
  gateway = loadIPAddress("gw", IPAddress(10, 255, 250, 1));
  outIp = loadIPAddress("out", IPAddress(10, 255, 250, 129));
  preferences.end();
}

void handleBTCommands() {
  if (!SerialBT.available()) return;

  String command = SerialBT.readStringUntil('\n');
  command.trim();

  auto updateIP = [&](const String& prefix, IPAddress& target, int offset) {
    String value = command.substring(offset);
    if (target.fromString(value)) {
      saveNetworkConfig();
      SerialBT.printf("%s updated and saved.\n", prefix.c_str());
    } else {
      SerialBT.printf("Invalid %s format.\n", prefix.c_str());
    }
  };

  if (command.startsWith("SET_IP ")) {
    updateIP("IP", ip, 7);
  } else if (command.startsWith("SET_SUBNET ")) {
    updateIP("Subnet", subnet, 11);
  } else if (command.startsWith("SET_GATEWAY ")) {
    updateIP("Gateway", gateway, 12);
  } else if (command.startsWith("SET_OUTIP ")) {
    updateIP("OutIP", outIp, 10);
  } else if (command == "GET_CONFIG") {
    SerialBT.printf("IP: %s\n", ip.toString().c_str());
    SerialBT.printf("Subnet: %s\n", subnet.toString().c_str());
    SerialBT.printf("Gateway: %s\n", gateway.toString().c_str());
    SerialBT.printf("OutIP: %s\n", outIp.toString().c_str());
  } else {
    SerialBT.println("Invalid command.");
  }
}

bool tfStatus_1 = 0;
bool tfStatus_2 = 0;
int distance1   = 0;
int distance2   = 0;

uint32_t lastMillis = 0;

void oscSend(uint8_t deviceId, int value) {
  char address[20];
  snprintf(address, sizeof(address), "/device%d/", deviceId);
  OSCMessage msg(address);
  msg.add(value);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void readSerial(){
  if (Serial.available()>0) {
    char c = Serial.read();
    if (c=='i'){ Serial.printf("ETH IP: %s\n", ETH.localIP().toString().c_str());}
    if (c=='m'){ Serial.printf("ETH MAC: %s\n", ETH.macAddress().c_str());}
  }
}

void readTFMini() {
  if (tfMini_1.readData()) {
    distance1 = tfMini_1.getDistance();
    if      (!tfStatus_1 && distance1 <  THRESHOLD_DISTANCE) { tfStatus_1 = 1; oscSend(deviceID, 1); if (DEBUG){ Serial.println("TF 1 Triggered"); }}
    else if (tfStatus_1  && distance1 >= THRESHOLD_DISTANCE) { tfStatus_1 = 0; oscSend(deviceID, 0); if (DEBUG){ Serial.println("TF 1 Reset"); }}
  }
  if (tfMini_2.readData()) {
    distance2 = tfMini_2.getDistance();
    if      (!tfStatus_2 && distance2 <  THRESHOLD_DISTANCE) { tfStatus_2 = 1; oscSend(deviceID, 1); if (DEBUG){ Serial.println("TF 2 Triggered"); }}
    else if ( tfStatus_2 && distance2 >= THRESHOLD_DISTANCE) { tfStatus_2 = 0; oscSend(deviceID, 0); if (DEBUG){ Serial.println("TF 2 Reset"); }}
  } 
  if (DEBUG && millis() - lastMillis > DELAY){
    lastMillis = millis();
    Serial.printf ("Device ID: %d Distance 1: %d \t", deviceID, distance1);
    Serial.printf ("Distance 2: %d \n", distance2);
  }
}

void readSerialBT() {
  if (SerialBT.available()) {
    char c = SerialBT.read();
    if (c=='i'){ SerialBT.printf("ETH IP: %s\n", ETH.localIP().toString().c_str());}
    if (c=='m'){ SerialBT.printf("ETH MAC: %s\n", ETH.macAddress().c_str());}
  }
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH IP: ");
      Serial.println(ETH.localIP());
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      break;
    default:
      break;
  }
}

void ethInit() {
  ETH.begin( ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE_0);
  ETH.config(ip, gateway, subnet);
  WiFi.onEvent(WiFiEvent);
  Udp.begin(inPort);
  delay(5000); // Wait for the Ethernet to initialize
  Serial.println("ETH Initialized");
  Serial.printf("ETH IP: %s\n", ETH.localIP().toString().c_str());
  Serial.printf("ETH MAC: %s\n", ETH.macAddress().c_str());
}

void tfminiInit() {
  tfMini_1.begin(115200);
  tfMini_2.begin(115200);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32-ETH"); // Bluetooth device name
  tfminiInit();

  loadNetworkConfig();

  Serial.printf("Loaded IP: %s\n", ip.toString().c_str());
  Serial.printf("Subnet: %s\n", subnet.toString().c_str());
  Serial.printf("Gateway: %s\n", gateway.toString().c_str());
  Serial.printf("OutIP: %s\n", outIp.toString().c_str());
  ethInit();
}

void loop() {
  readSerial();
  readTFMini();
  handleBTCommands();
}
