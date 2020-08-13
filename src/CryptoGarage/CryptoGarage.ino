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

//WiFi AccessPoint and TCP/UDP
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <LittleFS.h>

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
#include "Time.h"
#if ENABLE_STATUS_LED == 1
#include "StatusLED.h"
#endif

#include "Garage.h"

const char COMMAND_SET_DEVICE_PASS[] = "setDevicePass";
const char COMMAND_SET_WIFI_SSID[] = "setSSID";
const char COMMAND_SET_WIFI_PASS[] = "setWIFIPass";
const char COMMAND_SET_WIFI_MODE[] = "setWIFIMode";
const char COMMAND_GET_STATUS[] = "getStatus";
const char COMMAND_READ_SETTING[] = "reads";
const char COMMAND_WRITE_SETTING[] = "writes";
const char COMMAND_DISCOVER[] = "discover";
const char COMMAND_SAVE[] = "save";
const char COMMAND_PING[] = "ping";
const char COMMAND_REBOOT[] = "reboot";
const char COMMAND_RESET[] = "reset";
const char COMMAND_UPDATE[] = "update";

const char KEY_DEVICEPASS[] = "devicepass";
const char KEY_HOSTNAME[] = "hostname";
const char KEY_CHALLENGE_TIMEOUT[] = "challenge_timeout_seconds";

String hostname;
//OTA update
bool update_mode = false;
int challenge_validity_timeout;

//TCP server on port 4646
WiFiServer tcpserver(TCP_SERVER_PORT);
//TCP and UDP Client, global because we want to keep tcp connections between loops alive
WiFiClient tcpclient;
WiFiUDP udpclient;

#if ENABLE_STATUS_LED == 1
StatusLED &led = StatusLED::instance();
#endif
Uptime &uptime = Uptime::instance();
Time &mytime = Time::instance();

Crypto &crypt = Crypto::instance();
WiFiManager &wifi = WiFiManager::instance();
OTA ota;
ChallengeManager challengeManager;
RateLimit rateLimit;

//Devices
Garage garage;
Device * devices[] = {&garage};

//Create WiFi AP
void setupWiFi() {
  printDebug("[SYS] Initialising WiFi");
  PersistentMemory pmem("wifi", true);
  String ssid = pmem.readString(KEY_WIFISSID, DEFAULT_WIFISSID);
  String pass = pmem.readString(KEY_WIFIPASS, DEFAULT_WIFIPASS);
  String mode = pmem.readString(KEY_WIFIMODE, DEFAULT_WIFIMODE);
  pmem.commit();
  
  wifi.setCredentials(ssid, pass);
  wifi.setHostname(hostname);
  wifi.setMode(wifi.string2mode(mode));
  wifi.init();
}

void initSystem() {
  printDebug("[SYS] Initialising CryptoSystem");
  PersistentMemory pmem("system", true);
  hostname = pmem.readString(KEY_HOSTNAME, DEFAULT_HOSTNAME);
  String devicepass = pmem.readString(KEY_DEVICEPASS, DEFAULT_DEVICEPASS);
  challenge_validity_timeout = pmem.readInt(KEY_CHALLENGE_TIMEOUT, DEFAULT_CHALLENGE_VALIDITY_TIMEOUT);
  pmem.commit();
  crypt.init(devicepass);
  challengeManager.setChallengeTimeout(challenge_validity_timeout);
}

