/*
* CryptoGarage - WiFiManager
* 
* Class for managing WiFi Access Point and Client Mode
* All timings defined in AllConfig.h
*/

#ifndef WIFIMAN_H
#define WIFIMAN_H

#include <ESP8266WiFi.h>
#include "Debug.h"

const char KEY_WIFISSID[] = "ssid";
const char KEY_WIFIPASS[] = "pass";
const char KEY_WIFIMODE[] = "mode";

enum WiFiModes {AP, HYBRID, CLIENT};

class WiFiManager{

  public:

    void setMode(WiFiModes mode);
    WiFiModes getMode();
    void applyMode();
    bool setCredentials(String &ssid, String &pass);
    bool setHostname(String &hostname);
    void init();
    String getIP();
    String mode2string(WiFiModes m);
    WiFiModes string2mode(String m);
    
    static WiFiManager& instance() {
      static WiFiManager _instance;
      return _instance;
    }
    ~WiFiManager() {}

  private:

    WiFiModes mode = AP;
    String ssid;
    String pass;
    String hostname;

    WiFiManager() {}
    WiFiManager( const WiFiManager& );
    WiFiManager & operator = (const WiFiManager &);
  
};

#endif
