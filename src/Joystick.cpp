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
#include "../config.h"
#include "FFBDescriptor.h"
#include "filters.h"
#ifdef damperSplineGain
#include "spline.h"
#endif
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

const float cutoff_freq_damper   = 2.0;  //Cutoff frequency in Hz
const float sampling_time_damper = 0.002; //Sampling time in seconds.
LowPassFilter damperFilter[FFB_AXIS_COUNT];
LowPassFilter inertiaFilter[FFB_AXIS_COUNT];
LowPassFilter frictionFilter[FFB_AXIS_COUNT];

#ifdef damperSplineGain
damperSplineGain;
#endif


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
	bool includeThrottle)
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
		+ (includeThrottle == true); 
		
	static uint8_t tempHidReportDescriptor[150];
	int hidReportDescriptorSize = 0;

 //   // USAGE_PAGE (Generic Desktop)
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

		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		// END_COLLECTION (Physical) 
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;
	} // Simulation Controls

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
    for (int i=0; i <= FFB_AXIS_COUNT; ++i)
    {
        damperFilter[i] = LowPassFilter(cutoff_freq_damper, sampling_time_damper);
        inertiaFilter[i] = LowPassFilter(cutoff_freq_damper, sampling_time_damper);
        frictionFilter[i] = LowPassFilter(cutoff_freq_damper, sampling_time_damper);
    }
}

void Joystick_::getForce(int32_t* forces) {
	DynamicHID().RecvfromUsb();
	forceCalculator(forces);
}

float Joystick_::getAngleRatio(volatile TEffectState& effect, int axis)
{
    float angle = (axis < 2 ? effect.direction[0] : effect.direction[1]) * 360.0 / 255.0 * DEG_TO_RAD;
    if (axis == 0)
    {
        // angle=0 points "up"
        return -1 * sin(angle);
    } else {
        return cos(angle);
    }
}

int32_t Joystick_::getEffectForce(volatile TEffectState& effect, EffectParams _effect_params, uint8_t axis){

    // if (effect.state == MEFFECTSTATE_PLAYING)
    // {
    //    Serial.print("eA");
    //    Serial.print(effect.enableAxis);
    //    Serial.print("dX");
    //    Serial.print(effect.direction[0]);
    //    Serial.print("dY");
    //    Serial.println(effect.direction[1]);
    // }

    float angle_ratio;
    uint8_t condition = 0;
    if (effect.enableAxis == DIRECTION_ENABLE && effect.conditionReportsCount > 1)
    {
        angle_ratio = 1.0;
        condition = axis;
    }
    else
    {
        angle_ratio = getAngleRatio(effect, axis);
    }

	int32_t force = 0;
	switch (effect.effectType)
    {
	    case USB_EFFECT_CONSTANT://1
	        force = ConstantForceCalculator(effect) * constantGain * angle_ratio;
	        break;
	    case USB_EFFECT_RAMP://2
	    	force = RampForceCalculator(effect) * rampGain * angle_ratio;
	    	break;
	    case USB_EFFECT_SQUARE://3
	    	force = SquareForceCalculator(effect) * squareGain * angle_ratio;
	    	break;
	    case USB_EFFECT_SINE://4
	    	force = SinForceCalculator(effect) * sineGain * angle_ratio;
	    	break;
	    case USB_EFFECT_TRIANGLE://5
	    	force = TriangleForceCalculator(effect) * triangleGain * angle_ratio;
	    	break;
	    case USB_EFFECT_SAWTOOTHDOWN://6
	    	force = SawtoothDownForceCalculator(effect) * sawtoothdownGain * angle_ratio;
	    	break;
	    case USB_EFFECT_SAWTOOTHUP://7
	    	force = SawtoothUpForceCalculator(effect) * sawtoothupGain * angle_ratio;
	    	break;
	    case USB_EFFECT_SPRING://8
	    	force = ConditionForceCalculator(effect, NormalizeRange(_effect_params.springPosition, m_effect_params[axis].springMaxPosition), condition) * angle_ratio * springGain;
	    	break;
	    case USB_EFFECT_DAMPER://9
	    	force = ConditionForceCalculator(effect, NormalizeRange(_effect_params.damperVelocity, m_effect_params[axis].damperMaxVelocity), condition) * angle_ratio;
            #ifdef damperSplineGain
            force *= (
                Interpolation::CatmullSpline(damperSplinePoints[0], damperSplinePoints[1], damperSplineNumPoints, abs(force))
                /10000.0);
            #else
	    	force = force * damperGain;
            #endif
            force = damperFilter[axis].update(force);
	    	break;
	    case USB_EFFECT_INERTIA://10
	    	if (_effect_params.inertiaAcceleration < 0 && _effect_params.frictionPositionChange < 0) {
	    		force = ConditionForceCalculator(effect, abs(NormalizeRange(_effect_params.inertiaAcceleration, m_effect_params[axis].inertiaMaxAcceleration)), condition) * angle_ratio * inertiaGain;
	    	}
	    	else if (_effect_params.inertiaAcceleration < 0 && _effect_params.frictionPositionChange > 0) {
	    		force = -1 * ConditionForceCalculator(effect, abs(NormalizeRange(_effect_params.inertiaAcceleration, m_effect_params[axis].inertiaMaxAcceleration)), condition) * angle_ratio * inertiaGain;
	    	}
            force = inertiaFilter[axis].update(force);
	    	break;
	    case USB_EFFECT_FRICTION://11
	    	force = ConditionForceCalculator(effect, NormalizeRange(_effect_params.frictionPositionChange, m_effect_params[axis].frictionMaxPositionChange), condition) * angle_ratio * frictionGain;
            force = frictionFilter[axis].update(force);
	    	break;
	    case USB_EFFECT_CUSTOM://12
	    	break;
	    }

		return force;
}


