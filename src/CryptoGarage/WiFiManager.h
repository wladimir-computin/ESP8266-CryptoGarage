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

enum WiFiModes {AP, HYBRID, CLIENT};

class WiFiManager{

  public:

    void setMode(WiFiModes mode);
    WiFiModes getMode();
    void applyMode();
    bool setCredentials(String ssid, String pass);
    void init();
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

    WiFiManager() {}
    WiFiManager( const WiFiManager& );
    WiFiManager & operator = (const WiFiManager &);
  
};

#endif
