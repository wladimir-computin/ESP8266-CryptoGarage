/*
  CryptoGarage - Message

  (implementation)
*/

#include "Message.h"

String Message::encrypt(MessageType type, const String &data, const String &challenge_response_b64, const String &challenge_request_b64, const String &flags) {
  Crypto &crypto = Crypto::instance();
  
  String header = typeToString(type);

  switch (type) {

    case ERR:
      return header + ":::" + data;
    break;

    case ACK:
    case HELLO:
    case DATA:
      {
      String cleartext = flags + ":" + challenge_response_b64 + ":" + challenge_request_b64 + ":" + crypto.bytesToBase64((uint8_t*)data.c_str(), data.length());

      uint8_t _iv[AES_GCM_IV_LEN];
      uint8_t _tag[AES_GCM_TAG_LEN];
      uint8_t _encryptedData[cleartext.length()];
      crypto.getRandomIV(_iv);
      crypto.encryptData(cleartext, _iv, _tag, _encryptedData);
      String iv_b64 = crypto.bytesToBase64(_iv, AES_GCM_IV_LEN);
      String tag_b64 = crypto.bytesToBase64(_tag, AES_GCM_TAG_LEN);
      String encryptedData_b64 = crypto.bytesToBase64(_encryptedData, sizeof(_encryptedData));

      printDebug("Sending Challenge Response: " + challenge_response_b64);
      printDebug("Sending Challenge Request: " + challenge_request_b64);

      return header + ":" + iv_b64 + ":" + tag_b64 + ":" + encryptedData_b64;
      }
    break;

  }
  return "";
}

Msg Message::decrypt(String &s, ChallengeManager &cm) {

  Msg err = {NOPE, "", ""};
  Msg ret = {NOPE, "", ""};

  int deli1 = s.indexOf(":");
  int deli2 = s.indexOf(":", deli1 + 1);
  int deli3 = s.indexOf(":", deli2 + 1);

  if (deli1 != -1 && deli2 != -1 && deli3 != -1) { //Check if message is formed correctly.

    Crypto &crypto = Crypto::instance();
    
    MessageType type = stringToType(s.substring(0, deli1));
    String iv_b64 = s.substring(deli1 + 1, deli2);
    String tag_b64 = s.substring(deli2 + 1, deli3);
    String encryptedData_b64 = s.substring(deli3 + 1);
    
    switch (type) {

      case HELLO: //fall through
      case ACK:
      case DATA:

        {
        uint8_t _iv[AES_GCM_IV_LEN];
        uint8_t _tag[AES_GCM_TAG_LEN];
        uint8_t _encryptedData[crypto.base64DecodedLength(encryptedData_b64)];
    
        if (crypto.base64DecodedLength(iv_b64) == AES_GCM_IV_LEN) {//AES-GCM IV
          crypto.base64ToBytes(iv_b64, _iv);
          printDebug("Got IV: " + iv_b64);
        } else {
          printDebug("IV not ok!");
          return err;
        }
    
        if (crypto.base64DecodedLength(tag_b64) == AES_GCM_TAG_LEN) {//AES-GCM IV
          crypto.base64ToBytes(tag_b64, _tag);
          printDebug("Got TAG: " + tag_b64);
        } else {
          printDebug("TAG not ok!");
          return err;
        }
    
        crypto.base64ToBytes(encryptedData_b64, _encryptedData);
        printDebug("Got Payload: " + encryptedData_b64);
    
        String decrypted_message = crypto.decryptData(_encryptedData, sizeof(_encryptedData), _iv, _tag); //returns "" if decryption fails
    
        if (decrypted_message == ""){
          return err;
        }
        
        String _challenge_response_b64;
        String _challenge_request_b64;
        String _data_b64;
        
        deli1 = decrypted_message.indexOf(":");
        deli2 = decrypted_message.indexOf(":", deli1 + 1);
        
        if (deli1 != -1 && deli2 != -1){
          _challenge_response_b64 = decrypted_message.substring(0, deli1);
          _challenge_request_b64 = decrypted_message.substring(deli1 + 1, deli2);
          
          _data_b64 = decrypted_message.substring(deli2 + 1);
        }

        if(type == DATA){
          printDebug("Got Challenge Response: " + _challenge_response_b64);
          if (cm.verifyChallenge(_challenge_response_b64)){
            ret.data = String(crypto.base64ToBytes(_data_b64));
          } else {
            printDebug("Challenge missmatch!");
            return err;
          }
        }
    
        printDebug("Got Challenge Request: " + _challenge_request_b64);
        ret.challenge = _challenge_request_b64;

        ret.type = type;
    
        switch (type) {
    
          case ACK:
          case HELLO:
            printDebug("Decrypted message: [" + typeToString(type) + "]");
            return ret;
          break;  
            
          case DATA:
            printDebug("Decrypted message: [" + typeToString(type) + "] " + ret.data);
        
            if (ret.data != "") {
              return ret;
            } else {
              return err;
            }
          break;
    
          default:
            return err;
          break;
        }
        }
      break;

      case ERR:
        {
        ret.data = encryptedData_b64;
        ret.type = type;
        return ret;
        }
      break;

      default:
        return err;
      break;
    }
    }
    return err;
}

String Message::typeToString(MessageType t) {
  switch (t) {
    case HELLO:
      return HEADER_HELLO;
      break;
    case ACK:
      return HEADER_ACK;
      break;
    case ERR:
      return HEADER_ERR;
      break;
    case DATA:
      return HEADER_DATA;
      break;
  }
  return "";
}

MessageType Message::stringToType(String type) {
  if (type == HEADER_HELLO) {
    return HELLO;
  }
  if (type == HEADER_ACK) {
    return ACK;
  }
  if (type == HEADER_ERR) {
    return ERR;
  }
  if (type == HEADER_DATA) {
    return DATA;
  }
  return NOPE;
}

String Message::wrap(String &message) {
  return String(MESSAGE_BEGIN) + message + MESSAGE_END;
}

String Message::unwrap(String &message) { //REGEX: \[BEGIN\]\s*(.{0,200}?)\s*\[END\]
  String out;
  int startIndex = message.indexOf(MESSAGE_BEGIN);
  int endIndex = message.indexOf(MESSAGE_END, startIndex);

  if ((startIndex != -1) && (endIndex != -1) && (endIndex - startIndex <= MAX_MESSAGE_LEN)) {
    out = message.substring(startIndex + strlen(MESSAGE_BEGIN), endIndex);
  }
  out.trim();
  return out;
}
