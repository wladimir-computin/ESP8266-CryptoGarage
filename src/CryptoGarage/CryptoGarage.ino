/*
  CryptoGarage

  A pracitcal example of a secure IOT device. (still rare nowadays!)

  CryptoGarage uses a challenge-response system over TCP based on AES-GCM and SHA512 for key generation.
  Challenge-response has some major benefits over a rolling code based exchange, for example
  immunity to replay-attacks and easy managment of multiple remote devices, in our case smartphones.

  CryptoGarage *should* be at least equally or more secure than any comparable commercially available system.
  It isn't relying on any closed source / hidden algorithms. (Except for the hardware random generator in
  our ESP8266 for IV generation, which seems to be fine but that's the problem with random, you never know for sure.)
  Replay attacks are not possible and AFAIK the "best" way to get the key is to caputure the encrypted
  HELLO message of the client and try to brute force it. Since we are using AES256 in GCM-mode this
  should take plenty of time and breaking into the garage using dynamite is faster. As always, you MUST
  choose a strong password, 123456 won't last that long. BTW, keep in mind that physical access to the
  device renders any software security useless.

  I personally use CryptoGarage in AP-Mode, not connected to the internet. But I designed the system
  to be secure independent of the surrounding network. Exposing it as a client to the homenetwork should be fine.
  Even exposing it to the internet should work (however persistent denial-of-service attacks may
  render it unusable.) But I wouldn't recommend to make *any* IOT accessible from the internet without VPN.

  I DO NOT GUARANTEE ANYTHING!
  If someone break into your garage and steals your fancy car, I won't buy you a new one.

  If you find bugs, contact me :)
*/

/*
  CryptoGarage - Main

  Contains the main functionality.
  Entrypoint is setup()
*/

//Build configuration
#include "TCPCleanup.h"
#include "AllConfig.h"

//OTA Update
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>

//WiFi AccessPoint and TCP
#include <ESP8266WiFi.h>
//UDP
#include <WiFiUdp.h>

#include "Debug.h"
#include "Device.h"
#include "Crypto.h"
#include "PersistentMemory.h"
#include "WiFiManager.h"
#include "ChallengeManager.h"
#include "RateLimit.h"
#include "Message.h"
#include "Uptime.h"
#include "OTA.h"
#if ENABLE_STATUS_LED == 1
  #include "StatusLED.h"
#endif

#include "Garage.h"

//Default settings
String WIFISSID = DEFAULT_WIFISSID; //Defined in AllConfig.h
String WIFIPASS = DEFAULT_WIFIPASS;
String DEVICEPASS = DEFAULT_DEVICEPASS;
String WIFIMODE = DEFAULT_WIFIMODE;

const char COMMAND_HELLO[] = "hellofriend";
const char COMMAND_SET_DEVICE_PASS[] = "setDevicePass";
const char COMMAND_GET_STATUS[] = "getStatus";
const char COMMAND_SET_WIFI_SSID[] = "setSSID";
const char COMMAND_SET_WIFI_PASS[] = "setWIFIPass";
const char COMMAND_SET_WIFI_MODE[] = "setWIFIMode";
const char COMMAND_SET_AUTOTRIGGER_TIMEOUT[] = "setAutotriggerTimeout";
const char COMMAND_GET_SETTINGS[] = "getSettings";
const char COMMAND_DISCOVER[] = "discover";
const char COMMAND_SAVE[] = "save";
const char COMMAND_PING[] = "ping";
const char COMMAND_REBOOT[] = "reboot";
const char COMMAND_RESET[] = "reset";
const char COMMAND_UPDATE[] = "update";

//TCP server on port 4646
WiFiServer server(TCP_SERVER_PORT);
//TCP Client, global because we want to keep tcp connections between loops alive
WiFiClient client;

//UDP client
WiFiUDP udp;

#if ENABLE_STATUS_LED == 1
  StatusLED &led = StatusLED::instance();
#endif
Uptime &uptime = Uptime::instance();

//Garage specific stuff
Garage garage;
Device * device = &garage;

//OTA update
bool update_mode = false;

//Communication specific stuff
Crypto &crypt = Crypto::instance();
PersistentMemory &mem = PersistentMemory::instance();
WiFiManager &wifi = WiFiManager::instance();
OTA ota;
ChallengeManager challengeManager;
RateLimit rateLimit;

