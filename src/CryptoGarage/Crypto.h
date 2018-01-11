#ifndef CRYPTO_H
#define CRYPTO_H

#include <GCM.h>
#include <AES.h>
#include <SHA256.h>
#include <ESP8266TrueRandom.h>

#include "Debug.h"
#include "cBase64.h"
 
class Crypto {
  
  private:
      uint8_t shaKey[32];

      void generateSHA256Key(String devicepass);

   
  public:
      void init(String devicepass);

      void getRandomIV12(uint8_t * iv);
   
      void encryptData(String data, uint8_t * iv, uint8_t * tag, uint8_t * out);
      void encryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag, uint8_t * out);
      
      void decryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag, uint8_t * out);
      String decryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag);
      
      String bytesToBase64(uint8_t * bytes, int len);
      void base64ToBytes(String in, uint8_t * out);
      uint16_t base64DecodedLength(String b64);
   
      const uint8_t * getShaKey();
      String getShaKey_b64();
};
 
#endif
