/*
* CryptoGarage - GarageGate2
* 
* (implementation)
*/

#include "GarageGate2.h"

void GarageGate::gateTickerTick(void * context) {
  (*(GarageGate*)context).gateTickerTickStuff();
}

void GarageGate::gateTickerTickStuff(){
  gateTicker.detach();
  switch(gateState){
		case GATE_OPENING:
			gateState = GATE_OPEN;
		break;
		
		default:
		break;
  }
}

state GarageGate::getState(){
  return gateState;
}

String GarageGate::getStateStr(){
  const char * stateStr[] = {"GATE_NONE", "GATE_CLOSED", "GATE_OPENING", "GATE_OPEN", "GATE_CLOSING", "GATE_STOPPED_OPENING", "GATE_STOPPED_CLOSING"};
  return stateStr[gateState];
}

void GarageGate::trigger(){
  gateTicker.detach();
  switch (gateState){
		case GATE_CLOSED:
		break;
		
		case GATE_OPENING:
			gateState = GATE_STOPPED_OPENING;
		break;
		
		case GATE_OPEN:
			gateState = GATE_CLOSING;
		break;
		
		case GATE_CLOSING:
			gateState = GATE_STOPPED_CLOSING;
		break;
		
		case GATE_STOPPED_OPENING:
			gateState = GATE_CLOSING;
		break;
		
		case GATE_STOPPED_CLOSING:
			gateTicker.attach(movingTimeSeconds, gateTickerTick, (void*)this);
			gateState = GATE_OPENING;
		break;
		
		default:
		break;
  }
}

void GarageGate::updateState(int read){
	gateTicker.detach();
	switch (read){
		case 1:
			gateState = GATE_CLOSED;
		break;
		
		case 0:
			if (gateState != GATE_NONE){
				gateTicker.attach(movingTimeSeconds, gateTickerTick, (void*)this);
				gateState = GATE_OPENING;
			} else {
				gateState = GATE_OPEN;
			}
		break;
	}
}


void GarageGate::loop(){
	int read = digitalRead(D7);
	if(read != lastRead){
		updateState(read);
		lastRead = read;
	}
}


void GarageGate::setup(){
	pinMode(GARAGE_GATE_STATUS_PIN, INPUT_PULLUP);
}
