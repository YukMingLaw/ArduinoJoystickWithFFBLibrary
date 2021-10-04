#include "PIDReportHandler.h"

PIDReportHandler::PIDReportHandler() 
{
	nextEID = 1;
	devicePaused = 0;
}

PIDReportHandler::~PIDReportHandler() 
{
	FreeAllEffects();
}

uint8_t PIDReportHandler::GetNextFreeEffect(void)
{
	if (nextEID == MAX_EFFECTS)
		return 0;

	uint8_t id = nextEID++;

	while (g_EffectStates[nextEID].state != 0)
	{
		if (nextEID >= MAX_EFFECTS)
			break;  // the last spot was taken
		nextEID++;
	}

	g_EffectStates[id].state = MEFFECTSTATE_ALLOCATED;
    pidState.effectBlockIndex = id;

	return id;
}

void PIDReportHandler::StopAllEffects(void)
{
	for (uint8_t id = 0; id <= MAX_EFFECTS; id++)
		StopEffect(id);
}

void PIDReportHandler::StartEffect(uint8_t id)
{
	if (id > MAX_EFFECTS)
		return;
	g_EffectStates[id].state = MEFFECTSTATE_PLAYING;
	g_EffectStates[id].elapsedTime = 0;
	g_EffectStates[id].startTime = (uint64_t)millis();
}

void PIDReportHandler::StopEffect(uint8_t id)
{
	if (id > MAX_EFFECTS)
		return;
	g_EffectStates[id].state &= ~MEFFECTSTATE_PLAYING;
	pidBlockLoad.ramPoolAvailable += SIZE_EFFECT;
}

void PIDReportHandler::FreeEffect(uint8_t id)
{
	if (id > MAX_EFFECTS)
		return;
	g_EffectStates[id].state = 0;
	if (id < nextEID)
		nextEID = id;
}

void PIDReportHandler::FreeAllEffects(void)
{
	nextEID = 1;
	memset((void*)& g_EffectStates, 0, sizeof(g_EffectStates));
	pidBlockLoad.ramPoolAvailable = MEMORY_SIZE;

    USB_FFBReport_SetEffect_Output_Data_t springEffect;
	memset((void*)& springEffect, 0, sizeof(springEffect));
    springEffect.reportId = 1;
    springEffect.effectBlockIndex = 0;
    springEffect.effectType = 0x08; // TODO: include from PIDReportType
    springEffect.duration = 0xFF;
    springEffect.gain = 255;
    springEffect.enableAxis = 0x01; // TODO: include from PIDReportType
    SetEffect(&springEffect);

    USB_FFBReport_SetCondition_Output_Data_t springCondition;
	memset((void*)& springCondition, 0, sizeof(springCondition));
    springCondition.reportId = 3;
    springCondition.effectBlockIndex = 0;
    springCondition.cpOffset = 0;
    springCondition.positiveCoefficient = 10000;
    springCondition.negativeCoefficient = 10000;
    springCondition.positiveSaturation = 10000;
    springCondition.negativeSaturation = 10000;
    springCondition.deadBand = 0;


    for (int i=0; i<FFB_AXIS_COUNT; ++i)
    {
        springCondition.parameterBlockOffset = i;
        SetCondition(&springCondition, &g_EffectStates[0]);
    }
}

void PIDReportHandler::EffectOperation(USB_FFBReport_EffectOperation_Output_Data_t* data)
{
    //Serial.print("lC");
    //Serial.println(data->loopCount);
    g_EffectStates[data->effectBlockIndex].loopCount = data->loopCount;
	if (data->operation == 1)
	{ // Start
		if (data->loopCount == 0xFF)
        {
            g_EffectStates[data->effectBlockIndex].totalDuration = USB_DURATION_INFINITE;
		}
        else if (data->loopCount > 0)
        {
            g_EffectStates[data->effectBlockIndex].totalDuration = (
                g_EffectStates[data->effectBlockIndex].startDelay
                + g_EffectStates[data->effectBlockIndex].duration
              ) * data->loopCount;
        }
		StartEffect(data->effectBlockIndex);
	}
	else if (data->operation == 2)
	{ // StartSolo

	  // Stop all first
		StopAllEffects();
		// Then start the given effect
		StartEffect(data->effectBlockIndex);
	}
	else if (data->operation == 3)
	{ // Stop
		StopEffect(data->effectBlockIndex);
	}
	else
	{
	}
}

void PIDReportHandler::BlockFree(USB_FFBReport_BlockFree_Output_Data_t* data)
{
	uint8_t eid = data->effectBlockIndex;

	if (eid == 0xFF)
	{ // all effects
		FreeAllEffects();
	}
	else
	{
		FreeEffect(eid);
	}
}

