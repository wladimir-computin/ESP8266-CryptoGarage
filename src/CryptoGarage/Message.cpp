/*
  CryptoGarage - Message

  (implementation)
*/

#include "Message.h"

String Message::encrypt(MessageType type, String &data) {
  return encrypt(type, (uint8_t*)data.c_str(), strlen(data.c_str()));
}

String Message::encrypt(MessageType type, const char * data) {
  return encrypt(type, (uint8_t*)data, strlen(data));
}

String Message::encrypt(MessageType type, uint8_t * data, int data_len) {
  Crypto &crypto = Crypto::instance();
  
  String header = typeToString(type);

  switch (type) {
    case ACK:
      return header + ":::";
    break;

    case ERR:
      return header + "::" + (char*)data;
    break;

    case DATA:
      {
      uint8_t _iv[AES_GCM_IV_LEN];
      uint8_t _tag[AES_GCM_TAG_LEN];
      uint8_t _encryptedData[data_len];
      crypto.getRandomIV(_iv);
      crypto.encryptData(data, data_len, _iv, _tag, _encryptedData);
      String iv_b64 = crypto.bytesToBase64(_iv, AES_GCM_IV_LEN);
      String tag_b64 = crypto.bytesToBase64(_tag, AES_GCM_TAG_LEN);
      String encryptedData_b64 = crypto.bytesToBase64(_encryptedData, sizeof(_encryptedData));

      return header + ":" + iv_b64 + ":" + tag_b64 + ":" + encryptedData_b64;
      }
    break;

  }
  return "";
}

//iv may be NULL if s contains a valid b64 encrypted IV
Msg Message::decrypt(String &s, uint8_t * iv) {

  Msg err = {NOPE, ""};
  Msg ret = {NOPE, ""};

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

      case DATA:
        {
        uint8_t _iv[AES_GCM_IV_LEN];
        uint8_t _tag[AES_GCM_TAG_LEN];
        uint8_t _encryptedData[crypto.base64DecodedLength(encryptedData_b64)];

        if (crypto.base64DecodedLength(iv_b64) == AES_GCM_IV_LEN) {//AES-GCM IV
          crypto.base64ToBytes(iv_b64, _iv);
          printDebug("Got IV: " + iv_b64);
        } else if (iv != NULL) {
          memcpy(_iv, iv, AES_GCM_IV_LEN);
          printDebug("Using challenge IV");
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

        ret.type = DATA;
        ret.data = crypto.decryptData(_encryptedData, sizeof(_encryptedData), _iv, _tag); //returns "" if decryption fails
        printDebug("Decrypted message: " + ret.data);

        if (ret.data != "") {
          return ret;
        } else {
          return err;
        }
        }
      break;

      case ERR:
        ret.data = encryptedData_b64;
      //fall through
      case ACK:
        ret.type = type;
        return ret;
      break;
    }
  }
  return err;
}

String Message::typeToString(MessageType t) {
  switch (t) {
    case ACK:
      return TYPE_ACK;
      break;
    case ERR:
      return TYPE_ERR;
      break;
    case DATA:
      return TYPE_DATA;
      break;
  }
  return "";
}

MessageType Message::stringToType(String type) {
  if (type == TYPE_ACK) {
    return ACK;
  }
  if (type == TYPE_ERR) {
    return ERR;
  }
  if (type == TYPE_DATA) {
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

  if ((startIndex != -1) && (endIndex != -1) && (endIndex - startIndex <= 300)) {
    out = message.substring(startIndex + strlen(MESSAGE_BEGIN), endIndex);
  }
  out.trim();
  return out;
}