void initFS() {
  printDebug("[SYS] Initialising FileSystem");
  LittleFS.begin();
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

void enableOTA() {
  if (!update_mode) {
    ota.start();
    update_mode = true;
  }
}

//Here we process the plaintext commands and generate an answer for the client.
ProcessMessageStruct processMessage(String &message) {

  for(Device * d : devices){
    ProcessMessageStruct p = d->processMessage(message);
    if (!(p.responseCode == ERR && p.responseData == "NO_COMMAND")) {
      return p;
    }
  }

  if (message == COMMAND_PING) {
    return {ERR, ""};
  }

  if (message == COMMAND_DISCOVER) {
    return {DATA, hostname};
  }

  if (message == COMMAND_GET_STATUS) {
    String devicestatus = "";
    for(Device * d : devices){
      devicestatus += "\n[" + d->getName() + "]\n" + d->getStatus() + "\n";
    }
    
    char formats[] = 
           "Device name: %s\n"
           "FW-Version: %s\n"
           "Local IP: %s\n"
           "Ratelimit: %dms\n"
           "Challenge timeout: %ds\n"
           "Updatemode: %d\n"
           "Free HEAP: %dByte\n"
           "Uptime: %s\n"
           "\n"
           "Time: %s\n"
           "Sunrise: %s\n"
           "Sunset: %s\n"
            "%s";

    char buf[1024];
    sprintf(buf, formats,
      hostname.c_str(),
      FW_VERSION,
      wifi.getIP().c_str(),
      RATE_LIMIT_TIMEOUT_MS,
      challenge_validity_timeout,
      update_mode,
      ESP.getFreeHeap(),
      uptime.getUptime().c_str(),
      mytime.stringTime().c_str(),
      Time::min2str(mytime.custom_sunrise_minutes()).c_str(),
      Time::min2str(mytime.custom_sunset_minutes()).c_str(),
      devicestatus.c_str()
    );
    return {DATA, buf};
  }

  if (message == COMMAND_REBOOT) {
    printDebug("Rebooting...");
    ESP.restart();
    return {ACK, ""}; //this will never reach the client, whatever.
  }

  if (Message::getParam(message,0) == COMMAND_READ_SETTING){
    String vault = Message::getParam(message,1);
    String key = Message::getParam(message,2);
    
    if (vault != "" && key != ""){
       PersistentMemory pmem(vault);
       if (pmem){
          String value = pmem.readString(key, "EMPTY");
          if(value != "EMPTY"){
            return {DATA, value, FLAG_KEEP_ALIVE};
          } else {
            return {ERR, "Key not found"};
          }
       } else {
        return {ERR, "Vault not found"};
       }
       
    } else if (vault != ""){
      String json = PersistentMemory::toJSON(vault);
      if (json != ""){
        return {DATA, json, FLAG_KEEP_ALIVE};
      } else {
        return {ERR, "Vault not found"};
      }
      
    } else {
      String arr[10];
      int len = PersistentMemory::listVaults(arr, sizeof(arr) / sizeof(arr[0]));
      String out = "";
      for (int i = 0; i < len; i++){
        out += arr[i] + "\n";
      }
      return {DATA, out, FLAG_KEEP_ALIVE};
    }
    return {ERR, "Unknown error"};
  }

  if (Message::getParam(message,0) == COMMAND_WRITE_SETTING){
    String vault = Message::getParam(message,1);
    String key = Message::getParam(message,2);
    String value = Message::getParam(message,3, true);
    if (vault && key && value){
       PersistentMemory pmem(vault);
       if (pmem){
          String test = pmem.readString(key, "EMPTY");
          if(test != "EMPTY"){
            pmem.writeString(key, value);
            pmem.commit();
            return {ACK, "", FLAG_KEEP_ALIVE};
          } else {
            return {ERR, "Key not found"};
          }
       } else {
        return {ERR, "Vault not found"};
       }
    }
    return {ERR, "Vault or Register missing"};
  }
  
  if (Message::getParam(message, 0) == COMMAND_SET_DEVICE_PASS) {
    String setting = Message::getParam(message, 1, true);
    int length = setting.length();
    if (length >= 8 && length <= 64){
      PersistentMemory pmem("system");
      pmem.writeString(KEY_DEVICEPASS, setting);
      pmem.commit();
      return {ACK, ""};
    }
  }

  if (Message::getParam(message, 0) == COMMAND_SET_WIFI_SSID) {
    String setting = Message::getParam(message, 1, true);
    int length = setting.length();
    if (length >= 3 && length <= 32){
      PersistentMemory pmem("wifi");
      pmem.writeString(KEY_WIFISSID, setting);
      pmem.commit();
      return {ACK, ""};
    }
  }

  if (Message::getParam(message, 0) == COMMAND_SET_WIFI_PASS) {
    String setting = Message::getParam(message, 1, true);
    int length = setting.length();
    if (length >= 8 && length <= 63){
      PersistentMemory pmem("wifi");
      pmem.writeString(KEY_WIFIPASS, setting);
      pmem.commit();
      return {ACK, ""};
    }
  }

  if (Message::getParam(message, 0) == COMMAND_SET_WIFI_MODE) {
    String setting = Message::getParam(message, 1, true);
    int length = setting.length();
    if (length >= 2 && length <= 6){
      PersistentMemory pmem("wifi");
      pmem.writeString(KEY_WIFIMODE, setting);
      pmem.commit();
      return {ACK, ""};
    }
  }

  if (message == COMMAND_SAVE) {
    printDebug("Saving settings!");
    //mem.commit();
    return {ACK, ""};
  }

  if (Message::getParam(message, 0) == COMMAND_RESET) {
    String vault = Message::getParam(message, 1);
    printDebug(vault);
    if(vault != ""){
      if(PersistentMemory::remove(vault)){
        return {ACK, ""};
      } else {
        return {ERR, "Vault not found"};
      }
    } else {
      printDebug("Clearing PMEM!");
      PersistentMemory::format();
      return {ACK, ""};
    }
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
  udpclient.beginPacket(ip, port);
  udpclient.write((uint8_t*)data.c_str(), data.length());
  udpclient.endPacket();
}

void send_Data_UDP(String data) {
  send_Data_UDP(data, udpclient.remoteIP(), udpclient.remotePort());
}

String receive_Data_UDP() {
  int packetSize = udpclient.parsePacket();
  if (packetSize) {
    char incomingPacket[512];
    int len = udpclient.read(incomingPacket, sizeof(incomingPacket) - 1);
    udpclient.flush();
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

void send_Data_TCP(WiFiClient &tcpclient, String data) {
  data = Message::wrap(data);
  printDebug("\n[TCP] Sending:\n" + data + "\n");
  tcpclient.println(data);
  yield();
}

String receive_Data_TCP(WiFiClient &tcpclient) {
  if (tcpclient.available()) {
    String incoming = tcpclient.readStringUntil('\n');
    tcpclient.flush();
    yield();
    printDebug("\n[TCP] Received:\n" + incoming);
    String body = Message::unwrap(incoming);
    return body;
  }
  return "";
}

void stopClient_TCP(WiFiClient &tcpclient) {
  rateLimit.setState(BLOCKED);
  tcpclient.stop();
  yield();
}

void doTCPServerStuff() {

  if (!tcpclient || !tcpclient.connected()) { //garbage collection
    if (tcpclient) {
      tcpclient.stop();
    }
    // wait for a client to connect
    tcpclient = tcpserver.available();
    tcpclient.setTimeout(TCP_TIMEOUT_MS);
    tcpclient.setNoDelay(true);
    tcpclient.setSync(true);
  }

  if (rateLimit.getState() == OPEN) { //Check if we are ready for a new connection

    String s = receive_Data_TCP(tcpclient);

    if (s != "") {

      Msg message; //Msg defined in Message.h

      message = Message::decrypt(s, challengeManager);

      if (message.type != NOPE) {

        if (message.type == HELLO) {
          //preparing random challenge secret
          send_Data_TCP(tcpclient, Message::encrypt(HELLO, "", message.challenge, challengeManager.generateRandomChallenge(), FLAG_NONE));
          //sending encrypted challenge to client, the client has to prove its knowledge of the correct password by encrypting the next
          //message containing our challenge. We have proven our knowledge by sending the reencrypted client_challenge back.

          //the client send a correctly encrypted second message.
        } else if (message.type == DATA) {
          ProcessMessageStruct ret = processMessage(message.data); //process his data

          //preparing next random challenge secret
          //send answer
          send_Data_TCP(tcpclient, Message::encrypt(ret.responseCode, ret.responseData, message.challenge, challengeManager.generateRandomChallenge(), ret.flags));

          //for some messages (like setSettings) we know that the client may send a following one very quickly.
          //other messages (like trigger) shouldn't be called that quickly.
          if (!(ret.flags.indexOf(FLAG_KEEP_ALIVE) != -1)) {
            stopClient_TCP(tcpclient);
          }
        } else {
          printDebug("Wrong formatted message received!");
          send_Data_TCP(tcpclient, Message::encrypt(ERR, "Nope!", "", "", FLAG_NONE));
          stopClient_TCP(tcpclient);
        }

      } else {
        printDebug("Decryption failed!");
        challengeManager.resetChallenge();
        send_Data_TCP(tcpclient, Message::encrypt(ERR, "Nope!", "", "", FLAG_NONE));
        stopClient_TCP(tcpclient);
      }
      printDebug("-------------------------------");
    }
  } else {
    tcpclient.flush();
    tcpclient.stop();
    yield();
    tcpCleanup();
  }
}

String discover = String(HEADER_HELLO) + ":::" + COMMAND_DISCOVER;

void doUDPServerStuff() {

  if (rateLimit.getState() == OPEN) { //Check if we are ready for a new connection

    String s = receive_Data_UDP();

    if (s != "") {

      if (s == discover) {
        String discoverResponse = String(HEADER_DATA) + ":::" + hostname;
        send_Data_UDP(discoverResponse);
        stopClient_UDP();
        return;
      }

      Msg message; //Msg defined in Message.h

      message = Message::decrypt(s, challengeManager);

      //check if the client send a correctly encrypted message.
      if (message.type != NOPE) {

        if (message.type == HELLO) {
          //preparing random challenge secret
          send_Data_UDP(Message::encrypt(HELLO, "", message.challenge, challengeManager.generateRandomChallenge(), FLAG_NONE));
          //sending  challenge to client, the client has to prove its knowledge of the correct password by encrypting the next
          //message containing our challenge. We have proven our knowledge by sending the reencrypted client_challenge back.

        } else if (message.type == DATA) {
          ProcessMessageStruct ret = processMessage(message.data); //process his data

          //preparing next random challenge secret
          send_Data_UDP(Message::encrypt(ret.responseCode, ret.responseData, message.challenge, challengeManager.generateRandomChallenge(), ret.flags)); //send answer

          //for some messages (like setSettings) we know that the client may send a following one very quickly.
          //other messages (like trigger) shouldn't be called that quickly.
          if (!(ret.flags.indexOf(FLAG_KEEP_ALIVE) != -1)) {
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
  if (update_mode) {
    ota.loop();
  }
  
  for(Device * d : devices){
    d->loop();
  }

  doUDPServerStuff();
  doTCPServerStuff();

#if DEBUG == 1
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    Serial.println(cmd);
    ProcessMessageStruct out = processMessage(cmd);
    Serial.printf("%s: %s\n", Message::typeToString(out.responseCode).c_str(), out.responseData.c_str());
  }
#endif
}

//Here's the entrypoint.
void setup() {
  initHardware();
  printDebug("\nLoading Firmware: " + String(FW_VERSION));
  initFS();
  initSystem();
  setupWiFi();
  printDebug("[SYS] Starting TCP server on port " + String(TCP_SERVER_PORT));
  tcpserver.begin();
  printDebug("[SYS] Starting UDP server on port " + String(UDP_SERVER_PORT));
  udpclient.begin(UDP_SERVER_PORT);
  for(Device * d : devices){
    printDebug("[SYS] Initialising device: " + d->getName());
    d->setup();
  }
  mytime.setup();
  uptime.start();
  printDebug("Free HEAP: " + String(ESP.getFreeHeap()));
  printDebug("Bootsequence finished in " + String(millis()) + "ms" + "!\n");
#if ENABLE_STATUS_LED == 1
  led.fade(StatusLED::SINGLE_ON_OFF, 2000);
#endif
}
