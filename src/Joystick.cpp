/*
  Joystick.cpp

  Copyright (c) 2015-2017, Matthew Heironimus

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Joystick.h"
#include "FFBDescriptor.h"
#if defined(_USING_DYNAMIC_HID)

#define JOYSTICK_REPORT_ID_INDEX 7
#define JOYSTICK_AXIS_MINIMUM -32767
#define JOYSTICK_AXIS_MAXIMUM 32767
#define JOYSTICK_SIMULATOR_MINIMUM -32767
#define JOYSTICK_SIMULATOR_MAXIMUM 32767

#define JOYSTICK_INCLUDE_X_AXIS  B00000001
#define JOYSTICK_INCLUDE_Y_AXIS  B00000010
#define JOYSTICK_INCLUDE_Z_AXIS  B00000100
#define JOYSTICK_INCLUDE_RX_AXIS B00001000
#define JOYSTICK_INCLUDE_RY_AXIS B00010000
#define JOYSTICK_INCLUDE_RZ_AXIS B00100000

#define JOYSTICK_INCLUDE_RUDDER      B00000001
#define JOYSTICK_INCLUDE_THROTTLE    B00000010
#define JOYSTICK_INCLUDE_ACCELERATOR B00000100
#define JOYSTICK_INCLUDE_BRAKE       B00001000
#define JOYSTICK_INCLUDE_STEERING    B00010000

unsigned int timecnt = 0;

Joystick_::Joystick_(
	uint8_t hidReportId,
	uint8_t joystickType,
    uint8_t buttonCount,
	uint8_t hatSwitchCount,
	bool includeXAxis,
	bool includeYAxis,
	bool includeZAxis,
	bool includeRxAxis,
	bool includeRyAxis,
	bool includeRzAxis,
	bool includeRudder,
	bool includeThrottle,
	bool includeAccelerator,
	bool includeBrake,
	bool includeSteering)
{
    // Set the USB HID Report ID
    _hidReportId = hidReportId;

    // Save Joystick Settings
    _buttonCount = buttonCount;
	_hatSwitchCount = hatSwitchCount;
	_includeAxisFlags = 0;
	_includeAxisFlags |= (includeXAxis ? JOYSTICK_INCLUDE_X_AXIS : 0);
	_includeAxisFlags |= (includeYAxis ? JOYSTICK_INCLUDE_Y_AXIS : 0);
	_includeAxisFlags |= (includeZAxis ? JOYSTICK_INCLUDE_Z_AXIS : 0);
	_includeAxisFlags |= (includeRxAxis ? JOYSTICK_INCLUDE_RX_AXIS : 0);
	_includeAxisFlags |= (includeRyAxis ? JOYSTICK_INCLUDE_RY_AXIS : 0);
	_includeAxisFlags |= (includeRzAxis ? JOYSTICK_INCLUDE_RZ_AXIS : 0);
	_includeSimulatorFlags = 0;
	_includeSimulatorFlags |= (includeRudder ? JOYSTICK_INCLUDE_RUDDER : 0);
	_includeSimulatorFlags |= (includeThrottle ? JOYSTICK_INCLUDE_THROTTLE : 0);
	_includeSimulatorFlags |= (includeAccelerator ? JOYSTICK_INCLUDE_ACCELERATOR : 0);
	_includeSimulatorFlags |= (includeBrake ? JOYSTICK_INCLUDE_BRAKE : 0);
	_includeSimulatorFlags |= (includeSteering ? JOYSTICK_INCLUDE_STEERING : 0);
	
    // Build Joystick HID Report Description
	
	// Button Calculations
	uint8_t buttonsInLastByte = _buttonCount % 8;
	uint8_t buttonPaddingBits = 0;
	if (buttonsInLastByte > 0)
	{
		buttonPaddingBits = 8 - buttonsInLastByte;
	}
	
	// Axis Calculations
	uint8_t axisCount = (includeXAxis == true)
		+  (includeYAxis == true)
		+  (includeZAxis == true)
		+  (includeRxAxis == true)
		+  (includeRyAxis == true)
		+  (includeRzAxis == true);
		
	uint8_t simulationCount = (includeRudder == true)
		+ (includeThrottle == true)
		+ (includeAccelerator == true)
		+ (includeBrake == true)
		+ (includeSteering == true); 
		
	static uint8_t tempHidReportDescriptor[150];
	int hidReportDescriptorSize = 0;

    // USAGE_PAGE (Generic Desktop)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    // USAGE (Joystick - 0x04; Gamepad - 0x05; Multi-axis Controller - 0x08)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
    tempHidReportDescriptor[hidReportDescriptorSize++] = joystickType;

    // COLLECTION (Application)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0xa1;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
	// USAGE (Pointer)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
    // REPORT_ID (Default: 1)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x85;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

	// COLLECTION (Physical)
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0xa1;
	tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

	if (_buttonCount > 0) {

		// USAGE_PAGE (Button)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;

		// USAGE_MINIMUM (Button 1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x19;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// USAGE_MAXIMUM (Button 32)            
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x29;
		tempHidReportDescriptor[hidReportDescriptorSize++] = _buttonCount;

		// LOGICAL_MINIMUM (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// LOGICAL_MAXIMUM (1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// REPORT_SIZE (1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// REPORT_COUNT (# of buttons)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
		tempHidReportDescriptor[hidReportDescriptorSize++] = _buttonCount;

		// UNIT_EXPONENT (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x55;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// UNIT (None)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x65;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;

		if (buttonPaddingBits > 0) {
			
			// REPORT_SIZE (1)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

			// REPORT_COUNT (# of padding bits)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
			tempHidReportDescriptor[hidReportDescriptorSize++] = buttonPaddingBits;
					
			// INPUT (Const,Var,Abs)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x03;
			
		} // Padding Bits Needed
	} // Buttons

	if ((axisCount > 0) || (_hatSwitchCount > 0)) {
	
		// USAGE_PAGE (Generic Desktop)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
		
	}

	if (_hatSwitchCount > 0) {

		// USAGE (Hat Switch)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x39;

		// LOGICAL_MINIMUM (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// LOGICAL_MAXIMUM (7)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x07;

		// PHYSICAL_MINIMUM (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x35;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// PHYSICAL_MAXIMUM (315)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x46;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x3B;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// UNIT (Eng Rot:Angular Pos)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x65;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x14;

		// REPORT_SIZE (4)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;

		// REPORT_COUNT (1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
						
		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		if (_hatSwitchCount > 1) {
			
			// USAGE (Hat Switch)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x39;

			// LOGICAL_MINIMUM (0)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

			// LOGICAL_MAXIMUM (7)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x07;

			// PHYSICAL_MINIMUM (0)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x35;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

			// PHYSICAL_MAXIMUM (315)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x46;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x3B;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

			// UNIT (Eng Rot:Angular Pos)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x65;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x14;

			// REPORT_SIZE (4)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;

			// REPORT_COUNT (1)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
							
			// INPUT (Data,Var,Abs)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		} else {
		
			// Use Padding Bits
		
			// REPORT_SIZE (1)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

			// REPORT_COUNT (4)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;
					
			// INPUT (Const,Var,Abs)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x03;
			
		} // One or Two Hat Switches?

	} // Hat Switches

	if (axisCount > 0) {
	
		// USAGE (Pointer)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// LOGICAL_MINIMUM (-32767)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x16;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x80;

		// LOGICAL_MAXIMUM (+32767)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x26;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x7F;

		// REPORT_SIZE (16)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x10;

		// REPORT_COUNT (axisCount)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
		tempHidReportDescriptor[hidReportDescriptorSize++] = axisCount;
						
		// COLLECTION (Physical)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xA1;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		if (includeXAxis == true) {
			// USAGE (X)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x30;
		}

		if (includeYAxis == true) {
			// USAGE (Y)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x31;
		}
		
		if (includeZAxis == true) {
			// USAGE (Z)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x32;
		}
		
		if (includeRxAxis == true) {
			// USAGE (Rx)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x33;
		}
		
		if (includeRyAxis == true) {
			// USAGE (Ry)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x34;
		}
		
		if (includeRzAxis == true) {
			// USAGE (Rz)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x35;
		}
		
		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		// END_COLLECTION (Physical)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;
		
	} // X, Y, Z, Rx, Ry, and Rz Axis	
	
	if (simulationCount > 0) {
	
		// USAGE_PAGE (Simulation Controls)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		// LOGICAL_MINIMUM (-32767)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x16;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x80;

		// LOGICAL_MAXIMUM (+32767)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x26;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xFF;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x7F;

		// REPORT_SIZE (16)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x10;

		// REPORT_COUNT (simulationCount)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
		tempHidReportDescriptor[hidReportDescriptorSize++] = simulationCount;

		// COLLECTION (Physical)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xA1;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		if (includeRudder == true) {
			// USAGE (Rudder)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0xBA;
		}

		if (includeThrottle == true) {
			// USAGE (Throttle)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0xBB;
		}

		if (includeAccelerator == true) {
			// USAGE (Accelerator)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0xC4;
		}

		if (includeBrake == true) {
			// USAGE (Brake)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0xC5;
		}

		if (includeSteering == true) {
			// USAGE (Steering)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0xC8;
		}

		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		// END_COLLECTION (Physical) 
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;
	} // Simulation Controls

    // END_COLLECTION
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;

	// Create a copy of the HID Report Descriptor template that is just the right size
	uint8_t *customHidReportDescriptor = new uint8_t[hidReportDescriptorSize];
	memcpy(customHidReportDescriptor, tempHidReportDescriptor, hidReportDescriptorSize);
	// Register HID Report Description
	DynamicHIDSubDescriptor* node = new DynamicHIDSubDescriptor(customHidReportDescriptor, hidReportDescriptorSize,pidReportDescriptor, pidReportDescriptorSize, false);
	
	DynamicHID().AppendDescriptor(node);
	
    // Setup Joystick State
	if (buttonCount > 0) {
		_buttonValuesArraySize = _buttonCount / 8;
		if ((_buttonCount % 8) > 0) {
			_buttonValuesArraySize++;
		}
		_buttonValues = new uint8_t[_buttonValuesArraySize];
	}
	
	// Calculate HID Report Size
	_hidReportSize = _buttonValuesArraySize;
	_hidReportSize += (_hatSwitchCount > 0);
	_hidReportSize += (axisCount * 2);
	_hidReportSize += (simulationCount * 2);
	
	// Initalize Joystick State
	_xAxis = 0;
	_yAxis = 0;
	_zAxis = 0;
	_xAxisRotation = 0;
	_yAxisRotation = 0;
	_zAxisRotation = 0;
	_throttle = 0;
	_rudder = 0;
	_accelerator = 0;
	_brake = 0;
	_steering = 0;
	for (int index = 0; index < JOYSTICK_HATSWITCH_COUNT_MAXIMUM; index++)
	{
		_hatSwitchValues[index] = JOYSTICK_HATSWITCH_RELEASE;
	}
    for (int index = 0; index < _buttonValuesArraySize; index++)
    {
        _buttonValues[index] = 0;
    }
}

void Joystick_::begin(bool initAutoSendState)
{
	_autoSendState = initAutoSendState;
	sendState();
}

void Joystick_::getUSBPID()
{
	DynamicHID().RecvfromUsb();
}

void Joystick_::getForce(int32_t* forces) 
{
	forceCalculator(forces);
}

int32_t Joystick_::getEffectForce(volatile TEffectState& effect, Gains _gains, EffectParams _effect_params, uint8_t axis){
	uint8_t direction;
    uint8_t condition;
	bool useForceDirectionForConditionEffect = (effect.enableAxis == DIRECTION_ENABLE && effect.conditionBlocksCount == 1);

    if (effect.enableAxis == DIRECTION_ENABLE)
    {
        direction = effect.directionX;
		if (effect.conditionBlocksCount > 1) {
            condition = axis;
        } else {
	        condition = 0; // only one Condition Parameter Block is defined
		}
    }
    else
    {
        direction = axis == 0 ? effect.directionX : effect.directionY;
        condition = axis;
    }

    float angle = (direction * 360.0 / 255.0) * DEG_TO_RAD;
    float angle_ratio = axis == 0 ? sin(angle) : -1 * cos(angle);
	int32_t force = 0;
	switch (effect.effectType)
    {
	    case USB_EFFECT_CONSTANT://1
	        force = ConstantForceCalculator(effect) * _gains.constantGain * angle_ratio;
	        break;
	    case USB_EFFECT_RAMP://2
	    	force = RampForceCalculator(effect) * _gains.rampGain * angle_ratio;
	    	break;
	    case USB_EFFECT_SQUARE://3
	    	force = SquareForceCalculator(effect) * _gains.squareGain * angle_ratio;
	    	break;
	    case USB_EFFECT_SINE://4
	    	force = SinForceCalculator(effect) * _gains.sineGain * angle_ratio;
	    	break;
	    case USB_EFFECT_TRIANGLE://5
	    	force = TriangleForceCalculator(effect) * _gains.triangleGain * angle_ratio;
	    	break;
	    case USB_EFFECT_SAWTOOTHDOWN://6
	    	force = SawtoothDownForceCalculator(effect) * _gains.sawtoothdownGain * angle_ratio;
	    	break;
	    case USB_EFFECT_SAWTOOTHUP://7
	    	force = SawtoothUpForceCalculator(effect) * _gains.sawtoothupGain * angle_ratio;
	    	break;
	    case USB_EFFECT_SPRING://8
	    	force = ConditionForceCalculator(effect, NormalizeRange(_effect_params.springPosition, _effect_params.springMaxPosition), condition) * _gains.springGain;
	    	if (useForceDirectionForConditionEffect) {
				force *= angle_ratio;
			}
			break;
	    case USB_EFFECT_DAMPER://9
	    	force = ConditionForceCalculator(effect, NormalizeRange(_effect_params.damperVelocity, _effect_params.damperMaxVelocity), condition) * _gains.damperGain;
	    	if (useForceDirectionForConditionEffect) {
				force *= angle_ratio;
			}
			break;
	    case USB_EFFECT_INERTIA://10
	    	if (_effect_params.inertiaAcceleration < 0 && _effect_params.frictionPositionChange < 0) {
	    		force = ConditionForceCalculator(effect, abs(NormalizeRange(_effect_params.inertiaAcceleration, _effect_params.inertiaMaxAcceleration)), condition) * _gains.inertiaGain;
	    	}
	    	else if (_effect_params.inertiaAcceleration < 0 && _effect_params.frictionPositionChange > 0) {
	    		force = -1 * ConditionForceCalculator(effect, abs(NormalizeRange(_effect_params.inertiaAcceleration, _effect_params.inertiaMaxAcceleration)), condition) * _gains.inertiaGain;
	    	}
			if (useForceDirectionForConditionEffect) {
				force *= angle_ratio;
			}
	    	break;
	    case USB_EFFECT_FRICTION://11
	    		force = ConditionForceCalculator(effect, NormalizeRange(_effect_params.frictionPositionChange, _effect_params.frictionMaxPositionChange), condition) * _gains.frictionGain;
	    		if (useForceDirectionForConditionEffect) {
				    force *= angle_ratio;
			    }
				break;
	    case USB_EFFECT_CUSTOM://12
	    		break;
	    }
	    effect.elapsedTime = (uint64_t)millis() - effect.startTime;
		return force;
}


void Joystick_::forceCalculator(int32_t* forces) {
	forces[0] = 0;
    forces[1] = 0;
	    for (int id = 0; id < MAX_EFFECTS; id++) {
	    	volatile TEffectState& effect = DynamicHID().pidReportHandler.g_EffectStates[id];
	    	if ((effect.state == MEFFECTSTATE_PLAYING) &&
	    		((effect.elapsedTime <= effect.duration) ||
	    		(effect.duration == USB_DURATION_INFINITE)) && !DynamicHID().pidReportHandler.devicePaused)
	    	{
				forces[0] += (int32_t)(getEffectForce(effect, m_gains[0], m_effect_params[0], 0));
				forces[1] += (int32_t)(getEffectForce(effect, m_gains[1], m_effect_params[1], 1));
	    	}
	    }
	forces[0] = (int32_t)((float)1.0 * forces[0] * m_gains[0].totalGain / 10000); // each effect gain * total effect gain = 10000
	forces[1] = (int32_t)((float)1.0 * forces[1] * m_gains[1].totalGain / 10000); // each effect gain * total effect gain = 10000
	forces[0] = map(forces[0], -10000, 10000, -250, 250);
	forces[1] = map(forces[1], -10000, 10000, -250, 250);
}

int32_t Joystick_::ConstantForceCalculator(volatile TEffectState& effect) 
{
	return ApplyEnvelope(effect, (int32_t)effect.magnitude);
}

int32_t Joystick_::RampForceCalculator(volatile TEffectState& effect) 
{
	int32_t tempforce = (int32_t)(effect.startMagnitude + effect.elapsedTime * 1.0 * (effect.endMagnitude - effect.startMagnitude) / effect.duration);
	return ApplyEnvelope(effect, tempforce);
}

int32_t Joystick_::SquareForceCalculator(volatile TEffectState& effect)
{
	int16_t offset = effect.offset * 2;
	int16_t magnitude = effect.magnitude;
	uint16_t phase = effect.phase;
	uint16_t elapsedTime = effect.elapsedTime;
	uint16_t period = effect.period;

	int32_t maxMagnitude = offset + magnitude;
	int32_t minMagnitude = offset - magnitude;
	uint32_t phasetime = (phase * period) / 255;
	uint32_t timeTemp = elapsedTime + phasetime;
	uint32_t reminder = timeTemp % period;
	int32_t tempforce;
	if (reminder > (period / 2)) tempforce = minMagnitude;
	else tempforce = maxMagnitude;
	return ApplyEnvelope(effect, tempforce);
}

int32_t Joystick_::SinForceCalculator(volatile TEffectState& effect) 
{
	int16_t offset = effect.offset * 2;
	int16_t magnitude = effect.magnitude;
	uint16_t phase = effect.phase;
	uint16_t timeTemp = effect.elapsedTime;
	uint16_t period = effect.period;
	float angle = 0.0;
	if(period != 0)
		angle = ((timeTemp * 1.0 / period) * 2 * PI + (phase / 36000.0));
	float sine = sin(angle);
	int32_t tempforce = (int32_t)(sine * magnitude);
	tempforce += offset;
	return ApplyEnvelope(effect, tempforce);
}

int32_t Joystick_::TriangleForceCalculator(volatile TEffectState& effect)
{
	int16_t offset = effect.offset * 2;
	int16_t magnitude = effect.magnitude;
	uint16_t elapsedTime = effect.elapsedTime;
	uint16_t phase = effect.phase;
	uint16_t period = effect.period;
	uint16_t periodF = effect.period;

	int16_t maxMagnitude = offset + magnitude;
	int16_t minMagnitude = offset - magnitude;
	int32_t phasetime = (phase * period) / 255;
	uint32_t timeTemp = elapsedTime + phasetime;
	int32_t reminder = timeTemp % period;
	int32_t slope = ((maxMagnitude - minMagnitude) * 2) / periodF;
	int32_t tempforce = 0;
	if (reminder > (periodF / 2)) tempforce = slope * (periodF - reminder);
	else tempforce = slope * reminder;
	tempforce += minMagnitude;
	return ApplyEnvelope(effect, tempforce);
}

int32_t Joystick_::SawtoothDownForceCalculator(volatile TEffectState& effect) 
{
	int16_t offset = effect.offset * 2;
	int16_t magnitude = effect.magnitude;
	uint16_t elapsedTime = effect.elapsedTime;
	uint16_t phase = effect.phase;
	uint16_t period = effect.period;
	uint16_t periodF = effect.period;

	int16_t maxMagnitude = offset + magnitude;
	int16_t minMagnitude = offset - magnitude;
	int32_t phasetime = (phase * period) / 255;
	uint32_t timeTemp = elapsedTime + phasetime;
	int32_t reminder = timeTemp % period;
	int32_t slope = (maxMagnitude - minMagnitude) / periodF;
	int32_t tempforce = 0;
	tempforce = slope * (period - reminder);
	tempforce += minMagnitude;
	return ApplyEnvelope(effect, tempforce);
}

int32_t Joystick_::SawtoothUpForceCalculator(volatile TEffectState& effect) 
{
	int16_t offset = effect.offset * 2;
	int16_t magnitude = effect.magnitude;
	uint16_t elapsedTime = effect.elapsedTime;
	uint16_t phase = effect.phase;
	uint16_t period = effect.period;
	uint16_t periodF = effect.period;

	int16_t maxMagnitude = offset + magnitude;
	int16_t minMagnitude = offset - magnitude;
	int32_t phasetime = (phase * period) / 255;
	uint32_t timeTemp = elapsedTime + phasetime;
	int32_t reminder = timeTemp % period;
	int32_t slope = (maxMagnitude - minMagnitude) / periodF;
	int32_t tempforce = 0;
	tempforce = slope * reminder;
	tempforce += minMagnitude;
	return ApplyEnvelope(effect, tempforce);
}

int32_t Joystick_::ConditionForceCalculator(volatile TEffectState& effect, float metric, uint8_t axis)
{
	float deadBand;
	float cpOffset;
	float positiveCoefficient;
	float negativeCoefficient;
	float positiveSaturation;
	float negativeSaturation;

    deadBand = effect.conditions[axis].deadBand;
    cpOffset = effect.conditions[axis].cpOffset;
    negativeCoefficient = effect.conditions[axis].negativeCoefficient;
    negativeSaturation = effect.conditions[axis].negativeSaturation;
    positiveSaturation = effect.conditions[axis].positiveSaturation;
    positiveCoefficient = effect.conditions[axis].positiveCoefficient;

	float  tempForce = 0;
	if (metric < (cpOffset - deadBand)) 
	{
		tempForce = (metric - (float)1.00*(cpOffset - deadBand)/10000) * negativeCoefficient;
		tempForce = (tempForce < -negativeSaturation ? -negativeSaturation : tempForce);
	}
	else if (metric > (cpOffset + deadBand)) 
	{
		tempForce = (metric - (float)1.00 * (cpOffset + deadBand) / 10000) * positiveCoefficient;
		tempForce = (tempForce > positiveSaturation ? positiveSaturation : tempForce);
	}
	else return 0;
	tempForce = -tempForce * effect.gain / 255;
	switch (effect.effectType) {
	case  USB_EFFECT_DAMPER:
		//tempForce = damperFilter.filterIn(tempForce);
		break;
	case USB_EFFECT_INERTIA:
		//tempForce = interiaFilter.filterIn(tempForce);
		break;
	case USB_EFFECT_FRICTION:
		//tempForce = frictionFilter.filterIn(tempForce);
		break;
	default:
		break;
	}
	return (int32_t)tempForce;
}

inline float Joystick_::NormalizeRange(int32_t x, int32_t maxValue) {
	return (float)x * 1.00 / maxValue;
}

inline int32_t  Joystick_::ApplyGain(int16_t value, uint8_t gain)
{
	int32_t value_32 = value;
	return ((value_32 * gain) / 255);
}

inline int32_t Joystick_::ApplyEnvelope(volatile TEffectState& effect, int32_t value)
{
	int32_t magnitude = ApplyGain(effect.magnitude, effect.gain);
	int32_t attackLevel = ApplyGain(effect.attackLevel, effect.gain);
	int32_t fadeLevel = ApplyGain(effect.fadeLevel, effect.gain);
	int32_t newValue = magnitude;
	int32_t attackTime = effect.attackTime;
	int32_t fadeTime = effect.fadeTime;
	int32_t elapsedTime = effect.elapsedTime;
	int32_t duration = effect.duration;

	if (elapsedTime < attackTime)
	{
		newValue = (magnitude - attackLevel) * elapsedTime / attackTime;
		newValue += attackLevel;
	}
	if (elapsedTime > (duration - fadeTime))
	{
		newValue = (magnitude - fadeLevel) * (duration - elapsedTime);
		newValue /= fadeTime;
		newValue += fadeLevel;
	}
	newValue = newValue * value / magnitude;
	return newValue;
}

void Joystick_::end()
{
}

void Joystick_::setButton(uint8_t button, uint8_t value)
{
	if (value == 0)
	{
		releaseButton(button);
	}
	else
	{
		pressButton(button);
	}
}
void Joystick_::pressButton(uint8_t button)
{
    if (button >= _buttonCount) return;

    int index = button / 8;
    int bit = button % 8;

	bitSet(_buttonValues[index], bit);
	if (_autoSendState) sendState();
}
void Joystick_::releaseButton(uint8_t button)
{
    if (button >= _buttonCount) return;

    int index = button / 8;
    int bit = button % 8;

    bitClear(_buttonValues[index], bit);
	if (_autoSendState) sendState();
}

void Joystick_::setXAxis(int16_t value)
{
	_xAxis = value;
	if (_autoSendState) sendState();
}
void Joystick_::setYAxis(int16_t value)
{
	_yAxis = value;
	if (_autoSendState) sendState();
}
void Joystick_::setZAxis(int16_t value)
{
	_zAxis = value;
	if (_autoSendState) sendState();
}

void Joystick_::setRxAxis(int16_t value)
{
	_xAxisRotation = value;
	if (_autoSendState) sendState();
}
void Joystick_::setRyAxis(int16_t value)
{
	_yAxisRotation = value;
	if (_autoSendState) sendState();
}
void Joystick_::setRzAxis(int16_t value)
{
	_zAxisRotation = value;
	if (_autoSendState) sendState();
}

void Joystick_::setRudder(int16_t value)
{
	_rudder = value;
	if (_autoSendState) sendState();
}
void Joystick_::setThrottle(int16_t value)
{
	_throttle = value;
	if (_autoSendState) sendState();
}
void Joystick_::setAccelerator(int16_t value)
{
	_accelerator = value;
	if (_autoSendState) sendState();
}
void Joystick_::setBrake(int16_t value)
{
	_brake = value;
	if (_autoSendState) sendState();
}
void Joystick_::setSteering(int16_t value)
{
	_steering = value;
	if (_autoSendState) sendState();
}

void Joystick_::setHatSwitch(int8_t hatSwitchIndex, int16_t value)
{
	if (hatSwitchIndex >= _hatSwitchCount) return;
	
	_hatSwitchValues[hatSwitchIndex] = value;
	if (_autoSendState) sendState();
}

int Joystick_::buildAndSet16BitValue(bool includeValue, int16_t value, int16_t valueMinimum, int16_t valueMaximum, int16_t actualMinimum, int16_t actualMaximum, uint8_t dataLocation[]) 
{
	int16_t convertedValue;
	uint8_t highByte;
	uint8_t lowByte;
	int16_t realMinimum = min(valueMinimum, valueMaximum);
	int16_t realMaximum = max(valueMinimum, valueMaximum);

	if (includeValue == false) return 0;

	if (value < realMinimum) {
		value = realMinimum;
	}
	if (value > realMaximum) {
		value = realMaximum;
	}

	if (valueMinimum > valueMaximum) {
		// Values go from a larger number to a smaller number (e.g. 1024 to 0)
		value = realMaximum - value + realMinimum;
	}

	convertedValue = map(value, realMinimum, realMaximum, actualMinimum, actualMaximum);

	highByte = (uint8_t)(convertedValue >> 8);
	lowByte = (uint8_t)(convertedValue & 0x00FF);
	
	dataLocation[0] = lowByte;
	dataLocation[1] = highByte;
	
	return 2;
}

int Joystick_::buildAndSetAxisValue(bool includeAxis, int16_t axisValue, int16_t axisMinimum, int16_t axisMaximum, uint8_t dataLocation[]) 
{
	return buildAndSet16BitValue(includeAxis, axisValue, axisMinimum, axisMaximum, JOYSTICK_AXIS_MINIMUM, JOYSTICK_AXIS_MAXIMUM, dataLocation);
}

int Joystick_::buildAndSetSimulationValue(bool includeValue, int16_t value, int16_t valueMinimum, int16_t valueMaximum, uint8_t dataLocation[]) 
{
	return buildAndSet16BitValue(includeValue, value, valueMinimum, valueMaximum, JOYSTICK_SIMULATOR_MINIMUM, JOYSTICK_SIMULATOR_MAXIMUM, dataLocation);
}

void Joystick_::sendState()
{
	uint8_t data[_hidReportSize];
	int index = 0;
	
	// Load Button State
	for (; index < _buttonValuesArraySize; index++)
	{
		data[index] = _buttonValues[index];		
	}

	// Set Hat Switch Values
	if (_hatSwitchCount > 0) {
		
		// Calculate hat-switch values
		uint8_t convertedHatSwitch[JOYSTICK_HATSWITCH_COUNT_MAXIMUM];
		for (int hatSwitchIndex = 0; hatSwitchIndex < JOYSTICK_HATSWITCH_COUNT_MAXIMUM; hatSwitchIndex++)
		{
			if (_hatSwitchValues[hatSwitchIndex] < 0)
			{
				convertedHatSwitch[hatSwitchIndex] = 8;
			}
			else
			{
				convertedHatSwitch[hatSwitchIndex] = (_hatSwitchValues[hatSwitchIndex] % 360) / 45;
			}			
		}

		// Pack hat-switch states into a single byte
		data[index++] = (convertedHatSwitch[1] << 4) | (B00001111 & convertedHatSwitch[0]);
	
	} // Hat Switches

	// Set Axis Values
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_X_AXIS, _xAxis, _xAxisMinimum, _xAxisMaximum, &(data[index]));
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_Y_AXIS, _yAxis, _yAxisMinimum, _yAxisMaximum, &(data[index]));
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_Z_AXIS, _zAxis, _zAxisMinimum, _zAxisMaximum, &(data[index]));
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_RX_AXIS, _xAxisRotation, _rxAxisMinimum, _rxAxisMaximum, &(data[index]));
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_RY_AXIS, _yAxisRotation, _ryAxisMinimum, _ryAxisMaximum, &(data[index]));
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_RZ_AXIS, _zAxisRotation, _rzAxisMinimum, _rzAxisMaximum, &(data[index]));
	
	// Set Simulation Values
	index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_RUDDER, _rudder, _rudderMinimum, _rudderMaximum, &(data[index]));
	index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_THROTTLE, _throttle, _throttleMinimum, _throttleMaximum, &(data[index]));
	index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_ACCELERATOR, _accelerator, _acceleratorMinimum, _acceleratorMaximum, &(data[index]));
	index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_BRAKE, _brake, _brakeMinimum, _brakeMaximum, &(data[index]));
	index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_STEERING, _steering, _steeringMinimum, _steeringMaximum, &(data[index]));

	DynamicHID().SendReport(_hidReportId, data, _hidReportSize);
}

#endif
