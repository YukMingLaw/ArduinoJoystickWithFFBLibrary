
#include <Joystick.h>
#include "DigitalWriteFast.h"

#define encoderPinA 2
#define encoderPinB 3

#define motorPinA 7
#define motorPinB 8
#define motorPinPWM 9

#define ENCODER_MAX_VALUE 1200
#define ENCODER_MIN_VALUE -1200

#define MAX_PWM 200

bool isOutOfRange = false;
int32_t forces[2]={0};
Gains gains[2];
EffectParams effectparams[2];

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_JOYSTICK,
  8, 0,                  // Button Count, Hat Switch Count
  true, true, false,     // X and Y, but no Z Axis
  true, true, false,   //  Rx, Ry, no Rz
  false, false,          // No rudder or throttle
  false, false, false);    // No accelerator, brake, or steering

volatile long value = 0;
int32_t g_force = 0;

int32_t  currentPosition = 0;
volatile int8_t oldState = 0;
const int8_t KNOBDIR[] = {
  0, 1, -1, 0,
  -1, 0, 0, 1,
  1, 0, 0, -1,
  0, -1, 1, 0
};

void tick(void)
{
  int sig1 = digitalReadFast(encoderPinA);
  int sig2 = digitalReadFast(encoderPinB);
  int8_t thisState = sig1 | (sig2 << 1);

  if (oldState != thisState) {
    currentPosition += KNOBDIR[thisState | (oldState<<2)];
    oldState = thisState;
  } 
}

void setup() {
  Serial.begin(115200);                        
  attachInterrupt(digitalPinToInterrupt(encoderPinA),tick,CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB),tick,CHANGE);
  Joystick.setRyAxisRange(0, 500);
  Joystick.setRxAxisRange(0, 500);
  Joystick.setYAxisRange(0, 500);
  Joystick.setXAxisRange(ENCODER_MIN_VALUE, ENCODER_MAX_VALUE);
  Joystick.setGains(gains);
  Joystick.begin(true);

  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);
  pinMode(motorPinPWM, OUTPUT);

  pinMode(A0, INPUT_PULLUP);
  
  cli();
  TCCR3A = 0; //set TCCR1A 0
  TCCR3B = 0; //set TCCR1B 0
  TCNT3  = 0; //counter init
  OCR3A = 399;
  TCCR3B |= (1 << WGM32); //open CTC mode
  TCCR3B |= (1 << CS31); //set CS11 1(8-fold Prescaler)
  TIMSK3 |= (1 << OCIE3A);
  sei();
  
}

ISR(TIMER3_COMPA_vect){
  Joystick.getUSBPID();
}

unsigned int interval = 0;
void loop() {
  value = currentPosition;
  
  if(value > ENCODER_MAX_VALUE)
  {
    isOutOfRange = true;
    value = ENCODER_MAX_VALUE;
  }else if(value < ENCODER_MIN_VALUE)
  {
    isOutOfRange = true;
    value = ENCODER_MIN_VALUE;
  }else{
    isOutOfRange = false;
  }

  Joystick.setXAxis(value);
  Joystick.setRxAxis(analogRead(A1));
  Joystick.setRyAxis(analogRead(A2));
  Joystick.setYAxis(analogRead(A3));

  effectparams[0].springMaxPosition = ENCODER_MAX_VALUE;
  effectparams[0].springPosition = value;
  effectparams[1].springMaxPosition = 255;
  effectparams[1].springPosition = 0;
  Joystick.setEffectParams(effectparams);
  Joystick.getForce(forces);

  
  if(!isOutOfRange){
    if(forces[0] > 0)
    {
      digitalWrite(motorPinA, HIGH);
      digitalWrite(motorPinB, LOW);
      analogWrite(motorPinPWM, abs(forces[0]));
    }else{
      digitalWrite(motorPinA, LOW);
      digitalWrite(motorPinB, HIGH);
      analogWrite(motorPinPWM, abs(forces[0]));
    }
  }else{
    if(value < 0){
      digitalWrite(motorPinA, LOW);
      digitalWrite(motorPinB, HIGH);
    }else{
      digitalWrite(motorPinA, HIGH);
      digitalWrite(motorPinB, LOW);
    }
    analogWrite(motorPinPWM, MAX_PWM);
  }
}
