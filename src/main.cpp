#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <WiFiServer.h>

#define relay 0
int ledPin = 2;
int UdpbeginPacketTest;
int UdpendPacketTest;
int digitalStatus;

const char *ssid = "YZTRoomWifi";
const char *password = "fpclink114roomwifi";

WiFiUDP Udp;
unsigned int localUdpPort = 443;
char incomingPacket[537];
WiFiServer server(80);

void setup(){
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Smart Socket ESP8266EX Firmware Ver0.2.stable Build24");
  Serial.println("Developed by Madobi Nanami");
  Serial.println("Personal site: https://nanami.tech");
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,HIGH);
  pinMode(relay,OUTPUT);
  digitalWrite(relay,LOW);
  EEPROM.begin(512);
  EEPROM.read(0,32);
  EEPROM.read(32,32);
  if(strlen(ssid) == 0 || strlen(password) == 0){
    Serial.println("No WiFi settings found in EEPROM, please set them.");
    Serial.println("Please enter SSID:");
    while(!Serial.available());
    String inputSsid = Serial.readStringUntil('\n');
    inputSsid.trim();
    if(inputSsid.length() > 32) {
      Serial.println("SSID is too long, please enter a shorter one.");
      return;
    }
    Serial.println("Please enter Password:");
    while(!Serial.available());
    String inputPassword = Serial.readStringUntil('\n');
    inputPassword.trim();
    if(inputPassword.length() > 32) {
      Serial.println("Password is too long, please enter a shorter one.");
      return;
    }
    Serial.println("Trying to connect to WiFi...");
    WIFI.begin(inputSsid.c_str(), inputPassword.c_str());
    while(WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to WiFi.");
    Serial.println("Saving WiFi settings to EEPROM...");
    saveWiFiSettings(inputSsid.c_str(), inputPassword.c_str());
    Serial.println("WiFi settings saved.");
    
  } else {
    Serial.println("Loaded WiFi settings from EEPROM.");
    String ssid = EEPROM.getString(0, 32);
    String password = EEPROM.getString(32, 32);
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(password);
  }
  WiFi.begin(ssid,password);
  while(WiFi.status() !=WL_CONNECTED){
    delay(500);
    Serial.println(".");
  }
  Serial.println("Connected.");

  server.begin();
  Udp.begin(localUdpPort);
  Serial.println("UDP server started. Listening on port 443");
  Serial.print("Use this address to connect: ");
  Serial.println(WiFi.localIP());
}

void saveWiFiSettings(const char* ssid, const char* password) {
  EEPROM.begin(512);
  EEPROM.putString(0, ssid);
  EEPROM.putString(32, password);
  EEPROM.commit();
  EEPROM.end();
}
void loadWiFiSettings(char* ssid, char* password) {
  EEPROM.begin(512);
  EEPROM.getString(0, ssid, 32);
  EEPROM.getString(32, password, 32);
  EEPROM.end();
}

void loop(){
  int packetSize = Udp.parsePacket();
  if(packetSize){
    Serial.printf("Received %d bytes from client: %s:%d\n",packetSize,Udp.remoteIP().toString());
    int len = Udp.read(incomingPacket,536);
    if(len>0){
      incomingPacket[len] = 0;
      Serial.printf("UDP packet contents: %s\n", incomingPacket);
      if(strcmp(incomingPacket, "udp.test") == 0){
        UdpbeginPacketTest = Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        if(UdpbeginPacketTest=1){
          Serial.println("Udp beginPacket test pass.");
        }
        Udp.write("udp test pass");
        UdpendPacketTest = Udp.endPacket();
        if(UdpendPacketTest=1){
          Serial.println("Udp endPacket test pass.");
        }
        return;
      }
      if(strcmp(incomingPacket, "device.read") == 0){
        Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        Udp.write("[Chipset]ESP8266EX");
        Serial.println("chipset");
        Udp.write("[Manufacture]Espressif");
        Serial.println("manufacture");
        Udp.write("[DeviceType]relayController");
        Serial.println("devicetype");
        Udp.write("[FirmwareName]Smart Socket");
        Serial.println("firmwarename");
        Udp.write("[FirmwareVersion]0.1.stable");
        Serial.println("firmwareversion");
        Udp.write("[FirmwareBuildVer]24");
        Serial.println("firmwarebuildver");
        Udp.write("[Developer]Madobi Nanami");
        Serial.println("developer");
        Udp.write("[BuildDate]2024.08.07");
        Serial.println("builddate");
        delay(500);
        Udp.write("[Command]EOF");
        Serial.println("eof");
        Udp.endPacket();
        return;
      }
      if(strcmp(incomingPacket, "led.read") == 0){
        digitalStatus = digitalRead(ledPin);
        Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        Udp.write(digitalStatus);
        Udp.endPacket();
        return;
      }
      if(strcmp(incomingPacket, "relay.read") == 0){
        digitalStatus = digitalRead(relay);
        Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        Udp.write(digitalStatus);
        Udp.endPacket();
        return;
      }
      if(strcmp(incomingPacket, "led.on") == 0){
        digitalWrite(ledPin,LOW);
        Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        Udp.write("led.on");
        Udp.endPacket();
        return;
      }
      if(strcmp(incomingPacket, "led.off") == 0){
        digitalWrite(ledPin,HIGH);
        Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        Udp.write("led.off");
        Udp.endPacket();
        return;
      }
      if(strcmp(incomingPacket, "relay.on") == 0){
        digitalWrite(relay,HIGH);
        Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        Udp.write("relay.on");
        Udp.endPacket();
        return;
      }
      if(strcmp(incomingPacket, "relay.off") == 0){
        digitalWrite(relay,LOW);
        Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        Udp.write("relay.off");
        Udp.endPacket();
      }
    }
  }
}