//Create WiFi AP
void setupWiFi() {  
  wifi.setCredentials(WIFISSID, WIFIPASS);
  wifi.setMode(wifi.string2mode(WIFIMODE));
  wifi.init();
}

void initCrypt() {
  crypt.init(DEVICEPASS);
}

void loadSettings() {
  printDebug("Initialising PersistentMemory");

  //mem.clearEEPROM(MEM_FIRST, MEM_LAST); //uncomment this to clear all saved settings on startup.

  if (mem.readBoolFromEEPROM(MEM_DEV_PASS_SET) == true) { //True if this setting was set atleast once. Otherwise, use default value specified above.
    DEVICEPASS = mem.readStringFromEEPROM(MEM_DEV_PASS, 64);
  }
  printDebug("Loaded DEVICEPASS: " + DEVICEPASS);

  if (mem.readBoolFromEEPROM(MEM_WIFI_SSID_SET) == true) {
    WIFISSID = mem.readStringFromEEPROM(MEM_WIFI_SSID, 32);
  }
  printDebug("Loaded WIFISSID: " + WIFISSID);

  if (mem.readBoolFromEEPROM(MEM_WIFI_PASS_SET) == true) {
    WIFIPASS = mem.readStringFromEEPROM(MEM_WIFI_PASS, 63);
  }
  printDebug("Loaded WIFIPASS: " + WIFIPASS);

  if (mem.readBoolFromEEPROM(MEM_WIFI_MODE_SET) == true) {
    WIFIMODE = mem.readStringFromEEPROM(MEM_WIFI_MODE, 6);
  }
  printDebug("Loaded WIFIMODE: " + WIFIMODE);

  //if (mem.readBoolFromEEPROM(MEM_AUTOTRIGGER_TIMEOUT_SET) == true) {
    //autoTrigger.setEnd(mem.readIntFromEEPROM(MEM_AUTOTRIGGER_TIMEOUT));
  //}
}

void initHardware() {
  #if DEBUG == 1 //defined in AllConfig.h
    Serial.begin(115200);
    printDebug("Device running in Debug-Mode!");
  #else
    #if RELAYLCTECH == 1
      Serial.begin(9600); //The LCTech relay needs serial at 9600 baud.
    #endif
  #endif
}

void enableOTA(){
  if (!update_mode) {
    ota.start();
    update_mode = true;
  }
}

//Here we process the plaintext commands and generate an answer for the client.
ProcessMessageStruct processMessage(String &message) {
  
  ProcessMessageStruct p = device->processMessage(message);
  if (!(p.responseCode == ERR && p.responseData == "NO_COMMAND")) {
    return p;
  }

  if (message == COMMAND_PING) {
      return {ERR, ""};
  }

  if (message == COMMAND_GET_STATUS) {
    String data = String("Device name: ") + DEFAULT_HOSTNAME + "\n" +
                  "FW-Version: " + String(FW_VERSION) + "\n" +
                  "Local IP: " + wifi.getIP() + "\n\n" +
                  //"Autotrigger engaged: " + String(autoTrigger.isActive()) + "\n" +
                  //"Autotrigger timeout: " + String(autoTrigger.tickerEnd) + "s" + "\n" +
                  
                  "Ratelimit: " + RATE_LIMIT_TIMEOUT_MS + "ms" + "\n" +
                  //"Relaystate: " + String(relay.getState()) + "\n" +
                  "Updatemode: " + String(update_mode) + "\n" +
                  "Free HEAP: " + String(ESP.getFreeHeap()) + "Byte" + "\n" +
                  device->getStatus() + "\n\n" +
                  "Uptime: " + uptime.getUptime();
    return {DATA, data};
  }

  if (message == COMMAND_REBOOT) {
    printDebug("Rebooting...");
    ESP.restart();
    return {ACK, ""}; //this will never reach the client, whatever.
  }

  if (message.startsWith(COMMAND_SET_DEVICE_PASS)) {
    return mem.writeSettings(message, 8, 64, COMMAND_SET_DEVICE_PASS, MEM_DEV_PASS, MEM_DEV_PASS_SET, "string");
  }

  if (message.startsWith(COMMAND_SET_WIFI_SSID)) {
    return mem.writeSettings(message, 3, 32, COMMAND_SET_WIFI_SSID, MEM_WIFI_SSID, MEM_WIFI_SSID_SET, "string");
  }

  if (message.startsWith(COMMAND_SET_WIFI_PASS)) {
    return mem.writeSettings(message, 8, 63, COMMAND_SET_WIFI_PASS, MEM_WIFI_PASS, MEM_WIFI_PASS_SET, "string");
  }

  if (message.startsWith(COMMAND_SET_WIFI_MODE)) {
    return mem.writeSettings(message, 2, 6, COMMAND_SET_WIFI_MODE, MEM_WIFI_MODE, MEM_WIFI_MODE_SET, "string");
  }

  if (message.startsWith(COMMAND_SET_AUTOTRIGGER_TIMEOUT)) {
    return mem.writeSettings(message, 1, 4, COMMAND_SET_AUTOTRIGGER_TIMEOUT, MEM_AUTOTRIGGER_TIMEOUT, MEM_AUTOTRIGGER_TIMEOUT_SET, "int");
  }

  if (message == COMMAND_SAVE) {
    printDebug("Saving settings!");
    mem.commit();
    return {ACK, ""};
  }

  if (message == COMMAND_RESET) {
    printDebug("Clearing PMEM!");
    mem.clearEEPROM(MEM_FIRST, MEM_LAST);
    return {ACK, ""};
  }

  if (message == COMMAND_UPDATE) {
    enableOTA();
    return {DATA, "http://" + wifi.getIP() + ":" + HTTP_OTA_PORT + UPDATE_PATH};
  }

  printDebug("Received unknown command:" + message);
  return {ERR, "Unknown Command"};
}

