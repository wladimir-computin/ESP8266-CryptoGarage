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

  char buff[20];
  snprintf(buff, sizeof(buff), "%01d:%02d:%02d:%02d", days, hours, minutes, seconds); //Buffer overflow after 1000000000 days... getStatus = crash... fine.

  return String(buff);
}
