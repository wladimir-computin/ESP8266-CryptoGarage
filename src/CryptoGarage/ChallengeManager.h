/*
* CryptoGarage - ChallengeManager
* 
* Class for tracking the state of the challenge-response protocol.
* The state jumps back to NONE after CONNECTION_STATE_TIMEOUT seconds (defined in AllConfig.h)
* This makes replay attacks or brute forcing the 12 byte challenge IV practically impossible.
*/

#ifndef CHALLENGEMANAGER_H
#define CHALLENGEMANAGER_H

#include <Ticker.h>

#include "AllConfig.h"
#include "Debug.h"

#include "Crypto.h"

class ChallengeManager {
  private:
    String challenge;
    Ticker stateTicker;
    static void stateTick(void * context);
    int challenge_timeout = DEFAULT_CHALLENGE_VALIDITY_TIMEOUT;

  public:
    void setChallengeTimeout(int challenge_timeout);
    String generateRandomChallenge();
    String getCurrentChallenge();
    void resetChallenge();
    bool verifyChallenge(String challenge_response_b64);
};

#endif
