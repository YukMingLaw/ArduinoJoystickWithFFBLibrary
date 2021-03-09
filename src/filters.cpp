#include "filters.h"
#include <math.h>

#define PI 3.1415926535897932384626433832795

LowPassFilter::LowPassFilter():
	output(0),
	ePow(0){}

LowPassFilter::LowPassFilter(float iCutOffFrequency, float iDeltaTime):
	output(0),
	ePow(1-exp(-iDeltaTime * 2 * PI * iCutOffFrequency))
{
}

float LowPassFilter::update(float input){
	return output += (input - output) * ePow;
}

float LowPassFilter::update(float input, float deltaTime, float cutoffFrequency){
	reconfigureFilter(deltaTime, cutoffFrequency); //Changes ePow accordingly.
	return output += (input - output) * ePow;
}

void LowPassFilter::reconfigureFilter(float deltaTime, float cutoffFrequency){
	ePow = 1-exp(-deltaTime * 2 * PI * cutoffFrequency);
}
