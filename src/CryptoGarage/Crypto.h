/*
* CryptoGarage - Crypto
* 
* Class where all the crypto magic happens.
* CryptoGarage uses a challenge-response system based on AES-GCM and SHA256 for key generation.
* Challenge-response has some major benefits over a rolling code bases exchange, for example
* immunity to replay-attacks and easy managment of multiple remote devices, in our case smartphones.
* 
* Based on two highly awesome libraries:
* 
* # Arduino Cryptographic Library by Southern Storm Software, Pty Ltd.
* https://rweather.github.io/arduinolibs/crypto.html
* 
* # ESP8266TrueRandom by Peter Knight and Marvin Roger
* https://github.com/marvinroger/ESP8266TrueRandom
* 
*/

#ifndef CRYPTO_H
#define CRYPTO_H

#include <GCM.h>
#include <AES.h>
#include <SHA256.h>
#include <ESP8266TrueRandom.h>

#include "AllConfig.h"
#include "Debug.h"
#include "cBase64.h"
 
class Crypto {
  
  private:
      uint8_t shaKey[32];

      void generateSHA256Key(String devicepass);

   
  public:
  
      //on initialize the devicepass gets hashed with SHA256 and is used as AES key
      void init(String devicepass);
      
      //fill the passed iv with random bytes
      void getRandomIV(uint8_t * iv);

      //encrypt a String, using an IV and writes the tag into tag and the encrypted byte array into out
      void encryptData(String &data, uint8_t * iv, uint8_t * tag, uint8_t * out);
      
      //encrypt a byte array, using an IV writes the tag into tag and the encrypted byte array into out
      void encryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag, uint8_t * out);

      //decrypt a byte array using an IV and a tag, writes output into out
      void decryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag, uint8_t * out);

      //decrypt a byte array using an IV and a tag, return a String.
      String decryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag);
      
      String bytesToBase64(uint8_t * bytes, int len);
      void base64ToBytes(String &in, uint8_t * out);
      uint16_t base64DecodedLength(String &b64);

      //get SHA256(devicepass) as byte array
      const uint8_t * getShaKey();

      //get base64(SHA256(devicepass)) as String
      String getShaKey_b64();
};
 
#endif
