/*
  Modified by Matthew Heironimus to support HID Report Descriptors to be in 
  standard RAM in addition to program memory (PROGMEM).

   Copyright (c) 2015, Arduino LLC
   Original code (pre-library): Copyright (c) 2011, Peter Barrett

   Permission to use, copy, modify, and/or distribute this software for
   any purpose with or without fee is hereby granted, provided that the
   above copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
   BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
   OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
   ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
   SOFTWARE.
 */

#include "DynamicHID.h"

#if defined(USBCON)

#ifdef _VARIANT_ARDUINO_DUE_X_
#define USB_SendControl USBD_SendControl
#define USB_Send USBD_Send
#endif

DynamicHID_& DynamicHID()
{
	static DynamicHID_ obj;
	return obj;
}

int DynamicHID_::getInterface(uint8_t* interfaceCount)
{
	*interfaceCount += 1; // uses 1
	DYNAMIC_HIDDescriptor hidInterface = {
		D_INTERFACE(pluggedInterface, PID_ENPOINT_COUNT, USB_DEVICE_CLASS_HUMAN_INTERFACE, DYNAMIC_HID_SUBCLASS_NONE, DYNAMIC_HID_PROTOCOL_NONE),
		D_HIDREPORT(descriptorSize),
		D_ENDPOINT(USB_ENDPOINT_IN(PID_ENDPOINT_IN), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01),
		D_ENDPOINT(USB_ENDPOINT_OUT(PID_ENDPOINT_OUT), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
	};
	return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}

int DynamicHID_::getDescriptor(USBSetup& setup)
{
	// Check if this is a HID Class Descriptor request
	if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }
	if (setup.wValueH != DYNAMIC_HID_REPORT_DESCRIPTOR_TYPE) { return 0; }
	// In a HID Class Descriptor wIndex cointains the interface number
	if (setup.wIndex != pluggedInterface) { return 0; }

	int total = 0;
	DynamicHIDSubDescriptor* node;
	for (node = rootNode; node; node = node->next) {
		int res = USB_SendControl(0, node->data, node->length);
		if (res == -1)
			return -1;
		total += res;
		res = USB_SendControl(TRANSFER_PGM, node->pid_data, node->pid_length);
		if (res == -1)
			return -1;
		total += res;
	}
	
	// Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
	// due to the USB specs, but Windows and Linux just assumes its in report mode.
	protocol = DYNAMIC_HID_REPORT_PROTOCOL;
	
	return total;
}

uint8_t DynamicHID_::getShortName(char *name)
{
	name[0] = 'H';
	name[1] = 'I';
	name[2] = 'D';
	name[3] = 'A' + (descriptorSize & 0x0F);
	name[4] = 'A' + ((descriptorSize >> 4) & 0x0F);
	return 5;
}

void DynamicHID_::AppendDescriptor(DynamicHIDSubDescriptor *node)
{
	if (!rootNode) {
		rootNode = node;
	} else {
		DynamicHIDSubDescriptor *current = rootNode;
		while (current->next) {
			current = current->next;
		}
		current->next = node;
	}
	descriptorSize += node->length;
	descriptorSize += node->pid_length;
}

int DynamicHID_::SendReport(uint8_t id, const void* data, int len)
{
	uint8_t p[len + 1];
	p[0] = id;
	memcpy(&p[1], data, len);
	return USB_Send(PID_ENDPOINT_IN | TRANSFER_RELEASE, p, len + 1);
}

int DynamicHID_::RecvData(byte* data)
{
	int count = 0;
	while (usb_Available()) {
		data[count++] = (byte)USB_Recv(PID_ENDPOINT_OUT);
	}
	return count;
}

void DynamicHID_::RecvfromUsb() 
{
	if (usb_Available() > 0) {
		uint16_t len = USB_Recv(PID_ENDPOINT_OUT, &out_ffbdata, 64);
		if (len >= 0) {
			pidReportHandler.UppackUsbData(out_ffbdata, len);
		}
	}
}

