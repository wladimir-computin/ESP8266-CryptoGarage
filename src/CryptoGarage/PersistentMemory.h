/*
* CryptoGarage - PersistentMemory
* 
* We need persistent memory to remember settings (like WiFi SSID and passphrase) after reboots.
* This class contains helper/wrapper functions. They simplify the usage of the EEPROM library, which is just some sort of bytewise access to flash memory.
* Don't forget to call .commit() after finishing your memory writing operations, otherwise any changes will be lost after reboot.
*/

#ifndef PMEM_H
#define PMEM_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "Message.h"
#include "Debug.h"
 
class PersistentMemory {
  private:
    DynamicJsonDocument json;
    String filename;
    bool initsuccess = false;
    bool createifnotfound;
    bool changed = false;
    
  public:

    PersistentMemory(String settingsfile, bool create=false);
    ~PersistentMemory();

    explicit operator bool();

    static void format();
    static bool remove(String &vault);
    static int listVaults(String * arr, int size);
    static String toJSON(String &vault);

    void commit();
    
    String readString(String &key, String fallback="");
    String readString(const char * key, String fallback="");
    void writeString(String &key, String value);
    void writeString(const char * key, String value);
    
    bool readBool(String &key, bool fallback=false);
    bool readBool(const char * key, bool fallback=false);
    void writeBool(String &key, bool value);
    
    int readInt(String &key, int fallback=0);
    int readInt(const char * key, int fallback=0);
    void writeInt(String &key, int value);

    ProcessMessageStruct writeSettings(String &message, String type);
    
};
 
#endif
