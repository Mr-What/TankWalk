/*
Serial command interpreter for TankWalk

Main commands are to tweak control parameters,
and read/write them as persistant state
*/

#include <ioUtil.h>
SerialLineBuffer SerialCmd;

// extern : int nMsg = 9;

void setVal(const char *key, const char *valStr, float &val)
{
  if ( (valStr==NULL) || (valStr[0]==0) )
    {
      Serial.print(F("#Ignoring empty value for "));
      Serial.print(key);
      Serial.print(F(" command"));
      return;
    }
  val = atof(valStr);
  Serial.println();
  Serial.print(key);
  Serial.print('\t');
  Serial.println(val);
}

/*
void setVal(const char *key, const char *valStr, int &val)
{
  if ( (valStr==NULL) || (valStr[0]==0) )
    {
      Serial.print("#Ignoring empty value for ");
      Serial.print(key);
      Serial.print(" command");
      return;
    }
  val = atoi(valStr);
  Serial.print("\n");
  Serial.print(key);
  Serial.print("\t");
  Serial.println(val);
}
*/

// check if command was received, and process it
void processCommand()
{
  char *key,*val,*cmd;
  //static SerialLineBuffer cmd;  // doesn't work like this on Arduino gcc-avr
  if (!SerialCmd.isComplete()) return;
  cmd = SerialCmd.get();
  key = extractKey(cmd,&val);
  if ((key==NULL) || (key[0]==0)) return;  // blank line
  Serial.print(F("#CMD> "));Serial.print(key);
  if (val && *val) {Serial.print("\t");Serial.println(val);}
  else Serial.println("");

  if (keyMatch(key,F("help")) || (*key=='?') || keyMatch(key,F("show")) )
    {
      Serial.println(F("?|help|show : print this message"));
      Serial.println(F("save\t: save current state to EEPROM"));
      Serial.println(F("reset\t: reset current state to defaults"));
      printState();
    }
  else if (keyMatch(key,F("save"))) {updateState(&state);}
  else if (keyMatch(key,F("reset"))) {setDefault(&state);}
  else if (keyMatch(key,stateName[0]) ||
	   keyMatch(key,stateName[1]) ||
	   keyMatch(key,stateName[2]) ||
	   keyMatch(key,stateName[3]) ||
	   keyMatch(key,stateName[4]) ||
	   keyMatch(key,stateName[5]))
    {
      float x;
      setVal(key,val,x);
      if      (keyMatch(key,stateName[0])) s->avoidGain= x;
      else if (keyMatch(key,stateName[1])) s->avoidMin = x;
      else if (keyMatch(key,stateName[2])) s->cruise   = x;
      else if (keyMatch(key,stateName[3])) s->stepGain = x;
      else if (keyMatch(key,stateName[4])) s->stepBias = x;
      else if (keyMatch(key,stateName[5])) s->stepMin  = x;
      else Serial.println(F("Unidentified key.  should never happen."));
    }
  else
    {
      Serial.print("#Unrecognized command \"");
      Serial.print(key);
      Serial.println("\"");
    }
}