void Joystick_::forceCalculator(int32_t* forces) {
    forces[0] = 0;
    forces[1] = 0;
        int32_t force = 0;
	    for (int id = 0; id < MAX_EFFECTS; id++) {
	    	volatile TEffectState& effect = DynamicHID().pidReportHandler.g_EffectStates[id];

            Serial.print("id:");
            Serial.print(id);
            Serial.print("state:");
            Serial.print(effect.state);
            Serial.print("effectType:");
            Serial.print(effect.effectType);
            Serial.println("");

            effect.elapsedTime = (uint64_t)millis() - effect.startTime;
            // totalDuration counts all repetitions (duration+delay) * loopCount
            if ((effect.totalDuration == USB_DURATION_INFINITE) ||
                (effect.elapsedTime < effect.totalDuration))
            {
                // if we are still running the effect
                // show where in the loop we are
                effect.elapsedTime = effect.elapsedTime % (effect.duration + effect.startDelay);
            }
            effect.elapsedTime -= effect.startDelay;

	    	if ((effect.state == MEFFECTSTATE_PLAYING) &&
                // dont calculate effects that havent reached their startDelay
	    	    (effect.elapsedTime >= 0) &&
                // dont calculate effects that have already finished
	    	    (effect.elapsedTime <= effect.duration) &&
	    		!DynamicHID().pidReportHandler.devicePaused)
            {
                // if this is a directional conditional calculate the conditional parameters
                // as the length in the direction of its angle. This is the same as the dot product of the vectors
                EffectParams direction_effect_params;
                if (effect.conditionReportsCount == 1)
                {
                    for (int axis = 0; axis < FFB_AXIS_COUNT; ++axis)
                    {
                        direction_effect_params.springPosition += m_effect_params[axis].springPosition * getAngleRatio(effect, axis);
                        direction_effect_params.damperVelocity += m_effect_params[axis].damperVelocity * getAngleRatio(effect, axis);
                        direction_effect_params.inertiaAcceleration += m_effect_params[axis].inertiaAcceleration * getAngleRatio(effect, axis);
                        direction_effect_params.frictionPositionChange += m_effect_params[axis].frictionPositionChange * getAngleRatio(effect, axis);
                    }
                }

                for (int axis = 0; axis < FFB_AXIS_COUNT; ++axis)
                {
                    if (effect.enableAxis == DIRECTION_ENABLE
                        || effect.enableAxis & (1 << axis))
                    {
                        forces[axis] += (int32_t)(getEffectForce(effect, effect.conditionReportsCount == 1 ? direction_effect_params : m_effect_params[axis], axis));
                    }
                }
            }

            //if (effect.state == MEFFECTSTATE_PLAYING)
            //{
            //    Serial.print("eT");
            //    Serial.print(effect.elapsedTime);
            //    Serial.print("sD");
            //    Serial.print(effect.startDelay);
            //    Serial.print("d");
            //    Serial.print(effect.duration);
            //    Serial.print("tD");
            //    Serial.println(effect.totalDuration);
            //}
	    }

    for (int axis = 0; axis < FFB_AXIS_COUNT; ++axis)
    {
        forces[axis] = (int32_t)((float)totalGain * forces[axis]); // each effect gain * total effect gain = 10000
        forces[axis] = constrain(forces[axis], -10000, 10000);
    }
}