void send_Data_UDP(String data, IPAddress ip, int port) {
  data = Message::wrap(data);
  printDebug("\n[UDP] Sending:\n" + data + "\n");
  udp.beginPacket(ip, port);
  udp.write((uint8_t*)data.c_str(), data.length());
  udp.endPacket();
}

void send_Data_UDP(String data) {
  send_Data_UDP(data, udp.remoteIP(), udp.remotePort());
}

String receive_Data_UDP() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[512];
    int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
    udp.flush();
    if (len > 0) {
      incomingPacket[len] = '\0';
      String packet = String(incomingPacket);
      printDebug("\n[UDP] Received:\n" + packet);
      String body = Message::unwrap(packet);
      return body;
    }
  }
  return "";
}

void stopClient_UDP() {
  rateLimit.setState(BLOCKED);
  yield();
}

void send_Data_TCP(WiFiClient &client, String data) {
  data = Message::wrap(data);
  printDebug("\n[TCP] Sending:\n" + data + "\n");
  client.println(data);
  yield();
}

String receive_Data_TCP(WiFiClient &client) {
  if (client.available()) {
    String incoming = client.readStringUntil('\n');
    client.flush();
    yield();
    printDebug("\n[TCP] Received:\n" + incoming);
    String body = Message::unwrap(incoming);
    return body;
  }
  return "";
}

void stopClient_TCP(WiFiClient &client) {
  rateLimit.setState(BLOCKED);
  client.stop();
  yield();
}