void PIDReportHandler::DeviceControl(USB_FFBReport_DeviceControl_Output_Data_t* data)
{

	uint8_t control = data->control;

	if (control == 0x01)
	{ // 1=Enable Actuators
		pidState.status |= 2;
	}
	else if (control == 0x02)
	{ // 2=Disable Actuators
		pidState.status &= ~(0x02);
	}
	else if (control == 0x03)
	{ // 3=Stop All Effects
		StopAllEffects();
	}
	else if (control == 0x04)
	{ //  4=Reset
		FreeAllEffects();
	}
	else if (control == 0x05)
	{ // 5=Pause
		devicePaused = 1;
	}
	else if (control == 0x06)
	{ // 6=Continue
		devicePaused = 0;
	}
	else if (control & (0xFF - 0x3F))
	{
	}
}

void PIDReportHandler::DeviceGain(USB_FFBReport_DeviceGain_Output_Data_t* data)
{
	deviceGain.gain = data->gain;
}

void PIDReportHandler::SetCustomForce(USB_FFBReport_SetCustomForce_Output_Data_t* data)
{
}

void PIDReportHandler::SetCustomForceData(USB_FFBReport_SetCustomForceData_Output_Data_t* data)
{
}

void PIDReportHandler::SetDownloadForceSample(USB_FFBReport_SetDownloadForceSample_Output_Data_t* data)
{
}

void PIDReportHandler::SetEffect(USB_FFBReport_SetEffect_Output_Data_t* data)
{
	volatile TEffectState* effect = &g_EffectStates[data->effectBlockIndex];

	effect->duration = data->duration;
    for (int i=0; i<FFB_AXIS_COUNT; ++i)
    {
        effect->direction[i] = data->direction[i];
    }
	effect->effectType = data->effectType;
	effect->gain = data->gain;
	effect->enableAxis = data->enableAxis;
    effect->startDelay = data->startDelay;
    
    // we can receive new length of effect while the effect is already looping
    // recalculate durations as (duration+delay)*loopCount unless the loopCount is infinite
    if (effect->loopCount != 0xFF)
    {
        uint8_t loopCount = effect->loopCount > 0 ? effect->loopCount : 1;
        effect->totalDuration = (data->duration + data->startDelay) * loopCount;
    }
	//Serial.print("lC: ");
	//Serial.println(effect->loopCount);
	//Serial.print("d: ");
	//Serial.print(effect->duration);
	//Serial.print("tD: ");
	//Serial.println(effect->totalDuration);
	//Serial.print("sD: ");
	//Serial.println(effect->startDelay);
	//Serial.print("dX: ");
	//Serial.print(effect->directionX);
	//Serial.print(" dX: ");
	//Serial.print(effect->directionY);
	//Serial.print(" eT: ");
	//Serial.print(effect->effectType);
	//Serial.print(" eA: ");
	//Serial.println(effect->enableAxis);
	//  effect->triggerRepeatInterval;
	//  effect->samplePeriod;   // 0..32767 ms
	//  effect->triggerButton;
}

void PIDReportHandler::SetEnvelope(USB_FFBReport_SetEnvelope_Output_Data_t* data, volatile TEffectState* effect)
{
	effect->attackLevel = data->attackLevel;
	effect->fadeLevel = data->fadeLevel;
	effect->attackTime = data->attackTime;
	effect->fadeTime = data->fadeTime;
}

void PIDReportHandler::SetCondition(USB_FFBReport_SetCondition_Output_Data_t* data, volatile TEffectState* effect)
{
        uint8_t axis = data->parameterBlockOffset; 
        if (axis >= effect->conditionReportsCount)
        {
            effect->conditionReportsCount = axis + 1;
        }

        effect->conditions[axis].cpOffset = data->cpOffset;
        effect->conditions[axis].positiveCoefficient = data->positiveCoefficient;
        effect->conditions[axis].negativeCoefficient = data->negativeCoefficient;
        effect->conditions[axis].positiveSaturation = data->positiveSaturation;
        effect->conditions[axis].negativeSaturation = data->negativeSaturation;
        effect->conditions[axis].deadBand = data->deadBand;
}

void PIDReportHandler::SetPeriodic(USB_FFBReport_SetPeriodic_Output_Data_t* data, volatile TEffectState* effect)
{
	effect->magnitude = data->magnitude;
	effect->offset = data->offset;
	effect->phase = data->phase;
	effect->period = data->period;
}

void PIDReportHandler::SetConstantForce(USB_FFBReport_SetConstantForce_Output_Data_t* data, volatile TEffectState* effect)
{
	//  ReportPrint(*effect);
	effect->magnitude = data->magnitude;
}