int32_t Joystick_::ConstantForceCalculator(volatile TEffectState& effect) 
{
	float tempforce = (float)effect.magnitude * effect.gain / 255;
	return (int32_t)tempforce;
}

int32_t Joystick_::RampForceCalculator(volatile TEffectState& effect) 
{
	int32_t rampForce = effect.startMagnitude + ((float)effect.elapsedTime / effect.duration) * (effect.endMagnitude - effect.startMagnitude);
	return rampForce;
}

int32_t Joystick_::SquareForceCalculator(volatile TEffectState& effect)
{
	int32_t offset = effect.offset * 2;
	uint32_t magnitude = effect.magnitude;
	uint32_t elapsedTime = effect.elapsedTime;
	uint32_t phase = effect.phase;
	uint32_t period = effect.period;

	int32_t maxMagnitude = offset + magnitude;
	int32_t minMagnitude = offset - magnitude;
	uint32_t phasetime = (phase * period) / 36000;
	uint32_t timeTemp = elapsedTime + phasetime;
	uint32_t reminder = timeTemp % period;
	int32_t tempforce;
	if (reminder > (period / 2)) tempforce = minMagnitude;
	else tempforce = maxMagnitude;
	return ApplyEnvelope(effect, tempforce);
}

int32_t Joystick_::SinForceCalculator(volatile TEffectState& effect) 
{
	float offset = effect.offset * 2;
	float magnitude = effect.magnitude;
	float phase = effect.phase;
	float timeTemp = effect.elapsedTime;
	float period = effect.period;
	float angle = ((timeTemp / period) + (phase / 36000.0)) * 2 * PI;
	float sine = sin(angle);
	float tempforce = sine * magnitude;
	tempforce += offset;
	return ApplyEnvelope(effect, tempforce);
}

int32_t Joystick_::TriangleForceCalculator(volatile TEffectState& effect)
{
	float offset = effect.offset * 2;
	float magnitude = effect.magnitude;
	float elapsedTime = effect.elapsedTime;
	uint32_t phase = effect.phase;
	uint32_t period = effect.period;
	float periodF = effect.period;

	float maxMagnitude = offset + magnitude;
	float minMagnitude = offset - magnitude;
	uint32_t phasetime = (phase * period) / 36000;
	uint32_t timeTemp = elapsedTime + phasetime;
	float reminder = timeTemp % period;
	float slope = ((maxMagnitude - minMagnitude) * 2) / periodF;
	float tempforce = 0;
	if (reminder > (periodF / 2)) tempforce = slope * (periodF - reminder);
	else tempforce = slope * reminder;
	tempforce += minMagnitude;
	return ApplyEnvelope(effect, -tempforce);
}

