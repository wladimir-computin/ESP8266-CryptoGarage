/*
* CryptoGarage - PersistentMemory
* 
* (implementation)
*/

#include "PersistentMemory.h"

PersistentMemory::PersistentMemory(){
  EEPROM.begin(BYTES);
}

PersistentMemory::~PersistentMemory(){
  EEPROM.end();
}

void PersistentMemory::clearEEPROM(enum MemMap start_index, enum MemMap end_index) {
  for (int i = start_index; i <= end_index; i++) {
    EEPROM.write(i, 0x00);
  }
  commit();
  printDebug("Cleared EEPROM");
}

void PersistentMemory::commit(){
  EEPROM.commit();
  yield();
}

String PersistentMemory::readStringFromEEPROM(enum MemMap start_index, uint16_t maxLength) {
  String out;
  char c;
  for (uint16_t i = 0; i < maxLength; i++) {
    c = EEPROM.read(i + start_index);
    if (c == '\0') {
      break;
    }
    out += c;
  }
  return out;
}

void PersistentMemory::writeStringToEEPROM(enum MemMap start_index, uint16_t maxLength, String input) {
  uint16_t i = 0;
  const char * arr = input.c_str();
  for (; i < maxLength && i <= input.length(); i++) {
    EEPROM.write(i + start_index, arr[i]);
  }
}

bool PersistentMemory::readBoolFromEEPROM(enum MemMap start_index) {
  byte c = EEPROM.read(start_index);
  if (c == 1) {
    return true;
  } else {
    return false;
  }
}

void PersistentMemory::writeBoolToEEPROM(enum MemMap start_index, bool input) {
  EEPROM.write(start_index, input == true ? 1 : 0);
}

int PersistentMemory::readIntFromEEPROM(enum MemMap start_index) {
  byte low, high;
  low = EEPROM.read(start_index);
  high = EEPROM.read(start_index + 1);
  return low + ((high << 8) & 0xFF00);
}

void PersistentMemory::writeIntToEEPROM(enum MemMap start_index, int input) {
  byte low, high;
  low = input & 0xFF;
  high = (input >> 8) & 0xFF;
  EEPROM.write(start_index, low);
  EEPROM.write(start_index + 1, high);
}
