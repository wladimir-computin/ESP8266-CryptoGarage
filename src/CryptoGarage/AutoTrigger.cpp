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
  printDebug("AutoTrigger: " + String(tickerGoal - tickerCount));
  if(tickerCount >= tickerGoal){
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

void AutoTrigger::setGoal(int goal){
  tickerGoal = goal;
  printDebug("Loaded Autotrigger Timeout: " + String(goal, DEC));
}

void AutoTrigger::resetCounter(){
  tickerCount = 0;
}

