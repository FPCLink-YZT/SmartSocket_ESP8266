#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <WiFiServer.h>

#define relay 0
#define ledPin 2
int udpBeginPacketTest;
int udpEndPacketTest;
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
    // Serial.print(ssid[i]);
    if(ssid[i] == '\0') {
      break;
    }
  }
  for(int i=0; i<32; i++) {
    // Serial.print(password[i]);
    // Serial.print(" ");
    // Serial.print((int)password[i]);
    EEPROM.write(i+32, password[i]); //Write Password to EEPROM
    // Serial.print(password[i]);
    if(password[i] == '\0') {
      break;
    }
  }
  Serial.println("EEPROM.end");
  EEPROM.end();
}

void connectWiFi(const char* ssid, const char* password) { //Connect to WiFi
  WiFi.begin(ssid,password);
  while(WiFi.status() !=WL_CONNECTED){ // Wait for WiFi connection || Blink LED while connecting
    digitalWrite(ledPin,LOW); // Turn on LED
    delay(250);
    Serial.println(".");
    digitalWrite(ledPin,HIGH);// Turn off LED
    delay(250);
  }
}

void setup(){
  static String serial_command;
  Serial.begin(115200);
  Serial.println(""); //Print firmware information
  Serial.println("Smart Socket ESP8266EX Firmware Ver.1.0.1.stable");
  Serial.println("Developed by Madobi Nanami");
  Serial.println("Personal site: https://nanami.tech");
  pinMode(ledPin,OUTPUT); //Initialize LED pin
  pinMode(relay,OUTPUT); //Initialize relay pin
  digitalWrite(relay,LOW);
  digitalWrite(ledPin,LOW); //Turn on LED
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
    connectWiFi(inputSsid.c_str(), inputPassword.c_str()); //Connect to WiFi
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to connect to WiFi, please check your SSID and Password.");
      digitalWrite(ledPin,LOW); //Stop blinking LED if failed to connect
      return;
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
  connectWiFi(ssid, password); //Connect to WiFi
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi, please check your SSID and Password.");
    digitalWrite(ledPin,LOW); //Stop blinking LED if failed to connect
    return;
  }
  Serial.println("Connected.");
  digitalWrite(ledPin,LOW); //Turn on LED after connected
  server.begin();
  Udp.begin(localUdpPort);
  Serial.println("UDP server started. Listening on port 443");
  Serial.print("Use this address to connect: ");
  Serial.println(WiFi.localIP());
}

void serial_action(String serial_command){ // Function to handle serial commands
        if(serial_command == "device.read"){ // Read device information
          Serial.println("Device information requested.");
          Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
          Udp.write("[Chipset]ESP8266EX");
          Udp.write("[Manufacture]Espressif");
          Udp.write("[DeviceType]relayController");
          Udp.write("[FirmwareName]Smart Socket");
          Udp.write("[FirmwareVersion]1.0.1.stable");
          Udp.write("[FirmwareBuildVer]25");
          Udp.write("[Developer]Madobi Nanami");
          Udp.write("[BuildDate]2025.07.28");
          delay(500);
          Udp.write("[Command]EOF");
          Udp.endPacket();
          Serial.println("[Chipset]ESP8266EX");
          Serial.println("[Manufacture]Espressif");
          Serial.println("[DeviceType]relayController");
          Serial.println("[FirmwareName]Smart Socket");
          Serial.println("[FirmwareVersion]1.0.1.stable");
          Serial.println("[FirmwareBuildVer]25");
          Serial.println("[Developer]Madobi Nanami");
          Serial.println("[BuildDate]2025.07.28");
          Serial.println("End.");
          return;
        }
        if(serial_command == "led.read"){ // Read LED status
          digitalStatus = digitalRead(ledPin);
          Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
          Udp.write(digitalStatus);
          Udp.endPacket();
          return;
        }
        if(serial_command == "relay.read"){ // Read relay status
          digitalStatus = digitalRead(relay);
          Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
          Udp.write(digitalStatus);
          Udp.endPacket();
          return;
        }
        if(serial_command == "led.on"){ // Turn on LED
          digitalWrite(ledPin,LOW);
          Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
          Udp.write("led.on");
          Udp.endPacket();
          return;
        }
        if(serial_command == "led.off"){ // Turn off LED
          digitalWrite(ledPin,HIGH);
          Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
          Udp.write("led.off");
          Udp.endPacket();
          return;
        }
        if(serial_command == "relay.on"){ // Turn on relay
          digitalWrite(relay,HIGH);
          Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
          Udp.write("relay.on");
          Udp.endPacket();
          return;
        }
        if(serial_command == "relay.off"){ // Turn off relay
          digitalWrite(relay,LOW);
          Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
          Udp.write("relay.off");
          Udp.endPacket();
        }
        if(serial_command == "wifi.reset"){ // Reset WiFi settings
          Serial.println("Resetting WiFi settings...");
          EEPROM.begin(64);
          for(int i=0; i<64; i++) {
            EEPROM.write(i, 0); // Clear EEPROM
          }
          EEPROM.end();
          Serial.println("WiFi settings reset.");
          Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
          Udp.write("WiFi settings reset.");
          Udp.endPacket();
          return;
        }
      }

