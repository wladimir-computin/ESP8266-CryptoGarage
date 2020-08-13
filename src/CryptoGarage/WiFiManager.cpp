/*
* CryptoGarage - WiFiManager
* 
* (implementation)
*/

#include "WiFiManager.h"

bool WiFiManager::setCredentials(String &ssid, String &pass){
  if (!(pass.length() >= 8 && pass.length() <= 63))
    return false;
  if (!(ssid.length() >= 1 && ssid.length() <= 32))
    return false;

  this->ssid = ssid;
  this->pass = pass;

  return true;
}

bool WiFiManager::setHostname(String &hostname){
  this->hostname = hostname;

  return true;
}

void WiFiManager::setMode(WiFiModes mode){
  this->mode = mode;
}

WiFiModes WiFiManager::getMode(){
  return mode;
}

void WiFiManager::applyMode(){
  WiFi.mode(WIFI_OFF);
  delay(1);
  switch(mode){
    case AP:
      printDebug("[WiFi] Starting WiFi-AP: " + ssid + ":" + pass);
      WiFi.setPhyMode(WIFI_PHY_MODE_11B);
      WiFi.mode(WIFI_AP); 
      WiFi.softAP(ssid.c_str(), pass.c_str());
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    break;
    case CLIENT:
      printDebug("[WiFi] Connecting to: " + ssid + ":" + pass);
      WiFi.setPhyMode(WIFI_PHY_MODE_11G);
      WiFi.mode(WIFI_STA);
      WiFi.hostname(hostname.c_str());
      WiFi.begin(ssid.c_str(), pass.c_str());
    break;
    case HYBRID:
      printDebug("[WiFi] Initializing in Hybrid-Mode");
      printDebug("[WiFi] Starting WiFi-AP: " + hostname + ":" + pass);
      printDebug("[WiFi] Connecting to: " + ssid + ":" + pass);
      WiFi.setPhyMode(WIFI_PHY_MODE_11G);
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP(DEFAULT_HOSTNAME, pass.c_str());
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
      WiFi.hostname(hostname.c_str());
      WiFi.begin(ssid.c_str(), pass.c_str());
    break;
  }
}

void WiFiManager::init(){
  WiFi.persistent(false);
  
  applyMode();
}

String WiFiManager::getIP(){
  if (mode == AP){
    return WiFi.softAPIP().toString();
  } else if (mode == CLIENT){
    return WiFi.localIP().toString();
  } else {
    return WiFi.softAPIP().toString();
  }
}

String WiFiManager::mode2string(WiFiModes m){
  switch (m){
   case AP:
    return "AP";
   case CLIENT:
    return "CLIENT";
   case HYBRID:
    return "HYBRID";
  }
}

WiFiModes WiFiManager::string2mode(String m){
  m.toUpperCase();
  if(m == "AP"){
    return AP;
  }
  if(m == "HYBRID"){
    return HYBRID;
  }
  if(m == "CLIENT"){
    return CLIENT;
  }
  return AP;
}


bool emergency_hybrid_mode = false;
bool skip_disconnect_event = false;

WiFiEventHandler onStationModeConnected = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected& event) {
  printDebug("[WiFi] Connection to AP established!");
  if(emergency_hybrid_mode){
    printDebug("[WiFi] Connection to AP restored, leaving Hybrid-Mode!");
    emergency_hybrid_mode = false;
    skip_disconnect_event = true;
    WiFiManager::instance().setMode(CLIENT);
    WiFiManager::instance().applyMode();
  }
});

WiFiEventHandler onStationModeGotIP = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
  printDebug("[WiFi] Got IP from DHCP: " + WiFi.localIP().toString());
});

WiFiEventHandler onStationModeDisconnected = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
  if(!emergency_hybrid_mode && !skip_disconnect_event){
    printDebug("[WiFi] Connection to AP lost!");
    printDebug("[WiFi] Switching to Hybrid-Mode...");
    emergency_hybrid_mode = true;
    WiFiManager::instance().setMode(HYBRID);
    WiFiManager::instance().applyMode();
  } else {
    skip_disconnect_event = false;
  }
});

WiFiEventHandler onSoftAPModeStationConnected = WiFi.onSoftAPModeStationConnected([](const WiFiEventSoftAPModeStationConnected& event) {
  printDebug("[WiFi] Client connected to AP");
  if(emergency_hybrid_mode){
    WiFi.setAutoReconnect(false);
  }
});

WiFiEventHandler onSoftAPModeStationDisconnected = WiFi.onSoftAPModeStationDisconnected([](const WiFiEventSoftAPModeStationDisconnected& event) {
  printDebug("[WiFi] Client disconnected from AP");
  if(emergency_hybrid_mode){
    WiFi.setAutoReconnect(true);
  }
});
