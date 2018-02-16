/*
* CryptoGarage - Crypto
* 
* (implementation)
*/

#include "Crypto.h"

void Crypto::init(String devicepass){
  generateSHA256Key(devicepass);
}

void Crypto::generateSHA256Key(String devicepass) {
  printDebug("Generating SHA256-Key");
  SHA256 sha256;
  sha256.update(devicepass.c_str(), devicepass.length());
  sha256.finalize(shaKey, sizeof(shaKey));
  printDebug("SHA256-Key=" + getShaKey_b64());
}

void Crypto::getRandomIV(uint8_t * iv){
  ESP8266TrueRandom.memfill((char*)iv, AES_GCM_IV_LEN);
}

void Crypto::encryptData(String &data, uint8_t * iv, uint8_t * tag, uint8_t * out) {
  uint8_t in[data.length()];
  data.getBytes(in, sizeof(in));
  encryptData(in, sizeof(in), iv, tag, out);
}

void Crypto::encryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag, uint8_t * out) {
  GCM<AES256> gcm;
  gcm.setKey(shaKey, sizeof(shaKey));
  gcm.setIV(iv, AES_GCM_IV_LEN);
  gcm.encrypt(out, data, dataLen);
  gcm.computeTag(tag, AES_GCM_TAG_LEN);
}

void Crypto::decryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag, uint8_t * out) {
  GCM<AES256> gcm;
  gcm.setKey(shaKey, sizeof(shaKey));
  gcm.setIV(iv, AES_GCM_IV_LEN);
  gcm.decrypt(out, data, dataLen);
  if (!gcm.checkTag(tag, AES_GCM_TAG_LEN)) { //data corrupted or tampered, throw away!
    memset(out, '\0', dataLen);
  }
}

String Crypto::decryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag) {
  uint8_t out[dataLen + 1];
  decryptData(data, dataLen, iv, tag, out);
  out[dataLen] = '\0'; //never forget the null terminator, ever!
  return String((char*)out);
}

String Crypto::bytesToBase64(uint8_t * bytes, int len) {
  uint8_t temp[base64.encodedLength(len)];
  base64.encode(temp, bytes, len);
  return String((char*)temp);
}

void Crypto::base64ToBytes(String &in, uint8_t * out) {
  base64.decode(out, (char*)in.c_str(), in.length());
}

uint16_t Crypto::base64DecodedLength(String &b64){
  return base64.decodedLength((char*)b64.c_str(), b64.length());
}

const uint8_t * Crypto::getShaKey(){
  return shaKey;
}

String Crypto::getShaKey_b64(){
  return bytesToBase64(shaKey, sizeof(shaKey));
}

