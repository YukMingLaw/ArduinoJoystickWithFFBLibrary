/*
  Joystick.h

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

#ifndef JOYSTICK_h
#define JOYSTICK_h

#include "DynamicHID/DynamicHID.h"

#if ARDUINO < 10606
#error The Joystick library requires Arduino IDE 1.6.6 or greater. Please update your IDE.
#endif // ARDUINO < 10606

#if ARDUINO > 10606
#if !defined(USBCON)
#error The Joystick library can only be used with a USB MCU (e.g. Arduino Leonardo, Arduino Micro, etc.).
#endif // !defined(USBCON)
#endif // ARDUINO > 10606

#if !defined(_USING_DYNAMIC_HID)

#warning "Using legacy HID core (non pluggable)"

#else // !defined(_USING_DYNAMIC_HID)

//================================================================================
//  Joystick (Gamepad)

#define JOYSTICK_DEFAULT_REPORT_ID         0x01
#define JOYSTICK_DEFAULT_BUTTON_COUNT        32
#define JOYSTICK_DEFAULT_AXIS_MINIMUM         0
#define JOYSTICK_DEFAULT_AXIS_MAXIMUM      1023
#define JOYSTICK_DEFAULT_SIMULATOR_MINIMUM    0
#define JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM 1023
#define JOYSTICK_DEFAULT_HATSWITCH_COUNT      2
#define JOYSTICK_HATSWITCH_COUNT_MAXIMUM      2
#define JOYSTICK_HATSWITCH_RELEASE           -1
#define JOYSTICK_TYPE_JOYSTICK             0x04
#define JOYSTICK_TYPE_GAMEPAD              0x05
#define JOYSTICK_TYPE_MULTI_AXIS           0x08

#define FORCE_FEEDBACK_MAXGAIN              100
#define DEG_TO_RAD              ((float)((float)3.14159265359 / 180.0))

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

struct EffectParams{
    int32_t springMaxPosition = 0;
    int32_t springPosition = 0;

    int32_t damperMaxVelocity = 0;
    int32_t damperVelocity = 0;

    int32_t inertiaMaxAcceleration = 0;
    int32_t inertiaAcceleration = 0;

    int32_t frictionMaxPositionChange = 0;
    int32_t frictionPositionChange = 0;
};

class Joystick_
{
private:

    // Joystick State
	int16_t	                 _xAxis;
	int16_t	                 _yAxis;
	int16_t	                 _zAxis;
	int16_t	                 _xAxisRotation;
	int16_t	                 _yAxisRotation;
	int16_t	                 _zAxisRotation;
	int16_t                  _throttle;
	int16_t                  _rudder;
	int16_t	                 _hatSwitchValues[JOYSTICK_HATSWITCH_COUNT_MAXIMUM];
    uint8_t                 *_buttonValues = NULL;

    // Joystick Settings
    bool                     _autoSendState;
    uint8_t                  _buttonCount;
    uint8_t                  _buttonValuesArraySize = 0;
	uint8_t					 _hatSwitchCount;
	uint8_t					 _includeAxisFlags;
	uint8_t					 _includeSimulatorFlags;
	int16_t                  _xAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
	int16_t                  _xAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
	int16_t                  _yAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
	int16_t                  _yAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
	int16_t                  _zAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
	int16_t                  _zAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
	int16_t                  _rxAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
	int16_t                  _rxAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
	int16_t                  _ryAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
	int16_t                  _ryAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
	int16_t                  _rzAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
	int16_t                  _rzAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
	int16_t                  _rudderMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
	int16_t                  _rudderMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;
	int16_t                  _throttleMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
	int16_t                  _throttleMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;

	uint8_t                  _hidReportId;
	uint8_t                  _hidReportSize; 

	//force feedback effect params
	EffectParams* m_effect_params;

	///force calculate funtion
	float NormalizeRange(int32_t x, int32_t maxValue);
	int32_t ApplyEnvelope(volatile TEffectState& effect, int32_t value);
	int32_t ApplyGain(uint16_t value, uint8_t gain);
	int32_t ConstantForceCalculator(volatile TEffectState& effect);
	int32_t RampForceCalculator(volatile TEffectState& effect);
	int32_t SquareForceCalculator(volatile TEffectState& effect);
	int32_t SinForceCalculator(volatile TEffectState& effect);
	int32_t TriangleForceCalculator(volatile TEffectState& effect);
	int32_t SawtoothDownForceCalculator(volatile TEffectState& effect);
	int32_t SawtoothUpForceCalculator(volatile TEffectState& effect);
	int32_t ConditionForceCalculator(volatile TEffectState& effect, float metric, uint8_t conditionReport);
	void forceCalculator(int32_t* forces);
	float getAngleRatio(volatile TEffectState& effect, int axis);
	int32_t getEffectForce(volatile TEffectState& effect, EffectParams _effect_params, uint8_t axis);
protected:
	int buildAndSet16BitValue(bool includeValue, int16_t value, int16_t valueMinimum, int16_t valueMaximum, int16_t actualMinimum, int16_t actualMaximum, uint8_t dataLocation[]);
	int buildAndSetAxisValue(bool includeAxis, int16_t axisValue, int16_t axisMinimum, int16_t axisMaximum, uint8_t dataLocation[]);
	int buildAndSetSimulationValue(bool includeValue, int16_t value, int16_t valueMinimum, int16_t valueMaximum, uint8_t dataLocation[]);

public:
	Joystick_(
		uint8_t hidReportId = JOYSTICK_DEFAULT_REPORT_ID,
		uint8_t joystickType = JOYSTICK_TYPE_JOYSTICK,
        uint8_t buttonCount = JOYSTICK_DEFAULT_BUTTON_COUNT,
		uint8_t hatSwitchCount = JOYSTICK_DEFAULT_HATSWITCH_COUNT,
		bool includeXAxis = true,
		bool includeYAxis = true,
		bool includeZAxis = true,
		bool includeRxAxis = true,
		bool includeRyAxis = true,
		bool includeRzAxis = true,
		bool includeRudder = true,
		bool includeThrottle = true);

	void begin(bool initAutoSendState = true);
	void end();
	
	// Set Range Functions
	inline void setXAxisRange(int16_t minimum, int16_t maximum)
	{
		_xAxisMinimum = minimum;
		_xAxisMaximum = maximum;
	}
	inline void setYAxisRange(int16_t minimum, int16_t maximum)
	{
		_yAxisMinimum = minimum;
		_yAxisMaximum = maximum;
	}
	inline void setZAxisRange(int16_t minimum, int16_t maximum)
	{
		_zAxisMinimum = minimum;
		_zAxisMaximum = maximum;
	}
	inline void setRxAxisRange(int16_t minimum, int16_t maximum)
	{
		_rxAxisMinimum = minimum;
		_rxAxisMaximum = maximum;
	}
	inline void setRyAxisRange(int16_t minimum, int16_t maximum)
	{
		_ryAxisMinimum = minimum;
		_ryAxisMaximum = maximum;
	}
	inline void setRzAxisRange(int16_t minimum, int16_t maximum)
	{
		_rzAxisMinimum = minimum;
		_rzAxisMaximum = maximum;
	}
	inline void setRudderRange(int16_t minimum, int16_t maximum)
	{
		_rudderMinimum = minimum;
		_rudderMaximum = maximum;
	}
	inline void setThrottleRange(int16_t minimum, int16_t maximum)
	{
		_throttleMinimum = minimum;
		_throttleMaximum = maximum;
	}

	// Set Axis Values
	void setXAxis(int16_t value);
	void setYAxis(int16_t value);
	void setZAxis(int16_t value);
	void setRxAxis(int16_t value);
	void setRyAxis(int16_t value);
	void setRzAxis(int16_t value);

	// Set Simuation Values
	void setRudder(int16_t value);
	void setThrottle(int16_t value);

	void setButton(uint8_t button, uint8_t value);
	void pressButton(uint8_t button);
	void releaseButton(uint8_t button);
	void setHatSwitch(int8_t hatSwitch, int16_t value);

	void sendState();

	//force feedback Interfaces
	void getForce(int32_t* forces);
	//set effect params funtions
	int8_t setEffectParams(EffectParams* _effect_params){
	    if(_effect_params != nullptr){
	        m_effect_params = _effect_params;
	        return 0;
	    }
	    return -1;
	};
};

#endif // !defined(_USING_DYNAMIC_HID)
#endif // JOYSTICK_h
