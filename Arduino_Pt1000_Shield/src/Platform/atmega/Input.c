/**
 * Copyright (C) 2014-2016, RESOL - Elektronische Regelungen GmbH.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "Input.h"

#undef DTAG
#define DTAG "<IN>\t"

#ifndef INPUT_DEBUG
#define INPUT_DEBUG 0
#endif

#if INPUT_DEBUG

#define __DPRINTF DPRINTF
#define __DPRINT DPRINT

#else

#define __DPRINTF(x, ...)
#define __DPRINT(x)

#endif

#define INPUT_VALUES_COUNT 8

I32 AdcAD7790_ChannelValues [INPUT_VALUES_COUNT];
RS_PLATFORM_CONST UINT AdcAD7790_ChannelCount = INPUT_VALUES_COUNT;

static struct {
	U8 ResetRequest : 1;
	U8 Error : 1;
	int PriorityChannel;
} __AdcInfo = {0};


static I32 __InputValues [INPUT_VALUES_COUNT + 1];


#if INPUT_DEBUG

static volatile U8 __DipDebug = 0;

static void __PrintValues(void)
{
	int i;

	if (__AdcAD7790_State.CurrentChannel < (INPUT_VALUES_COUNT - 1)) {
		return;
	}

	DPRINT("\n" DTAG "ADC_MeasComp:\n");
	for (i = 0; i < INPUT_VALUES_COUNT; i++) {
		DPRINTF("\t[%d]:\t Raw: 0x%lX", i, AdcAD7790_ChannelValues[i]);
		DPRINTF(" (%lu)\t", AdcAD7790_ChannelValues[i]);
		DPRINTF("Resistor: %ld", __InputValues [i]);
		DPRINTF(" (0x%lX).\t", __InputValues [i]);
		DPRINT("\n");
	}
	DPRINTF("\t[%d]:\tDIP-Switch: 0x%lX (PinD 0x%X)\n", INPUT_VALUES_COUNT, __InputValues[INPUT_VALUES_COUNT], __DipDebug);
	DPRINT("\n\n");
}
#else

#define __PrintValues()

#endif //DEBUGMODE


void AdcAD7790_Reset()
{
	AdcAD7790_Enable(1);
	AdcAD7790_Transceive(0xFF, 0);
	AdcAD7790_Transceive(0xFF, 0);
	AdcAD7790_Transceive(0xFF, 0);
	AdcAD7790_Transceive(0xFF, 0);
	AdcAD7790_Enable(0);
}



void AdcAD7790_HardwareInitialize(void)
{
	// nop, already init in PlatformInitialize()
}



U32 AdcAD7790_SetMuxChannel(int Channel)
{
	U8 Port, Ch;

	Ch = (U8) (Channel & 0xFF);

	Port = PINB;
	Port &= 0xF0;
	Port |= (Ch & 0x07);

	PORTB = Port;

	return 20000; // 20 ms
}



void AdcAD7790_Enable(int Enable)
{
	// Phase 4 == Adc timeout, reset sequence tx
	if (ADCAD7790_STATE_(Phase) == 4) {
		__AdcInfo.Error = 1;
	}

	if (Enable) {
		SPI_ADC_ENABLE();
	} else {
		SPI_ADC_DISABLE();
	}
}



int AdcAD7790_Transceive(U8 Output, int CheckOnly)
{
	int Result;

	if (CheckOnly) {
		Result = (SPI_ADC_READY) ? -1 : 0;
	} else {
		Result = Spi_Transceive(Output) & 0xFF;
	}

	return Result;
}



static double __ConvertAdcDataToResistor(U32 Data, double RefResistor)
{
	double f, Resistor, dData;

	dData = (double) Data;

	f = dData / 32768.0 - 1.0;  // Verhältnis Spannungsteiler (Rin / (Rin + Rref))
	if (f <= 0.001) {
		Resistor = 0.0;
	} else if (f >= 0.999) {
		Resistor = RefResistor * 2000.0;
	} else {
		Resistor = RefResistor / (1.0 / f - 1.0);
	}

	return Resistor;
}



void AdcAD7790_MeasurementComplete(int Channel)
{
	double Resistor;

	if ((Channel < 0) || (Channel >= INPUT_VALUES_COUNT)) {
		__DPRINTF(DTAG "ERROR: Invalid channel in AdcMeasComp.! (ch. %d)\n", Channel);
		return;
	}

	// 8x PT1000
	Resistor = __ConvertAdcDataToResistor(AdcAD7790_ChannelValues [Channel], 2000.0);
	__InputValues [Channel] = (I32) (Resistor * 1000.0);


	// toggle channel: prio, normal[i], prio, normal[i + 1], prio, ....
	if (Channel == __AdcInfo.PriorityChannel) {
		if (__AdcAD7790_State.NextChannel == Channel) {
			if (++__AdcAD7790_State.NextChannel >= INPUT_VALUES_COUNT) {
				__AdcAD7790_State.NextChannel = 0;
			}
		}
		__DPRINTF(DTAG "PrioCh. (%d) done. Change to next (%d).\n", Channel, __AdcAD7790_State.NextChannel);
	} else {
		__AdcAD7790_State.PriorityChannel = __AdcInfo.PriorityChannel;
	}

	if (__AdcInfo.ResetRequest) {
		AdcAD7790_Reset();
		__AdcInfo.ResetRequest = 0;
	}

	__PrintValues();
}



void Input_Timer20msHandler(void)
{
	// get dip switch value
	U8 PinD = PIND;

	__InputValues[8] = ((~PinD >> 2) & 0x0F);
#if INPUT_DEBUG
	__DipDebug = PinD;
#endif
}



void Input_LoopHandler(void)
{
	AdcAD7790_LoopHandler();
}



RS_ERROR Input_GetControlValue(int Index, int Control, I32* pValue)
{
	if (pValue == 0) {
		return RS_ERROR_NULLPOINTER;
	} else if (Index == 0) {
		*pValue = __AdcInfo.PriorityChannel;
		return RS_OK;
	} else {
		return RS_ERROR_INDEXOUTOFBOUNDS;
	}
}



RS_ERROR Input_SetControlValue(int Index, int Control, I32 Value)
{
	if (Index == 0) {
		__AdcInfo.PriorityChannel = (int) Value;
		if (__AdcInfo.PriorityChannel >= INPUT_VALUES_COUNT) {
			__AdcInfo.PriorityChannel = -1;
		}
		return RS_OK;
	} else {
		return RS_ERROR_INDEXOUTOFBOUNDS;
	}
}



RS_ERROR Input_GetInputValueForType(int Index, INPUT_OUTPUT_TYPE Type, I32* pValue)
{
	RS_ERROR result = RS_OK;
	int valueIdsCount = 0;

	valueIdsCount = Platform_ReadConstInt(&kInputValueIdsCount);

	//__DPRINTF(DTAG "GetInput(%d)", Index);

	if (!pValue) {
		result = RS_ERROR_NULLPOINTER;
		__DPRINTF(DTAG "%s: NULL pointer.\n", __func__);
	} else if (Index >= 0 && Index < valueIdsCount) {
		if (Type == INPUT_OUTPUT_TYPE_RESISTOR) {
			if (Index < INPUT_VALUES_COUNT) {
				*pValue = __InputValues[Index];
			} else {
				result = RS_ERROR_INDEXOUTOFBOUNDS;
			}
		} else if (Type == INPUT_OUTPUT_TYPE_DIGITAL) {
			if (Index == INPUT_VALUES_COUNT) {
				*pValue = __InputValues[Index];
			} else {
				result = RS_ERROR_INDEXOUTOFBOUNDS;
			}
		} else {
			result = RS_ERROR_INVALIDARGUMENT;
		}
	} else {
		__DPRINTF(DTAG "%s: Index out of bound.\n", __func__);
		result = RS_ERROR_INDEXOUTOFBOUNDS;
	}

	return result;
}



RS_ERROR Input_Initialize(int Count, const INPUT_OUTPUT_TYPE* Types)
{
	RS_ERROR result = RS_OK;

	AdcAD7790_Initialize();
	__AdcInfo.PriorityChannel = -1;

	return result;
}

#undef INPUT_DEBUG
