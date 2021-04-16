# Arduino Joystick with Force Feedback

## Fino will make a Force Feedback Joystick out of your Arduino
### It requires an ATMega32UX chip device such as an Arduino Micro

## Usage

There are two different ways you can use this sketch:
1. You can use it as some sort of virtual vJoy device. This is what the sketch is doing at the moment. It will receive messages through the COM port and update the current position of the joystick. It can also send the current forces through the COM port. I'm using this in [BrunnerDX](https://github.com/jmriego/brunnerdx) repo to make the Brunner base appear as a DirectX joystick
2. You can create your own Arduino based joystick using this library as a base. Instead of sending the position/forces through the COM port you can add potentiometers/motors directly.


## Ref

### This is based on some other libraries. Thanks a lot for your work!
* [Heironimus](https://github.com/MHeironimus/ArduinoJoystickLibrary)
* [hoantv](https://github.com/hoantv/VNWheel)
* [YukMingLaw](https://github.com/YukMingLaw/ArduinoJoystickWithFFBLibrary) 
* [arafin](https://github.com/araffin/arduino-robust-serial/)