int32_t Joystick_::SawtoothDownForceCalculator(volatile TEffectState& effect) 
{
	float offset = effect.offset * 2;
	float magnitude = effect.magnitude;
	float elapsedTime = effect.elapsedTime;
	float phase = effect.phase;
	uint32_t period = effect.period;
	float periodF = effect.period;

	float maxMagnitude = offset + magnitude;
	float minMagnitude = offset - magnitude;
	int32_t phasetime = (phase * period) / 36000;
	uint32_t timeTemp = elapsedTime + phasetime;
	float reminder = timeTemp % period;
	float slope = (maxMagnitude - minMagnitude) / periodF;
	float tempforce = 0;
	tempforce = slope * (period - reminder);
	tempforce += minMagnitude;
	return ApplyEnvelope(effect, -tempforce);
}

int32_t Joystick_::SawtoothUpForceCalculator(volatile TEffectState& effect) 
{
	float offset = effect.offset * 2;
	float magnitude = effect.magnitude;
	float elapsedTime = effect.elapsedTime;
	uint32_t phase = effect.phase;
	uint32_t period = effect.period;
	float periodF = effect.period;

	float maxMagnitude = offset + magnitude;
	float minMagnitude = offset - magnitude;
	int32_t phasetime = (phase * period) / 36000;
	uint32_t timeTemp = elapsedTime + phasetime;
	float reminder = timeTemp % period;
	float slope = (maxMagnitude - minMagnitude) / periodF;
	float tempforce = 0;
	tempforce = slope * reminder;
	tempforce += minMagnitude;
	return ApplyEnvelope(effect, -tempforce);
}

int32_t Joystick_::ConditionForceCalculator(volatile TEffectState& effect, float metric, uint8_t conditionReport)
{
    float deadBand;
    float cpOffset;
    float negativeCoefficient;
    float negativeSaturation;
    float positiveSaturation;
    float positiveCoefficient;

    deadBand = effect.conditions[conditionReport].deadBand;
    cpOffset = effect.conditions[conditionReport].cpOffset;
    negativeCoefficient = effect.conditions[conditionReport].negativeCoefficient;
    negativeSaturation = effect.conditions[conditionReport].negativeSaturation;
    positiveSaturation = effect.conditions[conditionReport].positiveSaturation;
    positiveCoefficient = effect.conditions[conditionReport].positiveCoefficient;

    float  tempForce = 0;

	if (metric < (cpOffset - deadBand)) {
		tempForce = (metric - (float)1.00*(cpOffset - deadBand)/10000) * negativeCoefficient;
		// tempForce = ((float)1.00 * (cpOffset - deadBand) / 10000 - metric) * negativeCoefficient;
		   tempForce = (tempForce < -negativeSaturation ? -negativeSaturation : tempForce); // I dont know why negativeSaturation = 55536.00 after negativeSaturation = -effect.negativeSaturation;
		// tempForce = (tempForce < (-effect.negativeCoefficient) ? (-effect.negativeCoefficient) : tempForce);
	}
	else if (metric > (cpOffset + deadBand)) {
		tempForce = (metric - (float)1.00 * (cpOffset + deadBand) / 10000) * positiveCoefficient;
		tempForce = (tempForce > positiveSaturation ? positiveSaturation : tempForce);
	}
	else return 0;
	tempForce = -tempForce * effect.gain / 255;
	return (int32_t)tempForce;
}

float Joystick_::NormalizeRange(int32_t x, int32_t maxValue) {
    float value = (float)x * 1.00 / maxValue;
	return ((value > 1.0 ? 1.0 : value) < -1.0 ? -1.0 : value);
}

int32_t  Joystick_::ApplyGain(uint16_t value, uint8_t gain)
{
	int32_t value_32 = value;
	return ((value_32 * gain) / 255);
}

int32_t Joystick_::ApplyEnvelope(volatile TEffectState& effect, int32_t value)
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
		newValue = (magnitude - attackLevel) * elapsedTime;
		newValue /= attackTime;
		newValue += attackLevel;
	}
	if (elapsedTime > (duration - fadeTime))
	{
		newValue = (magnitude - fadeLevel) * (duration - elapsedTime);
		newValue /= fadeTime;
		newValue += fadeLevel;
	}

	newValue *= value;
	newValue /= magnitude;
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

	DynamicHID().SendReport(_hidReportId, data, _hidReportSize);
}

#endif
