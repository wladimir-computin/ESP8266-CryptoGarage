/*
* CryptoGarage - ConnectionState
* 
* (implementation)
*/

#include "ChallengeManager.h"

void ChallengeManager::stateTick(void * context) {
  printDebug("Challenge timed out");
  (*(ChallengeManager*)context).resetChallenge();
}

String ChallengeManager::getCurrentChallenge(){
  return challenge;
}

String ChallengeManager::generateRandomChallenge(){
  resetChallenge();
  challenge = Crypto::instance().getRandomChallengeBase64();
  stateTicker.attach(CHALLENGE_VALIDITY_TIMEOUT, stateTick, (void*)this);
  return challenge;
}

bool ChallengeManager::verifyChallenge(String challenge_response_b64){
  return (challenge == challenge_response_b64 && challenge != "");
}

void ChallengeManager::resetChallenge(){
  stateTicker.detach();
  challenge = "";
}
