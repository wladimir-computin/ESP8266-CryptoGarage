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

#include "Debug.h"
#include "Device.h"
#include "Crypto.h"
#include "PersistentMemory.h"
#include "ConnectionState.h"
#include "RateLimit.h"
#include "Message.h"
#include "Uptime.h"
#if ENABLE_STATUS_LED == 1
  #include "StatusLED.h"
#endif

#include "Garage.h"

//Default settings
String WIFISSID = DEFAULT_WIFISSID; //Defined in AllConfig.h
String WIFIPASS = DEFAULT_WIFIPASS;
String DEVICEPASS = DEFAULT_DEVICEPASS;
String WIFIMODE = DEFAULT_WIFIMODE;

const char RESPONSE_OK[] = "OK";
const char RESPONSE_DATA[] = "DATA";
const char RESPONSE_ERROR[] = "FAIL";

const char COMMAND_HELLO[] = "hellofriend";
const char COMMAND_SET_DEVICE_PASS[] = "setDevicePass";
const char COMMAND_GET_STATUS[] = "getStatus";
const char COMMAND_SET_WIFI_SSID[] = "setSSID";
const char COMMAND_SET_WIFI_PASS[] = "setWIFIPass";
const char COMMAND_SET_WIFI_MODE[] = "setWIFIMode";
const char COMMAND_SET_AUTOTRIGGER_TIMEOUT[] = "setAutotriggerTimeout";
const char COMMAND_GET_SETTINGS[] = "getSettings";
const char COMMAND_SAVE[] = "save";
const char COMMAND_PING[] = "ping";
const char COMMAND_REBOOT[] = "reboot";
const char COMMAND_RESET[] = "reset";
const char COMMAND_UPDATE[] = "update";

//TCP server on port 4646
WiFiServer server(TCP_SERVER_PORT);
//TCP Client, global because we want to keep tcp connections between loops alive
WiFiClient client;

#if ENABLE_STATUS_LED == 1
  StatusLED &led = StatusLED::instance();
#endif
Uptime &uptime = Uptime::instance();

//Garage specific stuff
Garage garage;
Device * device = &garage;

//OTA update
ESP8266WebServer httpUpdateServer(HTTP_OTA_PORT);
ESP8266HTTPUpdateServer httpUpdater;
bool update_mode = false;

//Communication specific stuff
Crypto &crypt = Crypto::instance();
PersistentMemory &mem = PersistentMemory::instance();;
ConnectionState connectionState;
RateLimit rateLimit;

//Create WiFi AP
void setupWiFi() {
  printDebug("Starting WiFi-AP: " + WIFISSID + ":" + WIFIPASS);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFISSID.c_str(), WIFIPASS.c_str());
}

//Well... this is kind of tricky to explain. Nah... nevermind!
ProcessMessageStruct writeSettings(String message, int min_length, int max_length, const char * command, MemMap addr, MemMap addr_set, String type) {
  printDebug(message);
  String setting = message.substring(message.indexOf(command) + strlen(command) + 1);
  printDebug("Writing setting: " + String(command) + " : " + setting);
  if (setting.length() >= min_length && setting.length() <= max_length) {
    if (type == "string") {
      mem.writeStringToEEPROM(addr, max_length, setting);
    } else if (type == "int") {
      int i = setting.toInt();
      if (i >= 1 && i <= 9999) {
        mem.writeIntToEEPROM(addr, i);
      } else {
        return {ERR, "Timeout not in [1,9999]", true};
      }
    }
    mem.writeBoolToEEPROM(addr_set, true);
    return {ACK, "", true};
  }
  return {ERR, "Parameter length not in [" + String(min_length) + "," + String(max_length) + "]", true};
}

void initCrypt() {
  printDebug("Initialising Crypto");
  crypt.init(DEVICEPASS);
}

