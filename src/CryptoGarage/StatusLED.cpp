/*
* CryptoGarage - StatusLED
* 
* (implementation)
*/

#include "StatusLED.h"

void StatusLED::setState(bool state){
  fadeStop();
  int cur = state ? 255 : 0;
  setVal(cur);
}

void StatusLED::setVal(int val){
  analogWrite(ledPin, conv1023(val));
  current = val;
}

void StatusLED::fadeToVal(int val, int time_ms){
  fadeToVal(val, time_ms, NULL);
}

void StatusLED::fadeToVal(int val, int time_ms, void (*callback)()){
  argument.old_current = current;
  argument.goal = val;
  argument.time_ms = time_ms;
  argument.ms_passed = 0;
  ledTicker.attach_ms(1000/60, ledTickerTick, (void*)callback); 
}

void StatusLED::fade(FadeMode fademode, int time_ms){
  argument.fademode = fademode;
  switch(fademode){
    case SINGLE_ON_OFF:
      fadeToVal(255, time_ms / 2);
    break;
    case SINGLE_OFF_ON:
      fadeToVal(0, time_ms / 2);
    break;
    case PERIODIC_FADE:
      if(current == 0)
        fadeToVal(255, time_ms / 2);
      else
        fadeToVal(0, time_ms / 2);
    break;
  }
}

void StatusLED::ledTickerTick(void * callback){
  StatusLED::instance().ledTickerTick2();
}

void StatusLED::ledTickerTick2(){
  if(argument.ms_passed >= argument.time_ms){
    ledTicker.detach();
    setVal(argument.goal);
    fadeCompleted();
  } else {
    double m = (double)(argument.goal - argument.old_current) / (double)argument.time_ms;
    int new_current = (int)(m * (double)(argument.ms_passed) + argument.old_current);
    argument.ms_passed += (1000/60);
    setVal(new_current);
  }
}

//Convert range pwm from [0,255] to [1023,1]
int StatusLED::conv1023(int val){
  int ret = (int)(-0.015717*(val*val)) + 1023;
  if(ret > MINLED)
    ret = MINLED;
  if(ret < MAXLED)
    ret = MAXLED;
  
  return ret;
}

void StatusLED::fadeCompleted(){
  switch(argument.fademode){
    case NONE:
    
    break;
    
    case SINGLE_ON_OFF:
      argument.fademode = NONE;
      fadeToVal(0, argument.time_ms);
    break;
    
    case SINGLE_OFF_ON:
      argument.fademode = NONE;
      fadeToVal(255, argument.time_ms);
    break;
    
    case PERIODIC_FADE:
      if(current == 0)
        fadeToVal(255, argument.time_ms);
      else
        fadeToVal(0, argument.time_ms);
    break;
  }
}

void StatusLED::fadeStop(){
  argument.fademode = NONE;
  ledTicker.detach();
}
