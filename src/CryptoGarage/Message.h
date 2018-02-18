/*
* CryptoGarage - Message
* 
* Static class which parses and encrypts/decrypts custom CryptoGarage TCP messages.
*/

#ifndef MESSAGE_H
#define MESSAGE_H

#include "AllConfig.h"
#include "Debug.h"
#include "Crypto.h"

//  Message structure:
//  [MESSAGE_BEGIN] CODE : b64(IV) : b64(TAG) : b64(enc(DATA, IV)) [MESSAGE_END]


//Our TCP messages are either plain ACK without data,
// ERR with error message string or encrypted DATA
enum MessageType {ACK, DATA, ERR, NOPE};

const char TYPE_ACK[] = "OK";
const char TYPE_DATA[] = "DATA";
const char TYPE_ERR[] = "FAIL";

struct Msg {
  MessageType type;
  String data;
};

class Message {
  private:
    static String typeToString(MessageType t);
    static MessageType stringToType(String type);

  public:
    static String encrypt(MessageType type, String &data);
    static String encrypt(MessageType type, const char * data);
    static String encrypt(MessageType type, uint8_t * data, int data_len);
    static Msg decrypt(String &s, uint8_t * iv);
    //static bool decrypt(String &raw, uint8_t (&iv)[AES_GCM_IV_LEN], uint8_t * out);
};

#endif