void doTCPServerStuff() {

  if (!client || !client.connected()) { //garbage collection
    if (client) {
      client.stop();
    }
    // wait for a client to connect
    client = server.available();
    client.setTimeout(TCP_TIMEOUT_MS);
    client.setNoDelay(true);
    client.setSync(true);
  }

  if (rateLimit.getState() == OPEN) { //Check if we are ready for a new connection

    String s = receive_Data_TCP(client);

    if (s != "") {

      Msg message; //Msg defined in Message.h

      message = Message::decrypt(s, challengeManager);

      if(message.type != NOPE) {
  
        if (message.type == HELLO) {
          //preparing random challenge secret
          send_Data_TCP(client, Message::encrypt(HELLO, "", message.challenge, challengeManager.generateRandomChallenge(), FLAG_NONE));
          //sending encrypted challenge to client, the client has to prove its knowledge of the correct password by encrypting the next
          //message containing our challenge. We have proven our knowledge by sending the reencrypted client_challenge back.
  
        //the client send a correctly encrypted second message.
        } else if (message.type == DATA){
          ProcessMessageStruct ret = processMessage(message.data); //process his data

          //preparing next random challenge secret
          //send answer
          send_Data_TCP(client, Message::encrypt(ret.responseCode, ret.responseData, message.challenge, challengeManager.generateRandomChallenge(), ret.flags));
          
          //for some messages (like setSettings) we know that the client may send a following one very quickly.
          //other messages (like trigger) shouldn't be called that quickly.
          if (!(ret.flags.indexOf(FLAG_KEEP_ALIVE) != -1)){
            stopClient_TCP(client);
          }
        } else {
          printDebug("Wrong formatted message received!");
          send_Data_TCP(client, Message::encrypt(ERR, "Nope!", "", "", FLAG_NONE));
          stopClient_TCP(client);
        }
        
      } else {
        printDebug("Decryption failed!");
        challengeManager.resetChallenge();
        send_Data_TCP(client, Message::encrypt(ERR, "Nope!", "", "", FLAG_NONE));
        stopClient_TCP(client);
      }
      printDebug("-------------------------------");
    }
  } else {
    client.stop();
    yield();
    tcpCleanup();
  }
}

String discoverResponse = String(HEADER_DATA) + ":::" + DEFAULT_HOSTNAME;
String discover = String(HEADER_HELLO) + ":::" + COMMAND_DISCOVER;

void doUDPServerStuff(){

  if (rateLimit.getState() == OPEN) { //Check if we are ready for a new connection

    String s = receive_Data_UDP();
    
    if (s != "") {

      if(s == discover){
        send_Data_UDP(discoverResponse);
        stopClient_UDP();
        return;
      }

      Msg message; //Msg defined in Message.h

      message = Message::decrypt(s, challengeManager);
      
      //check if the client send a correctly encrypted message.
      if(message.type != NOPE) {

        if (message.type == HELLO) {
          //preparing random challenge secret
          send_Data_UDP(Message::encrypt(HELLO, "", message.challenge, challengeManager.generateRandomChallenge(), FLAG_NONE));
          //sending  challenge to client, the client has to prove its knowledge of the correct password by encrypting the next
          //message containing our challenge. We have proven our knowledge by sending the reencrypted client_challenge back.
  
        } else if (message.type == DATA){
          ProcessMessageStruct ret = processMessage(message.data); //process his data
          
          //preparing next random challenge secret
          send_Data_UDP(Message::encrypt(ret.responseCode, ret.responseData, message.challenge, challengeManager.generateRandomChallenge(), ret.flags)); //send answer
         
          //for some messages (like setSettings) we know that the client may send a following one very quickly.
          //other messages (like trigger) shouldn't be called that quickly.
          if (!(ret.flags.indexOf(FLAG_KEEP_ALIVE) != -1)){
            stopClient_UDP();
          }
        } else {
          printDebug("Wrong formatted message received!");
          send_Data_UDP(Message::encrypt(ERR, "Nope!", "", "", FLAG_NONE));
          stopClient_UDP();
        }
        
      } else {
        printDebug("Decryption failed!");
        challengeManager.resetChallenge();
        send_Data_UDP(Message::encrypt(ERR, "Nope!", "", "", FLAG_NONE));
        stopClient_UDP();
      }
      printDebug("-------------------------------");
    }
  }
}

void loop() {
  device->loop();

  if (update_mode) {
    ota.loop();
  }
  
  doUDPServerStuff();
  doTCPServerStuff();
}

//Here's the entrypoint.
void setup() {
  initHardware();
  printDebug("\nLoading Firmware: " + String(FW_VERSION));
  loadSettings();
  initCrypt();
  setupWiFi();
  printDebug("Starting TCP server on port " + String(TCP_SERVER_PORT));
  server.begin();
  printDebug("Starting UDP server on port " + String(UDP_SERVER_PORT));
  udp.begin(UDP_SERVER_PORT);
  device->setup();
  uptime.start();
  printDebug("Free HEAP: " + String(ESP.getFreeHeap()));
  printDebug("Bootsequence finished in " + String(millis()) + "ms" + "!\n");
  #if ENABLE_STATUS_LED == 1
    led.fade(StatusLED::SINGLE_ON_OFF, 2000);
  #endif
}