void PIDReportHandler::SetRampForce(USB_FFBReport_SetRampForce_Output_Data_t* data, volatile TEffectState* effect)
{
	effect->startMagnitude = data->startMagnitude;
	effect->endMagnitude = data->endMagnitude;
}

void PIDReportHandler::CreateNewEffect(USB_FFBReport_CreateNewEffect_Feature_Data_t* inData)
{
	pidBlockLoad.reportId = 6;
	pidBlockLoad.effectBlockIndex = GetNextFreeEffect();

	if (pidBlockLoad.effectBlockIndex == 0)
	{
		pidBlockLoad.loadStatus = 2;    // 1=Success,2=Full,3=Error
	}
	else
	{
		pidBlockLoad.loadStatus = 1;    // 1=Success,2=Full,3=Error

		volatile TEffectState* effect = &g_EffectStates[pidBlockLoad.effectBlockIndex];

		memset((void*)effect, 0, sizeof(TEffectState));
		effect->state = MEFFECTSTATE_ALLOCATED;
		pidBlockLoad.ramPoolAvailable -= SIZE_EFFECT;
	}
}

void PIDReportHandler::UppackUsbData(uint8_t* data, uint16_t len)
{
	//Serial.print("len:");
	//Serial.println(len);
    //for (uint16_t i=0; i<len; ++i) {
    //  if (data[i] < 0xA0) {
    //    Serial.print(" ");
    //  }
    //  Serial.print(data[i], HEX);
    //}
    //Serial.println("");
	uint8_t effectId = data[1]; // effectBlockIndex is always the second byte.
	switch (data[0])    // reportID
	{
	case 1:
		//Serial.println("SetEffect");
		SetEffect((USB_FFBReport_SetEffect_Output_Data_t*)data);
		break;
	case 2:
		//Serial.println("SetEnvelop");
		SetEnvelope((USB_FFBReport_SetEnvelope_Output_Data_t*)data, &g_EffectStates[effectId]);
		break;
	case 3:
		//Serial.println("SetCondition");
		SetCondition((USB_FFBReport_SetCondition_Output_Data_t*)data, &g_EffectStates[effectId]);
		break;
	case 4:
		//Serial.println("SetPeriodic");
		SetPeriodic((USB_FFBReport_SetPeriodic_Output_Data_t*)data, &g_EffectStates[effectId]);
		break;
	case 5:
		//Serial.println("SetConstantForce");
		SetConstantForce((USB_FFBReport_SetConstantForce_Output_Data_t*)data, &g_EffectStates[effectId]);
		break;
	case 6:
		//Serial.println("SetRampForce");
		SetRampForce((USB_FFBReport_SetRampForce_Output_Data_t*)data, &g_EffectStates[effectId]);
		break;
	case 7:
		//Serial.println("SetCustomForceData");
		SetCustomForceData((USB_FFBReport_SetCustomForceData_Output_Data_t*)data);
		break;
	case 8:
		//Serial.println("SetDownloadForceSample");
		SetDownloadForceSample((USB_FFBReport_SetDownloadForceSample_Output_Data_t*)data);
		break;
	case 9:
		break;
	case 10:
		//Serial.println("EffectOperation");
		EffectOperation((USB_FFBReport_EffectOperation_Output_Data_t*)data);
		break;
	case 11:
		//Serial.println("BlockFree");
		BlockFree((USB_FFBReport_BlockFree_Output_Data_t*)data);
		break;
	case 12:
		//Serial.println("DeviceControl");
		DeviceControl((USB_FFBReport_DeviceControl_Output_Data_t*)data);
		break;
	case 13:
		//Serial.println("DeviceGain");
		DeviceGain((USB_FFBReport_DeviceGain_Output_Data_t*)data);
		break;
	case 14:
		//Serial.println("SetCustomForce");
		SetCustomForce((USB_FFBReport_SetCustomForce_Output_Data_t*)data);
		break;
	default:
		break;
	}
}

uint8_t* PIDReportHandler::getPIDPool()
{
	FreeAllEffects();

	pidPoolReport.reportId = 7;
	pidPoolReport.ramPoolSize = MEMORY_SIZE;
	pidPoolReport.maxSimultaneousEffects = MAX_EFFECTS;
	pidPoolReport.memoryManagement = 3;
	return (uint8_t*)& pidPoolReport;
}

uint8_t* PIDReportHandler::getPIDBlockLoad()
{
	return (uint8_t*)& pidBlockLoad;
}

uint8_t* PIDReportHandler::getPIDStatus()
{
	return (uint8_t*)& pidState;
}
