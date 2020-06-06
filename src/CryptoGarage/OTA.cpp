/*
  CryptoMX-3 - OTA

  (implementation)
*/

#include "OTA.h"

void OTA::start(){
  printDebug("Enabling OTA!");
  ArduinoOTA.setPort(ARDUINO_OTA_PORT);
  ArduinoOTA.setHostname(DEFAULT_HOSTNAME);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  ArduinoOTA.onStart([]() {
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      printDebug("Update started...");
      SPIFFS.end();
      StatusLED::instance().fadeStop();
      StatusLED::instance().setVal(255);
    });
  ArduinoOTA.onEnd([]() {
      printDebug("Done!");
    });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      printfDebug("Progress: %u%%\r", (progress / (total / 100)));
      StatusLED::instance().setVal(255 - progress / (total / 255));
    });
  ArduinoOTA.onError([](ota_error_t error) {
      printfDebug("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR){ printDebug("Auth Failed");}
      else if (error == OTA_BEGIN_ERROR){ printDebug("Begin Failed");}
      else if (error == OTA_CONNECT_ERROR){ printDebug("Connect Failed");}
      else if (error == OTA_RECEIVE_ERROR){ printDebug("Receive Failed");}
      else if (error == OTA_END_ERROR){ printDebug("End Failed");}
    });

  ArduinoOTA.begin();

  httpUpdater.setup(&httpUpdateServer, UPDATE_PATH);
  httpUpdateServer.begin(HTTP_OTA_PORT);

  #if ENABLE_STATUS_LED == 1
    StatusLED::instance().fade(StatusLED::PERIODIC_FADE, 2000);
  #endif
}

void OTA::loop(){
  ArduinoOTA.handle();
  httpUpdateServer.handleClient();
}
