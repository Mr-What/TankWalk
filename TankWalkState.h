/*
Persistant state for TankWalk
*/

#include <EEPROM.h>

#define VERSID 0x54570000  // "TW", revision 00

typedef struct TankWalkState_s {
  unsigned long id;
  
  float avoidGain;  // reduce speed by speed*avoidGain
  float avoidMin;   // reduce speed by at least this much

  float randStepGain;   // random step ~ U(1)*speed*gain
  float randStepOffset; // bias U(1) by this offset, usually small and positive
  float randStepMin;    // let (pre-random) step be at least this magniturde
  float cruise;         // de-bias step above this cruising speed

} TankWalkState;

void printState(const TankWalkState *s) {
  byte *b = (byte *)(&(s->id));
  Serial.print(F("Vers:"));
  Serial.print((char)(b[0]));
  Serial.print((char)(b[1]));
  Serial.print((int)b[2]); Serial.print('.');
  Serial.println((int)b[3]);

}

int isValid(const TankWalkState *s) {
  if (s->id != VERSID) return(0);

  if ( (s->avoidGain      <= 0.0f) || 
       (s->avoidMin       <= 0.0f) ||
       (s->randStepGain   <= 0.0f) ||
       (s->randStepOffset <= 0.0f) ||
       (s->randStepMin    <= 0.0f) ||
       (s->cruise < 40.0f) )
     return(0);
  
  if ( (s->avoidGain      >  0.7f) || 
       (s->avoidMin       > 99.0f) ||
       (s->randStepGain   >  0.8f) ||
       (s->randStepOffset >  5.0f) ||
       (s->randStepMin    >  9.9f) ||
       (s->cruise > 256.0f) )
     return(0);
  
  return(1);
}

void setDefault(const TankWalkState *s) {
  s->id = VERSID;

  s->avoidGain = 0.1f;
  s->avoidMin  = 0.5f;

  s->randStepGain = 0.1f;   // random step ~ U(1)*speed*gain
  s->randStepOffset = 0.01f; // bias U(1) 
  s->randStepMin = 0.4f;    // (pre-random) at least this mag
  s->cruise = 180.0f;     // de-bias step above this cruising speed
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
  updateEEPROM(0, sizeof(*s), (byte *)s)
}
