/*
* CryptoGarage - Crypto
* 
* (implementation)
*/

#include "Crypto.h"
#include "PersistentMemory.h"
 //bug in compiler? Can't move to Crypto.h

void Crypto::init(String devicepass){
  printDebug("[Crypto] Initialising with Password " + devicepass);
  keyDerivationFunction(devicepass);
}

void Crypto::keyDerivationFunction(String devicepass) { //bcrypt would be better, but I haven't found any implementation for Arduino/ESP yet.
  SHA512 sha512;
  String salted = devicepass + KEY_SALT;
  uint8_t sha512hash[sha512.hashSize()];
  sha512.update(salted.c_str(), salted.length());
  sha512.finalize(sha512hash, sizeof(sha512hash) / sizeof(sha512hash[0]));
  PersistentMemory pmem("crypto", true);
  String probe_b64 = pmem.readString(KEY_CRYPTO_PROBE, "EMPTY");
  String sha512_probe_b64 = bytesToBase64(sha512hash, sizeof(sha512hash) / sizeof(sha512hash[0]));
  if(probe_b64 != sha512_probe_b64){
    printDebug("[Crypto] Generating AES256-Key first time...");
    for(int i = 0; i < SHA_ROUNDS; i++){
      sha512.reset();
      sha512.update(sha512hash, sizeof(sha512hash) / sizeof(sha512hash[0]));
      sha512.finalize(sha512hash, sizeof(sha512hash) / sizeof(sha512hash[0]));
    }
    pmem.writeString(KEY_CRYPTO_KEY, bytesToBase64(sha512hash, sizeof(sha512hash) / sizeof(sha512hash[0])));
    pmem.writeString(KEY_CRYPTO_PROBE, sha512_probe_b64);
    pmem.commit();
  } else {
    printDebug("[Crypto] Loading cached AES256-Key");
    String sha512_b64 = pmem.readString(KEY_CRYPTO_KEY);
    base64ToBytes(sha512_b64, sha512hash);
  }
  memcpy(aesKey, sha512hash, AES256_KEY_LEN);
  
  printDebug("[Crypto] AES256-Key=" + getAES256Key_b64());
}

void Crypto::getRandomIV(uint8_t * iv){
  ESP8266TrueRandom.memfill((char*)iv, AES_GCM_IV_LEN);
}

void Crypto::getRandomChallenge(uint8_t * challenge){
  ESP8266TrueRandom.memfill((char*)challenge, CHALLENGE_LEN);
}

String Crypto::getRandomChallengeBase64(){
  uint8_t challenge[CHALLENGE_LEN];
  getRandomChallenge(challenge);
  String challenge_b64 = bytesToBase64(challenge, sizeof(challenge) / sizeof(challenge[0]));
  return challenge_b64;
}

void Crypto::encryptData(const String &data, uint8_t * iv, uint8_t * tag, uint8_t * out) {
  encryptData((uint8_t*) data.c_str(), data.length(), iv, tag, out);
}

void Crypto::encryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag, uint8_t * out) {
  GCM<AES256> gcm;
  gcm.setKey(aesKey, sizeof(aesKey) / sizeof(aesKey[0]));
  gcm.setIV(iv, AES_GCM_IV_LEN);
  gcm.encrypt(out, data, dataLen);
  gcm.computeTag(tag, AES_GCM_TAG_LEN);
}

void Crypto::decryptData(uint8_t * data, int dataLen, uint8_t * iv, uint8_t * tag, uint8_t * out) {
  GCM<AES256> gcm;
  gcm.setKey(aesKey, sizeof(aesKey) / sizeof(aesKey[0]));
  gcm.setIV(iv, AES_GCM_IV_LEN);
  gcm.decrypt(out, data, dataLen);
  if (!gcm.checkTag(tag, AES_GCM_TAG_LEN)) { //data corrupted or tampered, throw away!
    printDebug("[Crypto] TAG missmatch, packet corrupted!");
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
  uint8_t temp[base64.encodedLength(len)+1];
  base64.encode(temp, bytes, len);
  return String((char*)temp);
}

void Crypto::base64ToBytes(const String &in, uint8_t * out) {
  base64.decode(out, (char*)in.c_str(), in.length());
}

String Crypto::base64ToBytes(const String &in) {
  uint8_t temp[base64DecodedLength(in) + 1];
  base64ToBytes(in, temp);
  temp[(sizeof(temp) / sizeof(temp[0]))- 1] = '\0';
  return String((char*)temp);
}

uint16_t Crypto::base64DecodedLength(const String &b64){
  return base64.decodedLength((char*)b64.c_str(), b64.length());
}

const uint8_t * Crypto::getAES256Key(){
  return aesKey;
}

String Crypto::getAES256Key_b64(){
  return bytesToBase64(aesKey, sizeof(aesKey) / sizeof(aesKey[0]));
}
