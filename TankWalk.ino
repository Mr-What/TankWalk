/*
Main Tank-tread style random-walk control loop.

Two H-bridge motor drives, on left and right side

Motor drives take speed commands from -255..255,
with negative numbers for reverse.
If commands are not updated reguarly, the
motors will be commanded to stop.

See setup() funtion for pin assignments.

===========================================================

Aaron Birenboim, http://boim.com    25aug2015
provided under LGPL license

----------------- summary of LGPL :
You are free to use and/or alter this code as you need,
but no guarantee of correctness/appropriateness is implied.
If you do use this code please retain credit to above as originator.
*/

// for diagnostics, just print a few messages, then be quiet to improve
// performance when in actual use.
//      (must be at top, to force Ardiono.h include)
int nMsg = 9;

//#include <avr/eeprom.h>

//const char TAB = '\t';  // forces #include<Arduino.h> here, needed for following #include's

#define UPDATE_DT 25  // motor speed update period, ms
#define FLASH_DT 800  // heartbeat period, ms

#define L298  // use L298 motor driver
//#define DBH1  // use DBH1 modifications of 298 driver
#include "MotorDrive298.h"
// params : decelRate, deadmanTimeout, startupPulseDuration, stopTimeout, maxPWM
MotorDrive MotL(1.0f);
MotorDrive MotR(1.0f);

#include "TankWalkState.h"
TankWalkState state;

#include "AvoidingMeanderController.h" // assumes state is global
AvoidingMeanderController CtrlR, CtrlL;

#include "TankWalkCommand.h"  // serial command processing for TankWalk

unsigned long prevCommandTime = 0;
unsigned long tFlash = 0;

void setup()
{
  // AVR 168, 328 based Arduinos have PWM on 3, 5, 6, 9, 10, and 11
  // Timer2 for pins 3,11 : Timer 0 for pins 6,5 : Timer 1 for 9,10
  // Mega has PWM on on pins 2 through 13.

  // params: EN, IN1, IN2
  MotR.begin(3,2,4);
  MotL.begin(11,7,8);

  // first param is digital-in pin for collision avoidance sensor
  CtrlR.begin((const int) 9,&MotR);
  CtrlL.begin(10,&MotL);
  
  //Serial.begin(9600);
  Serial.begin(57600);  // nano
  //Serial.begin(115200);  # uno

  // When doing diagnostics, we may want to increase deadman time
  //MotL.setCommandTimeout(16000);
  //MotR.setCommandTimeout(16000);

//---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------  
//TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
//TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
//TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
//TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz
//TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
//TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
//TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz
//TCCR2A |= B00000011;  // Fast PWM (default for Arduino)
//TCCR2A = (TCCR2A & B11111100) | B00000001;  // timer 2, phase corrected PWM, which is 1/2 rate of Fast

//---------------------------------------------- Set PWM frequency for D9 & D10 ------------------------------
 
//TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
//TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
//TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
//TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
//TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz

  loadState(&state);
  if (!isValid(&state))
    {
      setDefault(&state);
      updateState(&state);
    }
}

void loop()
{

  unsigned long t = millis();
  if ((prevCommandTime > 0xfffff000) && (t < 999))
    {  // time counter must have wrapped around
      prevCommandTime = tFlash = 0;
      Serial.println(F("Clock wrap-around"));
    }
        
  if (t > prevCommandTime + UPDATE_DT)
    {
      prevCommandTime = t;
      CtrlL.update(t);
      CtrlR.update(t);
    }

  // check if serial command was received
  processCommand();

  if (t - tFlash > FLASH_DT)
    { // Flash standard LED to show things are running
      tFlash = t;
      digitalWrite(13,digitalRead(13)?LOW:HIGH);  // toggle heartbeat
    }
}