void loadSettings() {
  printDebug("Initialising PersistentMemory");

  //clearEEPROM(MEM_FIRST, MEM_LAST); //uncomment this to clear all saved settings on startup.

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
    WIFIMODE = mem.readStringFromEEPROM(MEM_WIFI_MODE, 1);
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

//Here's the entrypoint.
void setup() {
  initHardware();
  printDebug("\nLoading Firmware: " + String(FW_VERSION));
  loadSettings();
  initCrypt();
  setupWiFi();
  server.begin();
  device->setup();
  uptime.start();
  printDebug("Free HEAP: " + String(ESP.getFreeHeap()));
  printDebug("Bootsequence finished in " + String(millis()) + "ms" + "!\n");
  #if ENABLE_STATUS_LED == 1
    led.fade(StatusLED::SINGLE_ON_OFF, 2000);
  #endif
}


//Here we process the plaintext commands and generate an answer for the client.
ProcessMessageStruct processMessage(String &message) {
  if (message == COMMAND_HELLO) {
    return {ACK, "", true};
  }

  ProcessMessageStruct p = device->processMessage(message);
  if (!(p.responseCode == ERR && p.responseData == "NO_COMMAND")) {
    return p;
  }

  if (message == COMMAND_PING) {
      return {ERR, ""};
  }

  if (message == COMMAND_GET_STATUS) {
    String data = "FW-Version: " + String(FW_VERSION) + "\n\n" +
                  //"Autotrigger engaged: " + String(autoTrigger.isActive()) + "\n" +
                  //"Autotrigger timeout: " + String(autoTrigger.tickerEnd) + "s" + "\n" +
                  "Ratelimit: " + RATE_LIMIT_TIMEOUT_MS + "ms" + "\n" +
                  //"Relaystate: " + String(relay.getState()) + "\n" +
                  "Updatemode: " + String(update_mode) + "\n" +
                  "Free HEAP: " + String(ESP.getFreeHeap()) + "Byte" + "\n" +
                  device->getStatus() + "\n\n" +
                  "Uptime: " + uptime.getUptime();
    return {DATA, data, true};
  }

  if (message == COMMAND_REBOOT) {
    printDebug("Rebooting...");
    ESP.restart();
    return {ACK, ""}; //this will never reach the client, whatever.
  }

  if (message.startsWith(COMMAND_SET_DEVICE_PASS)) {
    return writeSettings(message, 8, 64, COMMAND_SET_DEVICE_PASS, MEM_DEV_PASS, MEM_DEV_PASS_SET, "string");
  }

  if (message.startsWith(COMMAND_SET_WIFI_SSID)) {
    return writeSettings(message, 3, 32, COMMAND_SET_WIFI_SSID, MEM_WIFI_SSID, MEM_WIFI_SSID_SET, "string");
  }

  if (message.startsWith(COMMAND_SET_WIFI_PASS)) {
    return writeSettings(message, 8, 63, COMMAND_SET_WIFI_PASS, MEM_WIFI_PASS, MEM_WIFI_PASS_SET, "string");
  }

  if (message.startsWith(COMMAND_SET_WIFI_MODE)) {
    return writeSettings(message, 1, 1, COMMAND_SET_WIFI_MODE, MEM_WIFI_MODE, MEM_WIFI_MODE_SET, "string");
  }

  if (message.startsWith(COMMAND_SET_AUTOTRIGGER_TIMEOUT)) {
    return writeSettings(message, 1, 4, COMMAND_SET_AUTOTRIGGER_TIMEOUT, MEM_AUTOTRIGGER_TIMEOUT, MEM_AUTOTRIGGER_TIMEOUT_SET, "int");
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
    if (!update_mode) {
      printDebug("Enabling Updatemode!");
      httpUpdater.setup(&httpUpdateServer, UPDATE_PATH);
      httpUpdateServer.begin();
      update_mode = true;
      #if ENABLE_STATUS_LED == 1
        led.fade(StatusLED::PERIODIC_FADE, 2000);
      #endif
    }
    return {DATA, String("http://192.168.4.1:") + HTTP_OTA_PORT + UPDATE_PATH}; //IP static for now...
  }

  printDebug("Received unknown command:" + message);
  return {ERR, "Unknown Command"};
}

String receive_Data(WiFiClient &client) {
  if (client.available()) {
    String httpMessage = client.readStringUntil('\n');
    printDebug("\nReceived:\n" + httpMessage);
    String body = Message::unwrap(httpMessage);
    return body;
  }
  return "";
}

void send_Data(WiFiClient &client, String data) {
  data = Message::wrap(data);
  printDebug("\nSending:\n" + data + "\n");
  client.print(data + "\r\n");
  client.flush();
}

void stopClient(WiFiClient &client) {
  rateLimit.setState(BLOCKED);
  client.stop();
  yield();
}

uint8_t iv_challenge[AES_GCM_IV_LEN]; //global, because we need to preserve the value between loops.

void doTCPServerStuff() {

  if (!client || !client.connected()) { //garbage collection
    if (client) {
      client.stop();
    }
    // wait for a client to connect
    client = server.available();
  }

  if (rateLimit.getState() == OPEN) { //Check if we are ready for a new connection

    //printDebug("[Client connected]");
    client.setTimeout(TCP_TIMEOUT_MS);
    client.setNoDelay(true);

    String s = receive_Data(client);

    if (s != "") {

      Msg message; //Msg defined in Message.h

      switch (connectionState.getState()) {
        case NONE:
          message = Message::decrypt(s, NULL); //IV is inside s
          break;
        case PHASE2:
          message = Message::decrypt(s, iv_challenge); //s doesn't contains the IV as it was send as challenge secret in the previous message
          break;
      }


      if (message.type == DATA) {

        switch (connectionState.getState()) {
          case NONE:
            if (message.data == COMMAND_HELLO) {
              printDebug("Sending IV challenge");
              crypt.getRandomIV(iv_challenge); //preparing random challenge secret
              send_Data(client, Message::encrypt(DATA, iv_challenge, sizeof(iv_challenge)));
              //sending encrypted challenge IV to client, the client has to prove its knowledge of the correct password by encrypting the next
              //message with the passsword + the decrypted challenge IV.
              connectionState.setState(PHASE2); //the client has only a certain amount of time to answer (and only one guess.)
            } else {
              send_Data(client, Message::encrypt(ERR, "Y u no greet me?!"));
              stopClient(client); //the client didn't know how to talk to us.
            }
            break;

          case PHASE2:
            connectionState.setState(NONE); //the client send a correctly encrypted second message.
            ProcessMessageStruct ret = processMessage(message.data); //process his data
            send_Data(client, Message::encrypt(ret.responseCode, ret.responseData)); //send answer

            //for some messages (like setSettings) we know that the client may send a following one very quickly.
            //other messages (like trigger) shouldn't be called that quickly.
            if (!ret.noRateLimit) {
              stopClient(client);
            }

            printDebug("-------------------------------");
            break;
        }
      } else {
        printDebug("Decryption failed!");
        send_Data(client, Message::encrypt(ERR, "Nope!"));
        stopClient(client);
      }
    }
  } else {
    client.stop();
    yield();
    tcpCleanup();
  }
}


void loop() {
  doTCPServerStuff();

  device->loop();

  if (update_mode) {
    httpUpdateServer.handleClient();
  }
}
