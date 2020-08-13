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
#include "ChallengeManager.h"

//  Message structure:
//  [MESSAGE_BEGIN] CODE : b64(IV) : b64(TAG) : b64(enc(challenge_response_b64 : challenge_request_b64 : DATA, IV)) [MESSAGE_END]


//Our TCP messages are either plain ACK without data,
// ERR with error message string or encrypted DATA
enum MessageType {HELLO, ACK, ERR, DATA, NOPE};

const char HEADER_HELLO[] = "HELLO";
const char HEADER_DATA[] = "DATA";
const char HEADER_ACK[] = "OK";
const char HEADER_ERR[] = "FAIL";

const char MESSAGE_BEGIN[] = "[BEGIN]";
const char MESSAGE_END[] = "[END]";

const char FLAG_NONE[] = "";
const char FLAG_KEEP_ALIVE[] = "F";

struct Msg {
  MessageType type;
  String data;
  String challenge;
};

struct ProcessMessageStruct{
  MessageType responseCode;
  String responseData;
  String flags;
};

class Message {
  public:
    static String typeToString(MessageType t);
    static MessageType stringToType(String type);
    static String encrypt(MessageType type, const String &data, const String &challenge_answer_b64, const String &challenge_request_b64, const String &flags);
    static Msg decrypt(String &s, ChallengeManager &cm);
    static String wrap(String &in);
    static String unwrap(String &in);
    static String getParam(String &message, int index, bool remaining=false);
};

#endif
