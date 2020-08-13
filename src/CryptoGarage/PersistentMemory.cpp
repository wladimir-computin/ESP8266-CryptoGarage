/*
  CryptoGarage - PersistentMemory

  (implementation)
*/

#include "PersistentMemory.h"


PersistentMemory::PersistentMemory(String settingsfile, bool create) : filename("/" + settingsfile + ".json"), json(512) {
  printDebug("[PMEM] Open vault " + settingsfile);
  File file = LittleFS.open(filename, "r");
  if (!file && create) {
    initsuccess = true;
    changed = true;
  } else if (file && !deserializeJson(json, file)) {
    initsuccess = true;
  }
  file.close();
  createifnotfound = create;
}

PersistentMemory::~PersistentMemory() {}

PersistentMemory::operator bool() {
  return initsuccess;
}

void PersistentMemory::format() {
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    if (dir.fileName().endsWith(".json")) {
      LittleFS.remove(dir.fileName());
    }
  }
  printDebug("[PMEM] Format!");
}

bool PersistentMemory::remove(String &vault) {
  String name = vault + ".json";
  printDebug("[PMEM] Removing vault " + vault);
  return LittleFS.remove(name); 
}

int PersistentMemory::listVaults(String * arr, int size) {
  Dir dir = LittleFS.openDir("/");
  int i = 0;
  while (dir.next()) {
    if (dir.fileName().endsWith(".json")) {
      if(i < size){
        String out = dir.fileName();
        out.replace(".json", "");
        arr[i++] = out;
      }
    }
  }
  return i;
}

String PersistentMemory::toJSON(String &vault) {
  File file = LittleFS.open(vault + ".json", "r");
  String out = "";
  int size = file.size();
  if (size){
    uint8_t buf[size + 1];
    file.read(buf, size);
    buf[size] = '\0';
    out = (char*)buf;
  }
  file.close();
  return out;
}

void PersistentMemory::commit() {
  if (changed) {
    File file = LittleFS.open(filename, "w");
    serializeJsonPretty(json, file);
    file.close();
    yield();
  }
}

String PersistentMemory::readString(String &key, String fallback) {
  return readString(key.c_str(), fallback);
}

String PersistentMemory::readString(const char * key, String fallback) {
  JsonVariant data = json[key];
  if (!data.isNull()) {
    return data;
  } else {
    if (createifnotfound) {
      writeString(key, fallback);
    }
    return fallback;
  }
}

void PersistentMemory::writeString(const char * key, String value) {
  json[key] = value;
  changed = true;
}

void PersistentMemory::writeString(String &key, String value) {
  return writeString(key.c_str(), value);
}

bool PersistentMemory::readBool(String &key, bool fallback) {
  return readBool(key.c_str(), fallback);
}

bool PersistentMemory::readBool(const char * key, bool fallback) {
  JsonVariant data = json[key];
  if (!data.isNull()) {
    return data;
  } else {
    if (createifnotfound) {
      json[key] = fallback;
      changed = true;
    }
    return fallback;
  }
}

void PersistentMemory::writeBool(String &key, bool value) {
  json[key] = value;
  changed = true;
}

int PersistentMemory::readInt(String &key, int fallback) {
  return readInt(key.c_str(), fallback);
}

int PersistentMemory::readInt(const char * key, int fallback) {
  JsonVariant data = json[key];
  if (!data.isNull()) {
    return data;
  } else {
    if (createifnotfound) {
      json[key] = fallback;
      changed = true;
    }
    return fallback;
  }
}

void PersistentMemory::writeInt(String &key, int value) {
  json[key] = value;
  changed = true;
}

//Well... this is kind of tricky to explain. Nah... nevermind!
ProcessMessageStruct PersistentMemory::writeSettings(String & message, String type) {
  int index = message.indexOf(':');
  String reg = message.substring(0, index);
  String value = message.substring(index + 1);
  printDebug("[PMEM] " + String(reg) + " = " + value);
  if (type == "string") {
    writeString(reg, value);
  } else if (type == "int") {
    int i = value.toInt();
    writeInt(reg, i);
  }
  else if (type == "bool") {
    bool b = value.toInt();
    writeBool(reg, b);
  }
  return {ACK, "", FLAG_KEEP_ALIVE};
}
