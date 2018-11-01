/*
* CryptoGarage - Garage
* 
* (implementation)
*/

#include "Garage.h"

void Garage::loop(){
  if(gateState.getState() == STILL){
    if (autoTrigger.isFinished()) {
      triggerRelay();
    }
  }
}

void Garage::setup(){
    PersistentMemory &mem = PersistentMemory::instance();
    if (mem.readBoolFromEEPROM(MEM_AUTOTRIGGER_TIMEOUT_SET) == true) {
      autoTrigger.setEnd(mem.readIntFromEEPROM(MEM_AUTOTRIGGER_TIMEOUT));
  }
}

ProcessMessageStruct Garage::processMessage(String &message) {

  if (message == COMMAND_TRIGGER) {
    printDebug("Trigger Relay!");
    if (autoTrigger.isActive()) {
      autoTrigger.disengage();
    }
    triggerRelay();
    return {ACK, ""};
  }

  if (message == COMMAND_TRIGGER_AUTO) {
    if (!autoTrigger.isActive()) {
      //triggerRelay();
      autoTrigger.engage();
    } else {
      autoTrigger.disengage();
    }
    return {ACK, ""};
  }

  
  if (message == COMMAND_AUTOTRIGGER_PING) {
    if(autoTrigger.isActive()){
      autoTrigger.resetCounter();
      return {ACK, ""};
    }
  }

  return {ERR, "NO_COMMAND"};
}

String Garage::getStatus(){
  return "Autotrigger engaged: " + String(autoTrigger.isActive()) + "\n" +
         "Autotrigger timeout: " + String(autoTrigger.tickerEnd) + "s" + "\n" +
         "Relaystate: " + String(relay.getState());
}

void Garage::triggerRelay() {
  relay.trigger();
  gateState.trigger();
}