void loop(){
  int packetSize = Udp.parsePacket();
  if(packetSize){
    Serial.printf("Received %d bytes from client: %s:%d\n",packetSize,Udp.remoteIP().toString()); // Report income udp packet
    int len = Udp.read(incomingPacket,536);
    if(len>0){
      incomingPacket[len] = 0;
      Serial.printf("UDP packet contents: %s\n", incomingPacket);
      if(strcmp(incomingPacket, "udp.test") == 0){ // Test UDP connection
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

  if(strcmp(incomingPacket, "device.read") == 0 || 
     strcmp(incomingPacket, "led.read") == 0 || 
     strcmp(incomingPacket, "relay.read") == 0 || 
     strcmp(incomingPacket, "led.on") == 0 || 
     strcmp(incomingPacket, "led.off") == 0 || 
     strcmp(incomingPacket, "relay.on") == 0 || 
     strcmp(incomingPacket, "relay.off") == 0) {
        serial_action(incomingPacket);
      } else {
        Serial.println("Unknown command received.");
        Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        Udp.write("Unknown command");
        Udp.endPacket();
      }
    }
  }

  if(Serial.available()) { // Check if there is data available on serial port
    String serial_command = Serial.readStringUntil('\n'); // Read serial command
    serial_command.trim(); // Remove whitespace
    Serial.printf("Received command: %s\n", serial_command.c_str()); // Print received command
    if(serial_command == "device.read" || 
       serial_command == "led.read" || 
       serial_command == "relay.read" || 
       serial_command == "led.on" || 
       serial_command == "led.off" || 
       serial_command == "relay.on" || 
       serial_command == "relay.off" ||
       serial_command == "wifi.reset") {
      serial_action(serial_command); // Handle the command
    } else {
      Serial.println("Unknown command received.");
    }
  }

  if(!WL_CONNECTED == WiFi.status()){ // Check if WiFi is connected
    Serial.println("WiFi disconnected, trying to reconnect...");
    digitalWrite(ledPin,LOW); // Turn off LED while reconnecting
    WiFi.disconnect();
    WiFi.begin();
    while(WiFi.status() != WL_CONNECTED){ // Wait for WiFi connection
      delay(500);
      Serial.print(".");
      digitalWrite(ledPin,HIGH); // Blink LED while connecting
      delay(500);
      digitalWrite(ledPin,LOW); // Blink LED while connecting
    }
    Serial.println("Reconnected to WiFi.");
    digitalWrite(ledPin,LOW); // Turn on LED after reconnected
  }
  // TODO: Implement web server functionality
  // if(server.hasClient()){ // Check if there is a client connected
  //   Serial.println("Client connected to web server.");
  //   server.setNoDelay(true); // Disable Nagle's algorithm for low latency
  //   WiFiClient client = server.available();
  //   if(client){
  //     Serial.println("New client connected.");
  //     String currentLine = "";
  //     while(client.connected()){
  //       if(client.available()){
  //         char c = client.read();
  //         Serial.write(c);
  //         if(c == '\n'){
  //           if(currentLine.length() == 0){
  //             client.println("HTTP/1.1 200 OK");
  //             client.println("Content-Type: text/html");
  //             client.println("Connection: close");
  //             client.println();
  //             client.println("<!DOCTYPE HTML>");
  //             client.println("<html>");
  //             client.println("<head><title>Smart Socket</title></head>");
  //             client.println("<body>");
  //             client.println("<h1>Smart Socket</h1>");
  //             client.println("<p>ESP8266EX Smart Socket</p>");
  //             client.println("<p>Firmware Version: 1.0.1.stable</p>");
  //             client.println("<p>Developer: Madobi Nanami</p>");
  //             client.println("</body></html>");
  //             break;
  //           } else {
  //             currentLine = "";
  //           }
  //         } else if(c != '\r') {
  //           currentLine += c;
  //         }
  //       }
  //     }
  //     delay(1);
  //     client.stop();
  //     Serial.println("Client disconnected.");
  //   }
  // }
}