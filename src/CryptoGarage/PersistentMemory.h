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

#include <EEPROM.h>

#include "Message.h"
#include "Debug.h"

//Persistent memory address map. Otherwise we don't know at which address which setting is saved.
enum MemMap {
  MEM_FIRST = 0,
  MEM_DEV_PASS = MEM_FIRST + 0,
  MEM_WIFI_SSID = MEM_DEV_PASS + 65, //the numbers are offsets in bytes
  MEM_WIFI_PASS = MEM_WIFI_SSID + 33,
  MEM_WIFI_MODE = MEM_WIFI_PASS + 64,
  MEM_AUTOTRIGGER_TIMEOUT = MEM_WIFI_MODE + 7,
  MEM_DEV_PASS_SET = MEM_AUTOTRIGGER_TIMEOUT + 2,
  MEM_WIFI_SSID_SET = MEM_DEV_PASS_SET + 1,
  MEM_WIFI_PASS_SET = MEM_WIFI_SSID_SET + 1,
  MEM_WIFI_MODE_SET = MEM_WIFI_PASS_SET + 1,
  MEM_AUTOTRIGGER_TIMEOUT_SET = MEM_WIFI_MODE_SET + 1,
  MEM_CRYPTO_PROBE = MEM_AUTOTRIGGER_TIMEOUT_SET + 2,
  MEM_CRYPTO_KEY = MEM_CRYPTO_PROBE + 90,
  MEM_LAST = MEM_CRYPTO_KEY + 46
  
};
 
class PersistentMemory {
  private:
    const size_t BYTES = 512; //Size of EEPROM reserved memory. More than enough.
    PersistentMemory();         
    PersistentMemory( const PersistentMemory& );
    PersistentMemory & operator = (const PersistentMemory &);
    
  public:

    static PersistentMemory& instance() {
      static PersistentMemory _instance;
      return _instance;
    }
    ~PersistentMemory();

    void clearEEPROM(enum MemMap start_index, enum MemMap end_index);
    void commit();
    
    String readStringFromEEPROM(enum MemMap start_index, uint16_t maxLength);
    void writeStringToEEPROM(enum MemMap start_index, uint16_t maxLength, String input);
    
    bool readBoolFromEEPROM(enum MemMap start_index);
    void writeBoolToEEPROM(enum MemMap start_index, bool input);
    
    int readIntFromEEPROM(enum MemMap start_index);
    void writeIntToEEPROM(enum MemMap start_index, int input);

    ProcessMessageStruct writeSettings(String message, int min_length, int max_length, const char * command, MemMap addr, MemMap addr_set, String type);
    
};
 
#endif
