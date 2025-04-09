#define DEBUG              0 // Enable debug messages

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

uint8_t   deviceID;
uint16_t  thresh_dist;

bool tfStatus_1 = 0;
bool tfStatus_2 = 0;
int distance1   = 0;
int distance2   = 0;

uint32_t lastMillis = 0;

IPAddress ip, subnet, gateway, outIp;
uint16_t inPort = 7001;
uint16_t outPort = 7000;

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
  preferences.putUChar("deviceID", deviceID);  // Save deviceID
  preferences.putUInt("thresh_dist", thresh_dist); // Save threshold distance
  preferences.putUInt("inPort", inPort); // Save input port
  preferences.putUInt("outPort", outPort); // Save output port
  preferences.end();
}

void loadNetworkConfig() {
  preferences.begin(namespaceName, true);
  ip = loadIPAddress("ip", IPAddress(10, 255, 250, 150));
  subnet = loadIPAddress("sub", IPAddress(255, 255, 254, 0));
  gateway = loadIPAddress("gw", IPAddress(10, 255, 250, 1));
  outIp = loadIPAddress("out", IPAddress(10, 255, 250, 129));
  deviceID = preferences.getUChar("deviceID", 1);  // Load deviceID
  thresh_dist = preferences.getUInt("thresh_dist", 100); // Load threshold distance
  inPort = preferences.getUInt("inPort", 7001); // Load input port
  outPort = preferences.getUInt("outPort", 7000); // Load output port
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
      SerialBT.printf("✅ %s updated and saved.\n", prefix.c_str());
    } else {
      SerialBT.printf("❌ Invalid %s format.\n", prefix.c_str());
    }
  };

  if (command.startsWith("SET_IP ")) { updateIP("IP", ip, 7); } 
  else if (command.startsWith("SET_SUBNET ")) { updateIP("Subnet", subnet, 11); } 
  else if (command.startsWith("SET_GATEWAY ")) { updateIP("Gateway", gateway, 12);  } 
  else if (command.startsWith("SET_OUTIP ")) { updateIP("OutIP", outIp, 10); } 
  else if (command.startsWith ("SET_INPORT ")) {
    int port = command.substring(10).toInt();
    if (port > 0 && port < 65536) { inPort = static_cast<uint16_t>(port); saveNetworkConfig(); SerialBT.printf("✅ Input port set to %d and saved.\n", inPort); } 
    else { SerialBT.println("❌ Invalid port. Must be between 1 and 65535."); }
  }
  else if (command.startsWith("SET_OUTPORT ")) {
    int port = command.substring(12).toInt();
    if (port > 0 && port < 65536) { outPort = static_cast<uint16_t>(port); saveNetworkConfig(); SerialBT.printf("✅ Output port set to %d and saved.\n", outPort); } 
    else { SerialBT.println("❌ Invalid port. Must be between 1 and 65535."); }
  }
  else if (command.startsWith("SET_THRESHOLD ")) {
    int threshold = command.substring(14).toInt();
    if (threshold > 0) { thresh_dist = static_cast<uint16_t>(threshold); saveNetworkConfig(); SerialBT.printf("✅ Threshold distance set to %d and saved.\n", thresh_dist); } 
    else { SerialBT.println("❌ Invalid threshold. Must be greater than 0."); }
  }
  else if (command == "IP") { SerialBT.printf("ETH IP: %s\n", ETH.localIP().toString().c_str());}
  else if (command == "MAC") { SerialBT.printf("ETH MAC: %s\n", ETH.macAddress().c_str());}
  else if (command == "GET_CONFIG") {
    SerialBT.printf("deviceID: %d\n", deviceID);
    SerialBT.printf("Threshold dist: %d\n", thresh_dist);
    SerialBT.printf("Input port: %d\n", inPort);
    SerialBT.printf("Output port: %d\n", outPort);
    SerialBT.printf("IP: %s\n", ip.toString().c_str());
    SerialBT.printf("Subnet: %s\n", subnet.toString().c_str());
    SerialBT.printf("Gateway: %s\n", gateway.toString().c_str());
    SerialBT.printf("OutIP: %s\n", outIp.toString().c_str());
  }
  else if (command.startsWith("SET_ID ")) {
    int id = command.substring(7).toInt();
    if (id >= 0 && id <= 255) { deviceID = static_cast<uint8_t>(id); saveNetworkConfig(); SerialBT.printf("✅ deviceID set to %d and saved.\n", deviceID);} 
    else { SerialBT.println("❌ Invalid ID. Must be between 0 and 255."); }
  }
  else { SerialBT.println("Invalid command."); }
}

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
    if      (!tfStatus_1 && distance1 <  thresh_dist) { tfStatus_1 = 1; oscSend(deviceID, 1); if (DEBUG){ Serial.println("TF 1 Triggered"); }}
    else if (tfStatus_1  && distance1 >= thresh_dist) { tfStatus_1 = 0; oscSend(deviceID, 0); if (DEBUG){ Serial.println("TF 1 Reset"); }}
  }
  if (tfMini_2.readData()) {
    distance2 = tfMini_2.getDistance();
    if      (!tfStatus_2 && distance2 <  thresh_dist) { tfStatus_2 = 1; oscSend(deviceID, 1); if (DEBUG){ Serial.println("TF 2 Triggered"); }}
    else if ( tfStatus_2 && distance2 >= thresh_dist) { tfStatus_2 = 0; oscSend(deviceID, 0); if (DEBUG){ Serial.println("TF 2 Reset"); }}
  } 
  if (DEBUG && millis() - lastMillis > DELAY){
    lastMillis = millis();
    Serial.printf ("Device ID: %d Distance 1: %d \t", deviceID, distance1);
    Serial.printf ("Distance 2: %d \n", distance2);
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
