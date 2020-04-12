#ifndef _PIDREPORTHANDLER_H
#define _PIDREPORTHANDLER_H
#include <Arduino.h>
#include "PIDReportType.h"

class PIDReportHandler {
public:
	PIDReportHandler();
	~PIDReportHandler();
	// Effect management
	volatile uint8_t nextEID;  // FFP effect indexes starts from 1
	volatile TEffectState  g_EffectStates[MAX_EFFECTS + 1];
	volatile uint8_t devicePaused;
	//variables for storing previous values
	volatile int32_t inertiaT = 0;
	volatile int16_t oldSpeed = 0;
	volatile int16_t oldAxisPosition = 0;
	volatile USB_FFBReport_PIDStatus_Input_Data_t pidState = { 2, 30, 0 };
	volatile USB_FFBReport_PIDBlockLoad_Feature_Data_t pidBlockLoad;
	volatile USB_FFBReport_PIDPool_Feature_Data_t pidPoolReport;
	volatile USB_FFBReport_DeviceGain_Output_Data_t deviceGain;

	//ffb state structures
	uint8_t GetNextFreeEffect(void);
	void StartEffect(uint8_t id);
	void StopEffect(uint8_t id);
	void StopAllEffects(void);
	void FreeEffect(uint8_t id);
	void FreeAllEffects(void);

	//handle output pid report
	void EffectOperation(USB_FFBReport_EffectOperation_Output_Data_t* data);
	void BlockFree(USB_FFBReport_BlockFree_Output_Data_t* data);
	void DeviceControl(USB_FFBReport_DeviceControl_Output_Data_t* data);
	void DeviceGain(USB_FFBReport_DeviceGain_Output_Data_t* data);
	void SetCustomForceData(USB_FFBReport_SetCustomForceData_Output_Data_t* data);
	void SetDownloadForceSample(USB_FFBReport_SetDownloadForceSample_Output_Data_t* data);
	void SetCustomForce(USB_FFBReport_SetCustomForce_Output_Data_t* data);
	void SetEffect(USB_FFBReport_SetEffect_Output_Data_t* data);
	void SetEnvelope(USB_FFBReport_SetEnvelope_Output_Data_t* data, volatile TEffectState* effect);
	void SetCondition(USB_FFBReport_SetCondition_Output_Data_t* data, volatile TEffectState* effect);
	void SetPeriodic(USB_FFBReport_SetPeriodic_Output_Data_t* data, volatile TEffectState* effect);
	void SetConstantForce(USB_FFBReport_SetConstantForce_Output_Data_t* data, volatile TEffectState* effect);
	void SetRampForce(USB_FFBReport_SetRampForce_Output_Data_t* data, volatile TEffectState* effect);

	// Handle incoming data from USB
	void CreateNewEffect(USB_FFBReport_CreateNewEffect_Feature_Data_t* inData);
	void UppackUsbData(uint8_t* data, uint16_t len);
	uint8_t* getPIDPool();
	uint8_t* getPIDBlockLoad();
	uint8_t* getPIDStatus();
};
#endif
