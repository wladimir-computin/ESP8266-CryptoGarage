/*
* CryptoMX-3 - OTA
* 
* Static class which parses and encrypts/decrypts custom CryptoMX-3 TCP messages.
*/

#ifndef OTA_H
#define OTA_H

#include "AllConfig.h"
#include "Debug.h"
#include <ArduinoOTA.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

#if ENABLE_STATUS_LED == 1
  #include "StatusLED.h"
#endif

class OTA{
  public:
    void start();
    void loop();

  private:
    ESP8266WebServer httpUpdateServer;
    ESP8266HTTPUpdateServer httpUpdater;
};

#endif
