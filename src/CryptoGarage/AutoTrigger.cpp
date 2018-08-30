/*
* CryptoGarage - AutoTrigger
* 
* (implementation)
*/

#include "AutoTrigger.h"

void AutoTrigger::AutoTriggerTick(void * context) {
  (*(AutoTrigger*)context).AutoTriggerTickStuff();
}

void AutoTrigger::AutoTriggerTickStuff(){
  tickerCount++;
  printDebug("AutoTrigger: " + String(tickerEnd - tickerCount));
  if(tickerCount >= tickerEnd){
    finished = true;
    disengage();
  }
}

void AutoTrigger::engage() {
  printDebug("Engaging AutoTrigger");
  active = true;
  finished = false;
  AutoTriggerTicker.attach(1, AutoTriggerTick, (void*)this);
  #if ENABLE_STATUS_LED == 1
    led.fade(StatusLED::PERIODIC_FADE, 1000);
  #endif
}

void AutoTrigger::disengage() {
  printDebug("Disengaging AutoTrigger");
  active = false;
  AutoTriggerTicker.detach();
  tickerCount = 0;
  #if ENABLE_STATUS_LED == 1
    led.setState(false);
  #endif
}

bool AutoTrigger::isActive(){
  return active;
}

bool AutoTrigger::isFinished(){
  bool temp = finished;
  finished = false;
  return temp;
}

void AutoTrigger::setEnd(int seconds){
  tickerEnd = seconds;
  printDebug("Loaded Autotrigger Timeout: " + String(seconds, DEC));
}

void AutoTrigger::resetCounter(){
  tickerCount = 0;
}