bool DynamicHID_::GetReport(USBSetup& setup) {
	uint8_t report_id = setup.wValueL;
	uint8_t report_type = setup.wValueH;
	if (report_type == DYNAMIC_HID_REPORT_TYPE_INPUT)
	{
		//        /* Create the next HID report to send to the host */
		//        GetNextReport(0xFF, &JoystickReportData);
		//        /* Write the report data to the control endpoint */
		//        USB_SendControl(TRANSFER_RELEASE, &JoystickReportData, sizeof(JoystickReportData));
	}
	if (report_type == DYNAMIC_HID_REPORT_TYPE_OUTPUT) {}
	if (report_type == DYNAMIC_HID_REPORT_TYPE_FEATURE) {
		if ((report_id == 6))// && (gNewEffectBlockLoad.reportId==6))
		{
			_delay_us(500);
			USB_SendControl(TRANSFER_RELEASE, pidReportHandler.getPIDBlockLoad(), sizeof(USB_FFBReport_PIDBlockLoad_Feature_Data_t));
			pidReportHandler.pidBlockLoad.reportId = 0;
			return (true);
		}
		if (report_id == 7)
		{
			USB_FFBReport_PIDPool_Feature_Data_t ans;
			ans.reportId = report_id;
			ans.ramPoolSize = 0xffff;
			ans.maxSimultaneousEffects = MAX_EFFECTS;
			ans.memoryManagement = 3;
			USB_SendControl(TRANSFER_RELEASE, &ans, sizeof(USB_FFBReport_PIDPool_Feature_Data_t));
			return (true);
		}
	}
	return (false);
}

bool DynamicHID_::SetReport(USBSetup& setup) {
	uint8_t report_id = setup.wValueL;
	uint8_t report_type = setup.wValueH;
	uint16_t length = setup.wLength;
	uint8_t data[10];
	if (report_type == DYNAMIC_HID_REPORT_TYPE_FEATURE) {
		if (length == 0)
		{
			USB_RecvControl(&data, length);
			// Block until data is read (make length negative)
			//disableFeatureReport();
			return true;
		}
		if (report_id == 5)
		{
			USB_FFBReport_CreateNewEffect_Feature_Data_t ans;
			USB_RecvControl(&ans, sizeof(USB_FFBReport_CreateNewEffect_Feature_Data_t));
			pidReportHandler.CreateNewEffect(&ans);
		}
		return (true);
	}
	if (setup.wValueH == DYNAMIC_HID_REPORT_TYPE_INPUT)
	{
		/*if(length == sizeof(JoystickReportData))
		  {
		  USB_RecvControl(&JoystickReportData, length);
		  return true;
		  }*/
	}
}

bool DynamicHID_::setup(USBSetup& setup)
{
	if (pluggedInterface != setup.wIndex) {
		return false;
	}

	uint8_t request = setup.bRequest;
	uint8_t requestType = setup.bmRequestType;

	if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
	{
		if (request == DYNAMIC_HID_GET_REPORT) {
			// TODO: DYNAMIC_HID_GetReport();
			GetReport(setup);
			return true;
		}
		if (request == DYNAMIC_HID_GET_PROTOCOL) {
			// TODO: Send8(protocol);
			return true;
		}
		if (request == DYNAMIC_HID_GET_IDLE) {
			// TODO: Send8(idle);
		}
	}

	if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE)
	{
		if (request == DYNAMIC_HID_SET_PROTOCOL) {
			// The USB Host tells us if we are in boot or report mode.
			// This only works with a real boot compatible device.
			protocol = setup.wValueL;
			return true;
		}
		if (request == DYNAMIC_HID_SET_IDLE) {
			idle = setup.wValueL;
			return true;
		}
		if (request == DYNAMIC_HID_SET_REPORT)
		{
			SetReport(setup);
			return true;
		}
	}
	return false;
}

DynamicHID_::DynamicHID_(void) : PluggableUSBModule(PID_ENPOINT_COUNT, 1, epType),
                   rootNode(NULL), descriptorSize(0),
                   protocol(DYNAMIC_HID_REPORT_PROTOCOL), idle(1)
{
	epType[0] = EP_TYPE_INTERRUPT_IN;
	epType[1] = EP_TYPE_INTERRUPT_OUT;
	PluggableUSB().plug(this);
}

int DynamicHID_::begin(void)
{
	return 0;
}

bool DynamicHID_::usb_Available() {
	return USB_Available(PID_ENDPOINT_OUT);
}

#endif /* if defined(USBCON) */
