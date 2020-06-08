# Arduino Joystick With Force Feedback Library

## Statement

### This is a joy library for Atmega32UX chip with force feedback, which can be used to make game handle with vibration, game steering wheel with force feedback, etc.Multi-axis-force-feedback feature is added.


## Usage

### 0. example：`examples/JoystickWithFFB/JoystickWithFFB.ino`

### 1. create `JoyStick` object

| params            | describe                                     |
|:------------------|:---------------------------------------------|
| REPORT_ID         | Hid report id                                |
| JOYSTICK_TYPE     | Type of devece：JOYSTICK，GAMEPAD，MULTI_AXIS |
| button count      | `[0-32]`                                     |
| Hat Switch count  | `[0,1,2]`                                    |
| X Axis enable     | True or False                                |
| Y Axis enable     | True or False                                |
| Z Axis enable     | True or False                                |
| Rx Axis enable    | True or False                                |
| Ry Axis enable    | True or False                                |
| Rz Axis enable    | True or False                                |
| Rudder enable     | True or False                                |
| Throttle enable   | True or False                                |
| Accelerator enable| True or False                                |
| Brake enable      | True or False                                |
| Steering enable   | True or False                                |

`Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_JOYSTICK,8, 0,false, true,true,false, false, false,false, false,false, false, false);`


### 2. After the object is created, the x-axis and y-axis are bound as the force feedback axis by default.The gains of various forces effect are set through the struct and the interface as following:

```
//struct of gains
struct Gains{
    uint8_t totalGain         = FORCE_FEEDBACK_MAXGAIN;
	uint8_t constantGain      = FORCE_FEEDBACK_MAXGAIN;
	uint8_t rampGain          = FORCE_FEEDBACK_MAXGAIN;
	uint8_t squareGain        = FORCE_FEEDBACK_MAXGAIN;
	uint8_t sineGain          = FORCE_FEEDBACK_MAXGAIN;
	uint8_t triangleGain      = FORCE_FEEDBACK_MAXGAIN;
	uint8_t sawtoothdownGain  = FORCE_FEEDBACK_MAXGAIN;
	uint8_t sawtoothupGain    = FORCE_FEEDBACK_MAXGAIN;
	uint8_t springGain        = FORCE_FEEDBACK_MAXGAIN;
	uint8_t damperGain        = FORCE_FEEDBACK_MAXGAIN;
	uint8_t inertiaGain       = FORCE_FEEDBACK_MAXGAIN;
	uint8_t frictionGain      = FORCE_FEEDBACK_MAXGAIN;
	uint8_t customGain        = FORCE_FEEDBACK_MAXGAIN;
};
/* set gains interface func
 * param:a <Gains> array of length 2;
 *       _gains[0]-->X_Axis_gains;
 *       _gains[1]-->Y_Axis_gains;
 * return 0 ：set gains successful
 *        -1：set gains failed
*/
int8_t setGains(Gains* _gains);
```
#### example code
```
#include "Joystick.h"

Joystick_ Joystick();

Gains mygains[2];

//set X Axis gains
mygains[0].totalGain = 50;
mygains[0].springGain = 80;

//set Y Axis gains
mygains[1].totalGain = 50;
mygains[1].springGain = 70;

void setup{
    //enable gains REQUIRED
    Joystick.setGains(mygains);
}

```


### 3.Set the parameters of spring effect, damper effect, inertia effect and friction effect through the struct and the interface as following:

```
//struct of Effect Params
struct EffectParams{
    //If you need to use the spring effect, set the following parameters.`Position` is the current position of the force feedback axis. 
    //For example, connect the encoder with the action axis,the current encoder value is `Positon` and the max encoder value is `MaxPosition`.
    int32_t springMaxPosition = 0;
    int32_t springPosition = 0;

    //If you need to use the damper effect, set the following parameters.`Velocity` is the current velocity of the force feedback axis.
    int32_t damperMaxVelocity = 0;
    int32_t damperVelocity = 0;

    //If you need to use the inertia effect, set the following parameters.`Acceleration` is the current acceleration of the force feedback axis.
    int32_t inertiaMaxAcceleration = 0;
    int32_t inertiaAcceleration = 0;
    
    //If you need to use the friction effect, set the following parameters.`PositionChange` is the position difference of the force feedback axis.
    int32_t frictionMaxPositionChange = 0;
    int32_t frictionPositionChange = 0;
};

/* set effect params interface func
 * param:a <EffectParams> array of length 2;
 *       _effect_params[0]-->X_Axis_params;
 *       _effect_params[1]-->Y_Axis_params;
 *
 * return 0 ：set Effect Params successful
 *        -1：set Effect Params failed
*/
int8_t setEffectParams(EffectParams* _effect_params);
```

#### example code
```
#include "Joystick.h"

Joystick_ Joystick();

Gains mygains[2];
EffectParams myeffectparams[2];

//set X Axis gains
mygains[0].totalGain = 50;
mygains[0].springGain = 80;

//set Y Axis gains
mygains[1].totalGain = 50;
mygains[1].springGain = 70;

void setup(){
    //enable gains REQUIRED
    Joystick.setGains(mygains);
}

void loop(){
  //set X Axis Spring Effect Param
  myeffectparams[0].springMaxPosition = 1023;
  myeffectparams[0].springPosition = analogRead(A2);
  
  //set Y Axis Spring Effect Param
  myeffectparams[1].springMaxPosition = 1023;
  myeffectparams[1].springPosition = analogRead(A3);
  
  Joystick.setEffectParams(myeffectparams);
  Joystick.getForce(forces);
}

```


### 4.Finally,get the force value with

`JoyStick.getForce(int32_t* forces)`

params `int32_t* forces` is an `int32` array of length 2

`forces[0]` is the X-Axis force data

`forces[1]` is the Y-Axis force data

return type `void`

range`[-255,255]`

#### **Pay Attention!**

You can test your FFB device with [this tool](https://www.dirks-software.ca/downloads/ForceTest.exe).

**`Joystick.setGains(mygains)` and `Joystick.setEffectParams(myeffectparams)` must be invoked before `JoyStick.getForce(int32_t* forces)`**

## Ref

### This library is based on [Heironimus](https://github.com/MHeironimus/ArduinoJoystickLibrary) and [hoantv](https://github.com/hoantv/VNWheel) 's work，very grateful for their work.

