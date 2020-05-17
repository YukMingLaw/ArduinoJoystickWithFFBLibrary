# Arduino Joystick With Force Feedback Library

## 声明

### 这是一个适用于Atmega32UX芯片带力反馈的Joystick库，可用于制作带震动的游戏手柄、带力反馈的游戏方向盘等。支持两轴力反馈。


## 用法

### 0.例程：`examples/JoystickWithFFB/JoystickWithFFB.ino`

### 1. 创建JoyStick对象

| 参数表          | 描述                                     |
|:---------------|:----------------------------------------|
| REPORT_ID      | Hid report id 值                         |
| JOYSTICK_TYPE  | 设备的类型：JOYSTICK，GAMEPAD，MULTI_AXIS |
| 按钮数量        | 值:`[0-32]`                              |
| Hat Switch 数量 | 值:`[0,1,2]`                            |
| X轴使能         | True or False                           |
| Y轴使能         | True or False                           |
| Z轴使能         | True or False                           |
| Rx轴使能        | True or False                           |
| Ry轴使能        | True or False                           |
| Rz轴使能        | True or False                           |
| Rudder使能      | True or False                           |
| Throttle使能    | True or False                           |
| Accelerator使能 | True or False                           |
| Brake使能       | True or False                           |
| Steering使能    | True or False                           |

##### `Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_JOYSTICK,8, 0,false, true,true,false, false, false,false, false,false, false, false);`


### 2. 创建好对象后默认绑定X轴Y轴作为力反馈轴，通过下列接口设置各种力的增益:

#### 1）设置总增益
`JoyStick.setTotalGain(uint8_t _gain);`

#### 2）设置恒定力增益
`JoyStick.setConstantGain(uint8_t _gain);`

#### 3）设置斜坡力增益
`JoyStick.setRampGain(uint8_t _gain);`

#### 4）设置方波力增益
`JoyStick.setSquareGain(uint8_t _gain);`

#### 5）设置余弦力增益
`JoyStick.setSineGain(uint8_t _gain);`

#### 6）设置三角力增益
`JoyStick.setTriangleGain(uint8_t _gain);`

#### 7）设置下锯齿力增益
`JoyStick.setSawToothDownGain(uint8_t _gain);`

#### 8）设置上锯齿力增益
`JoyStick.setSawToothUpGain(uint8_t _gain);`

#### 9）设置弹簧力增益
`JoyStick.setSpringGain(uint8_t _gain);`

#### 10）设置减震力增益
`JoyStick.setDamperGain(uint8_t _gain);`

#### 11）设置惯性力增益
`JoyStick.setInertiaGain(uint8_t _gain);`

#### 12）设置摩擦力增益
`JoyStick.setFrictionGain(uint8_t _gain);`

#### 13）设置自定义力增益
`JoyStick.setCustomGain(uint8_t _gain);`

### 3.通过下列接口设置spring效果，Damper效果，Inertia效果，Friction效果参数：

#### 1）设置弹簧力对象位置参数
` JoyStick.setSpringPosition(int32_t value)；`

#### 2）设置弹簧力对象最大位置参数
` JoyStick.setSpringMaxPosition(int32_t value)；`

#### 3）设置减震力对象速度参数
` JoyStick.setDamperVelocity(int32_t value)；`

#### 4）设置减震力对象最大速度参数
` JoyStick.setDamperMaxVelocity(int32_t value)；`

#### 5）设置惯性力加速度参数
` JoyStick.setInertiaAcceleration(int32_t value)；`

#### 6）设置惯性力最大加速度参数
` JoyStick.setInertiaMaxAcceleration(int32_t value)；`

#### 7）设置摩擦力位置差参数
` JoyStick.setFrictionPositionChange(int32_t value)；`

#### 8）设置摩擦力最大位置差参数
` JoyStick.setFrictionMaxPositionChange(int32_t value)；`

### 4.通过：`JoyStick.getForce(int32_t* forces)`获取计算好的力

#### 传入参数为一个长度为2的int32_t数组，输出范围`[-255,255]`；

## 引用

### 本库基于[Heironimus](https://github.com/MHeironimus/ArduinoJoystickLibrary) 跟 [hoantv](https://github.com/hoantv/VNWheel) 的库修改与整理，非常感谢他们的工作。

