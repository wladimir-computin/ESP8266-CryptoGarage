/*
* CryptoGarage - LCTechRelay
* 
* (implementation)
*/

#include "LCTechRelay.h"

void LCTechRelay::setState(bool state){
  if(state == true){
    blankPadding(10);
    Serial.write(RELAY_ON, sizeof(RELAY_ON));
    blankPadding(10);
    Serial.flush();
    printDebug("Relay up");
  } else {
    blankPadding(10);
    Serial.write(RELAY_OFF, sizeof(RELAY_OFF));
    blankPadding(10);
    Serial.flush();
    printDebug("Relay down");
  }
  relayState = state;
}

bool LCTechRelay::getState(){
  return relayState;
}

void LCTechRelay::blankPadding(int count){ //ugly workaround :)
  for(;count > 0; count--){
    Serial.write(0x00);
  }
}

