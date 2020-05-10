#include "Joystick.h"

//
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_JOYSTICK,
  8, 0,                  // Button Count, Hat Switch Count
  true, true, false,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering

//Your Own Force calculator Func:
int32_t forceCalculate()
{
  int32_t force = 0;
  for(int id = 0; id < MAX_EFFECTS; id++){
     volatile TEffectState &effect = DynamicHID().pidReportHandler.g_EffectStates[id];
     if((effect.state == MEFFECTSTATE_PLAYING) && 
       ((effect.elapsedTime <= effect.duration) || 
       (effect.duration == USB_DURATION_INFINITE)) &&
       !DynamicHID().pidReportHandler.devicePaused)
       {
         switch(effect.effectType)
         {
           case USB_EFFECT_CONSTANT://1
              Serial.println("EFFECT_CONSTANT");
              //force += ConstantForceCalculator(effect) * constantGain;
              break;
           case USB_EFFECT_RAMP://2
              Serial.println("EFFECT_RAMP");
              break;
           case USB_EFFECT_SQUARE://3
              Serial.println("EFFECT_SQUARE");
              break;
           case USB_EFFECT_SINE://4
              Serial.println("EFFECT_SINE");
              break;
           case USB_EFFECT_TRIANGLE://5
              Serial.println("EFFECT_TRIANGLE");
              break;
           case USB_EFFECT_SAWTOOTHDOWN://6
              Serial.println("EFFECT_SAWTOOTHDOWN");
              break;
           case USB_EFFECT_SAWTOOTHUP://7
              Serial.println("EFFECT_SAWTOOTHUP");
              break;
           case USB_EFFECT_SPRING://8
              Serial.println("EFFECT_SPRING");
              break;
           case USB_EFFECT_DAMPER://9
              Serial.println("EFFECT_DAMPER");
              break;
           case USB_EFFECT_INERTIA://10
              Serial.println("EFFECT_INERTIA");
              break;
           case USB_EFFECT_FRICTION://11
              Serial.println("EFFECT_FRICTION");
              break;
           case USB_EFFECT_CUSTOM://12
              Serial.println("EFFECT_CUSTOM");
              break;
         }
         effect.elapsedTime = (uint64_t)millis() - effect.startTime;
       }
  }
  force = (int32_t)((float)1.00 * force * totalGain / 10000); // each effect gain * total effect gain = 10000
  return constrain(force, -250, 250);
}

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
  //Receive Force Feedback message:
  Joystick.recv_from_usb();
  //Get Force
  int32_t force = forceCalculate();
  Serial.println(force);
}
