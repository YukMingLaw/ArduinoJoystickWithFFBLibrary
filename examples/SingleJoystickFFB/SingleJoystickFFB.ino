#include "Joystick.h"

//X-axis & Y-axis REQUIRED
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_MULTI_AXIS, 4, 0,
  true, true, true, //X,Y,Z
  false, false, false,//Rx,Ry,Rz
  false, false, false, false, false);

Gains mygains[2];
EffectParams myeffectparams[2];

//set X Axis gains
mygains[0].totalGain = 100;//0-100
mygains[0].springGain = 100;//0-100

void setup(){
    pinMode(A2,INPUT);
    pinMode(9,INPUT);
    Joystick.setXAxisRange(0, 1023);
    //Steering wheel
    //Joystick.setXAxisRange(-512, 512);
    //enable gains REQUIRED
    Joystick.setGains(mygains);
    Joystick.begin();
}

void loop(){
  int value = analogRead(A2);
  //set X Axis Spring Effect Param
  myeffectparams[0].springMaxPosition = 1023;
  myeffectparams[0].springPosition = value;//0-1023
  //Steering wheel
  //myeffectparams[0].springMaxPosition = 512;
  //myeffectparams[0].springPosition = value - 512; //-512-512
  
  Joystick.setXAxis(value);
  //Steering wheel
  //Joystick.setXAxis(value - 512);
  Joystick.setEffectParams(myeffectparams);
  Joystick.getForce(forces);

  analogWrite(9,forces);
  delay(1);
}
