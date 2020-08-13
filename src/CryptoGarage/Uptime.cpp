/*
* CryptoGarage - Uptime
* 
* (implementation)
*/

#include "Uptime.h"

void Uptime::uptimeTick(void * context) {
  (*(Uptime*)context).secsUp++;
}

void Uptime::start(){
  uptimeTicker.attach(1, uptimeTick, (void*)this);
}

String Uptime::getUptime(){
  int seconds = secsUp%60;
  int minutes = (secsUp/60)%60;
  int hours = (secsUp/(60*60))%24;
  int days = secsUp/(60*60*24);

  char buf[15];
  snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%01d:%02d:%02d:%02d", days, hours, minutes, seconds);

  return String(buf);
}
