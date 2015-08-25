/*
Persistant state for TankWalk
*/

//#include <avr/eeprom.h>  // in later? (1.5+) versions of Arduino <EEPROM.h> is depricated
//#include <EEPROM.h>  // in later? (1.5+) versions of Arduino <EEPROM.h> is depricated
#include </usr/share/arduino/libraries/EEPROM/EEPROM.h>

#define VERSID 0x54570000  // "TW", revision 00

typedef struct TankWalkState_s {
  unsigned long id;
  
  float avoidGain;  // reduce speed by speed*avoidGain
  float avoidMin;   // reduce speed by at least this much

  float stepGain;   // random step ~ U(1)*speed*gain
  float stepBias;   // bias U(1) by this offset, usually small and positive
  float stepMin;    // let (pre-random) step be at least this magniturde
  float cruise;     // de-bias step above this cruising speed

} TankWalkState;

const char *stateName[] = {
  "avoidGain",
  "avoidMin",
  "cruise",
  "stepGain",
  "stepBias",
  "stepMin"
};

void printStateParam(const char *nam, const float val) {
  Serial.print(nam);Serial.print('\t');Serial.println(val);
}

void printState(const TankWalkState *s) {
  byte *b = (byte *)(&(s->id));
  Serial.print(F("Vers:"));
  Serial.print((char)(b[0]));
  Serial.print((char)(b[1]));
  Serial.print((int)b[2]); Serial.print('.');
  Serial.println((int)b[3]);
  printStateParam(stateName[0],s->avoidGain);
  printStateParam(stateName[1],s->avoidMin);
  printStateParam(stateName[2],s->cruise);
  printStateParam(stateName[3],s->stepGain);
  printStateParam(stateName[4],s->stepBias);
  printStateParam(stateName[5],s->stepMin);
}

int isValid(const TankWalkState *s) {
  if (s->id != VERSID) return(0);

  if ( (s->avoidGain <= 0.0f) || 
       (s->avoidMin  <= 0.0f) ||
       (s->stepGain  <= 0.0f) ||
       (s->stepBias  <= 0.0f) ||
       (s->stepMin   <= 0.0f) ||
       (s->cruise    < 40.0f) )
     return(0);
  
  if ( (s->avoidGain >  0.7f) || 
       (s->avoidMin  > 99.0f) ||
       (s->stepGain  >  0.8f) ||
       (s->stepBias  >  5.0f) ||
       (s->stepMin   >  9.9f) ||
       (s->cruise    > 256.0f) )
     return(0);
  
  return(1);
}

void setDefault(TankWalkState *s) {
  s->id = VERSID;

  s->avoidGain = 0.1f;
  s->avoidMin  = 0.5f;

  s->stepGain = 0.1f;  // random step ~ U(1)*speed*gain
  s->stepBias = 0.01f; // bias U(1) 
  s->stepMin  = 0.4f;  // (pre-random) at least this mag
  s->cruise = 180.0f;  // de-bias step above this cruising speed
}

void loadEEPROM(const int offset, const int n, byte *b) {
  for (int i=0; i < n; i++, b++)
    *b = EEPROM.read(i+offset);
}
void updateEEPROM(const int offset, const int n, byte *b) {
  for (int i=0; i < n; i++, b++)
    {
      byte prev = EEPROM.read(i+offset);
      if (prev != *b)
	{
          EEPROM.write(i+offset,*b);
          Serial.print(F("#State byte ")); Serial.print(i);
	  Serial.println(F(" updated."));
	}
    }
}

void loadState(TankWalkState *s) {
  loadEEPROM(0,sizeof(*s),(byte *)s);
}

void updateState(const TankWalkState *s) {
  updateEEPROM(0, sizeof(*s), (byte *)s);
}

