#include "Joystick.h"
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_JOYSTICK,
  8, 0,                  // Button Count, Hat Switch Count
  true, true, false,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering

void setup() {
  Serial.begin(115200);
  //Set YAxis Range:
  Joystick.setYAxisRange(0, 255);
  //Set XAxis Range:
  Joystick.setXAxisRange(-255,255);
  //Begin Dynamic SendReport:
  Joystick.begin(true);
}

void loop(){
  //Set XAxis Value:
  Joystick.setXAxis(0);
  //Set YAxis Value:
  Joystick.setYAxis(0);
  //set gains [0,100]
  Joystick.setTotalGain(100);
  Joystick.setConstantGain(80);
  Joystick.setSpringGain(80);
  //set effect params
  Joystick.setSpringPosition(0);
  Joystick.setSpringMaxPosition(180);
  //Get Force [-255,255] you can set PWM with this value
  int32_t force = Joystick.getForce();
  Serial.println(force);
}
