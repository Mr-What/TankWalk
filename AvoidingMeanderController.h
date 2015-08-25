/*
Do a velocity-based random walk, but with some obsticle
avoidance from a binary proximity sensor pointinto toward
the OPPOSITE side of the robot from this drive motor.

Assumes that MotorDrive and ... classes have already been defined
and that a global TankWalkState state has already been instanciated.

===========================================================

Aaron Birenboim, http://boim.com    25aug2015
provided under LGPL license

----------------- summary of LGPL :
You are free to use and/or alter this code as you need,
but no guarantee of correctness/appropriateness is implied.
If you do use this code please retain credit to above as originator.
*/

// uniform random number within [-1,1]
inline float randu() {
  return( (float)(random(0x40000001) - 0x20000000) / 0x20000000 );
}

// this should be defined and global for diagnostics : int nMsg = 9;

class AvoidingMeanderController {
 public:
  MotorDrive *_mot;
  float _speed;  // current commanded speed
  int _pinSens;  // input pin for proximity sensor

  AvoidingMeanderController()
    {
      _mot = 0;
      _speed = 0;
      _pinSens = -1;
    }

  void begin(const int pinSens, MotorDrive *md)
    {
      _mot = md;
      _mot->stop();
      _pinSens = pinSens;
      pinMode(_pinSens, INPUT);
    }

  inline void setSpeed(const long t)
    {
      if (_speed < -255.0f) _speed = -255.0f;
      if (_speed >  255.0f) _speed =  255.0f;
      _mot->setSpeed((int)_speed,t);
    }

  // perform avoidance maneuver update
  void avoid(const long t)
    {
      float dSpeed = _speed * state.avoidGain;
      dSpeed = ABS(dSpeed);
      if (dSpeed < state.avoidMin) dSpeed = state.avoidMin;
      _speed -= dSpeed;
      setSpeed(t);
    }

  void updateRandomWalk(const long t)
    {
      float dSpeed = state.stepGain * _speed;
      dSpeed = ABS(dSpeed);
      if (dSpeed < state.stepMin) dSpeed = state.stepMin;
      float offst = state.stepBias;
      if (_speed > state.cruise) offst *= -0.3f;
      dSpeed = dSpeed * (randu() + offst);
      _speed += dSpeed;
      setSpeed(t);
    }

  void update(const long t)
    {
      if (digitalRead(_pinSens) == LOW)
	avoid(t);
      else
	updateRandomWalk(t);
    }

};
