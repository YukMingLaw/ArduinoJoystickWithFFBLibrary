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


### 2. After the object is created, the x-axis and y-axis are bound as the force feedback axis by default.The gains of various forces effect are set through the following interfaces:

#### 1）set total gain `[0-100]`
`JoyStick.setTotalGain(uint8_t _gain);`

#### 2）set constant gain `[0-100]`
`JoyStick.setConstantGain(uint8_t _gain);`

#### 3）set ramp gain `[0-100]`
`JoyStick.setRampGain(uint8_t _gain);`

#### 4）set square gain `[0-100]`
`JoyStick.setSquareGain(uint8_t _gain);`

#### 5）set sine gain `[0-100]`
`JoyStick.setSineGain(uint8_t _gain);`

#### 6）set triangle gain `[0-100]`
`JoyStick.setTriangleGain(uint8_t _gain);`

#### 7）set sawtoothdown gain `[0-100]`
`JoyStick.setSawToothDownGain(uint8_t _gain);`

#### 8）set sawtoothup gain `[0-100]`
`JoyStick.setSawToothUpGain(uint8_t _gain);`

#### 9）set spring gain `[0-100]`
`JoyStick.setSpringGain(uint8_t _gain);`

#### 10）set damper gain `[0-100]`
`JoyStick.setDamperGain(uint8_t _gain);`

#### 11）set intertia gain `[0-100]`
`JoyStick.setInertiaGain(uint8_t _gain);`

#### 12）set friction gain `[0-100]`
`JoyStick.setFrictionGain(uint8_t _gain);`

#### 13）set custom gain `[0-100]`
`JoyStick.setCustomGain(uint8_t _gain);`

### 3.Set the parameters of spring effect, damper effect, inertia effect and friction effect through the following interfaces:

#### 1）If you need to use the spring effect, set the following parameters.`Position` is the current position of the force feedback axis. For example, connect the encoder with the action axis,the current encoder value is `Positon` and the max encoder value is `MaxPosition`.
` JoyStick.setSpringPosition(int32_t value)；`

` JoyStick.setSpringMaxPosition(int32_t value)；`

#### 2）If you need to use the damper effect, set the following parameters.`Velocity` is the current velocity of the force feedback axis.
` JoyStick.setDamperVelocity(int32_t value)；`

` JoyStick.setDamperMaxVelocity(int32_t value)；`

#### 3）If you need to use the inertia effect, set the following parameters.`Acceleration` is the current acceleration of the force feedback axis.
` JoyStick.setInertiaAcceleration(int32_t value)；`

` JoyStick.setInertiaMaxAcceleration(int32_t value)；`

#### 4）If you need to use the friction effect, set the following parameters.`PositionChange` is the position difference of the force feedback axis.
` JoyStick.setFrictionPositionChange(int32_t value)；`

` JoyStick.setFrictionMaxPositionChange(int32_t value)；`

### 4.Finally,get the force value with

`JoyStick.getForce(int32_t* forces)`

params `int32_t* forces` is an `int32` array of length 2

`forces[0]` is the X-Axis force data

`forces[1]` is the Y-Axis force data

return type `void`

range`[-255,255]`

## Ref

### This library is based on [Heironimus](https://github.com/MHeironimus/ArduinoJoystickLibrary) and [hoantv](https://github.com/hoantv/VNWheel) 's work，very grateful for their work.

