/*
* CryptoGarage - AllConfig
* Compiletime constants and defines.
* 
* Arduino IDE processes files in alphabetical order, hence the defines MUST be in a file named beginning with an "A"...
*/

#ifndef ALLCONFIG
#define ALLCONFIG

  #define DEBUG 0   //Enable debug output. If you have a LCTech relay, you must disable this or the relay won't work.
  #define RELAYLCTECH 0   //either this
  #define RELAYWEMOS 1    //or that
  #define ENABLE_STATUS_LED 1   //Works only if the builtin LED is not connected to the same pin as the relay. Disable when using ESP-01(s) with LCTech relay

  const int TCP_SERVER_PORT = 4646;
  const int HTTP_OTA_PORT = 8266;
  const int CONNECTION_STATE_TIMEOUT = 2; //Seconds which the client has to answer the challenge message.
  const int RATE_LIMIT_TIMEOUT_MS = 200; //Milliseconds between communication attempts, set to 0 to disable rate limiting.
  const int TCP_TIMEOUT_MS = 50;  //not sure why, but smaller values behave faster and more stable.
  const int RELAY_TRIGGER_TIME_MS = 250;  //click ...time in ms... clack
  const int AUTOTRIGGER_TIMEOUT_DEFAULT = 10; //Seconds after last succesfull ping.
  const int AES_GCM_TAG_LEN = 16; //Recommended value is 16. Don't change unless you understand how AES-GCM works.
  const int AES_GCM_IV_LEN = 12; //Recommended value is 12.

  const char UPDATE_PATH[] = "/update";

  const char DEFAULT_WIFISSID[] = "GarageTest";
  const char DEFAULT_WIFIPASS[] = "12345670";
  const char DEFAULT_DEVICEPASS[] = "TestTest1";

  const char KEY_SALT[] = "FTh.!%B$";
  const int SHA_ROUNDS = 5000;

  const char FW_VERSION[] = "4.0";

#endif
