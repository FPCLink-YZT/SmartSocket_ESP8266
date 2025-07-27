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

WiFiUDP Udp;
unsigned int localUdpPort = 443;
char incomingPacket[537];
WiFiServer server(80);

void loadWiFiSettings(char* ssid, char* password) { //Read WiFi settings from EEPROM
  EEPROM.begin(64); //Initialize EEPROM with size 64 bytes
  for (int i = 0; i < 32; i++) {
   char c = EEPROM.read(i);
  //  Serial.print(c);
  //  Serial.print(" ");
  //  Serial.print((int)c);
  //  Serial.print("  ");
   ssid[i] = c;
   if(c =='\0'){
    break;
   }
  }
  for (int i = 0; i < 32; i++) {
   char c = EEPROM.read(i + 32);
  //  Serial.print(c);
  //  Serial.print(" ");
  //  Serial.print((int)c);
  //  Serial.print("  ");
   password[i] = c;
   if(c == '\0'){
    break;
   }
  }
  EEPROM.end();
}

void saveWiFiSettings(const char ssid[32], const char password[32]) {
  EEPROM.begin(64); //Initialize EEPROM with size 64 bytes
  for(int i=0; i<32; i++) {
    // Serial.print(ssid[i]);
    // Serial.print(" ");
    // Serial.print((int)ssid[i]);
    EEPROM.write(i, ssid[i]); //Write SSID to EEPROM
    Serial.print(ssid[i]);
    if(ssid[i] == '\0') {
      break;
    }
  }
  for(int i=0; i<32; i++) {
    // Serial.print(password[i]);
    // Serial.print(" ");
    // Serial.print((int)password[i]);
    EEPROM.write(i+32, password[i]); //Write Password to EEPROM
    Serial.print(password[i]);
    if(password[i] == '\0') {
      break;
    }
  }
  Serial.println("EEPROM.end");
  EEPROM.end();
}

void setup(){
  Serial.begin(115200);
  Serial.println(""); //Print firmware information
  Serial.println("Smart Socket ESP8266EX Firmware Ver.1.0");
  Serial.println("Developed by Madobi Nanami");
  Serial.println("Personal site: https://nanami.tech");
  pinMode(ledPin,OUTPUT); //Initialize LED pin
  digitalWrite(ledPin,HIGH);
  pinMode(relay,OUTPUT); //Initialize relay pin
  digitalWrite(relay,LOW);
  char ssid[32];
  char password[32];
  loadWiFiSettings(ssid, password); //Load WiFi settings from EEPROM
  if(strlen(ssid) == 0 || strlen(password) == 0){ //Check if WiFi settings are empty
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
    WiFi.begin(inputSsid.c_str(), inputPassword.c_str());
    while(WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to WiFi.");
    Serial.println("Saving WiFi settings to EEPROM...");
    saveWiFiSettings(inputSsid.c_str(), inputPassword.c_str());
    Serial.println("WiFi settings saved.");
    
  } else {
    Serial.println("Load WiFi settings from EEPROM.");
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
        Serial.println("Device information requested.");
        Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        Udp.write("[Chipset]ESP8266EX");
        Udp.write("[Manufacture]Espressif");
        Udp.write("[DeviceType]relayController");
        Udp.write("[FirmwareName]Smart Socket");
        Udp.write("[FirmwareVersion]1.0.stable");
        Udp.write("[FirmwareBuildVer]24");
        Udp.write("[Developer]Madobi Nanami");
        Udp.write("[BuildDate]2025.07.26");
        delay(500);
        Udp.write("[Command]EOF");
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