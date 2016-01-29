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
#include "Application.h"

#undef DTAG
#define DTAG "[APP]\t"

#ifndef APP_DEBUG
#define APP_DEBUG 0
#endif


#if APP_DEBUG

#define __DPRINTF DPRINTF
#define __DPRINT DPRINT

#else

#define __DPRINTF(x, ...)
#define __DPRINT(x)

#endif


#define APPLICATION_VBUSGP_BUFFERSIZE 64

static RS_RINGBUFFER_DECLARATION(__VBusTx_Buffer, APPLICATION_VBUSGP_BUFFERSIZE);

static IPTR* const __UartTxBuffers [] = {
		__VBusTx_Buffer,
};


static const int __UartTxBufferSizes [] = {
	APPLICATION_VBUSGP_BUFFERSIZE,
};

static RS_PLATFORM_CONST UART_TYPE __UartTypes [] = {
	UART_TYPE_VBUS,
};


RS_PLATFORM_CONST INPUT_OUTPUT_TYPE kApplication_InputTypes [] = {
		INPUT_OUTPUT_TYPE_RESISTOR,  // Sensor1
		INPUT_OUTPUT_TYPE_RESISTOR,  // Sensor2
		INPUT_OUTPUT_TYPE_RESISTOR,  // Sensor3
		INPUT_OUTPUT_TYPE_RESISTOR,  // Sensor4
		INPUT_OUTPUT_TYPE_RESISTOR,  // Sensor5
		INPUT_OUTPUT_TYPE_RESISTOR,  // Sensor6
		INPUT_OUTPUT_TYPE_RESISTOR,  // Sensor7
		INPUT_OUTPUT_TYPE_RESISTOR,  // Sensor8
		INPUT_OUTPUT_TYPE_DIGITAL,   // DIP-Switch
};
RS_PLATFORM_CONST int kApplication_InputTypesCount = countof(kApplication_InputTypes);


I16 VBusAddress = VBUS_ADDRESS_BROADCAST;



void Application_SetValueById(RS_VALUEID ValueId, RS_VALUE Value)
{
	int index = 0;
	int typesCount = 0;

	if (ValueId < VALUEID_INPUT_BASE) {
		index = ValueId - VALUEID_OUTPUT_BASE - 1;
		typesCount = Platform_ReadConstInt(&kApplication_OutputTypesCount);
		if (index >= 0 && index < typesCount) {
			// TODO

		}
	} else {
		if (ValueId == VALUEID_INPUT_PRIORITY_CHANNEL) {
			Input_SetControlValue(0, 0, Value); // set ADC priority channel
			Input_GetControlValue(0, 0, &_Input_PriorityChannel); // sync value with input
		}
	}
}



RS_ERROR Application_Timer20msInterrupt(void)
{
	Application_Flags.Timer20msFlag = 1;

	return RS_OK;
}



RS_ERROR Application_Timer1sInterrupt(void)
{
	Application_Flags.Timer1sFlag = 1;

	return RS_OK;
}



RS_ERROR Application_Uart_DidReceive(int Index, int Received)
{
	Communication_DidReceive(Index, Received);
	return RS_OK;
}



void Application_Timer1sHandler(void)
{

}



RS_ERROR Application_LoopHandler(void)
{

	if (Application_Flags.Timer20msFlag) {
		Application_Flags.Timer20msFlag = 0;

		Communication_Timer20msHandler();
	}


	if (Application_Flags.Timer1sFlag) {
		Application_Flags.Timer1sFlag = 0;

		Application_Timer1sHandler();
		Communication_StartCycle();
	}

	Input_GetInputValueForType(0, INPUT_OUTPUT_TYPE_RESISTOR, &_Input_Sensor1);
	Input_GetInputValueForType(1, INPUT_OUTPUT_TYPE_RESISTOR, &_Input_Sensor2);
	Input_GetInputValueForType(2, INPUT_OUTPUT_TYPE_RESISTOR, &_Input_Sensor3);
	Input_GetInputValueForType(3, INPUT_OUTPUT_TYPE_RESISTOR, &_Input_Sensor4);
	Input_GetInputValueForType(4, INPUT_OUTPUT_TYPE_RESISTOR, &_Input_Sensor5);
	Input_GetInputValueForType(5, INPUT_OUTPUT_TYPE_RESISTOR, &_Input_Sensor6);
	Input_GetInputValueForType(6, INPUT_OUTPUT_TYPE_RESISTOR, &_Input_Sensor7);
	Input_GetInputValueForType(7, INPUT_OUTPUT_TYPE_RESISTOR, &_Input_Sensor8);
	Input_GetInputValueForType(8, INPUT_OUTPUT_TYPE_DIGITAL, &_Input_DipSwitch);
	VBusAddress = (VBUS_ADDRESS_BROADCAST + (_Input_DipSwitch & 0x0F));

	Communication_LoopHandler();

	return RS_OK;
}



RS_ERROR Application_Initialize(void)
{
	RS_ERROR Error;

	Error = RS_OK;

	if (Error >= RS_OK) {
		int i;
		RS_RINGBUFFER_STATE *Uart_Buffers[countof(__UartTypes)];

		for (i = 0; i < countof(__UartTypes); i++) {
			Uart_Buffers[i] = Rs_RingBuffer_Initialize(__UartTxBuffers[i], __UartTxBufferSizes[i]);
		}
		Error = Uart_Initialize(countof(__UartTypes), __UartTypes, Uart_Buffers);
		_Output_Baudrate = 9600;
#if DEBUG
		_Output_Baudrate = 57600UL;
		Uart_SetBaud(0, 57600UL);
#endif
		__DPRINTF(DTAG "Uart_Initialized() returned %d\n", Error);
	}
	__DPRINTF("\n\n\n\n****Arduino Pt1000 %s-%s Booting ****\n\n\n", VERSION, BUILDVER);

	if (Error >= RS_OK) {
		Error = Input_Initialize(countof (kApplication_InputTypes), kApplication_InputTypes);
		Input_GetControlValue(0, 0, &_Input_PriorityChannel); // get default ADC priority channel
		__DPRINTF(DTAG "Input_Initialized() returned %d\n", Error);
	}

	return Error;
}



RS_ERROR Application_RunMain()
{

	RS_ERROR Error;

	Error = Platform_Initialize();

	Error = Communication_Initialize();

	if (Error >= RS_OK) {
		__DPRINTF(DTAG "Calling Platform_RunLoop()...\n");
		Error = Platform_RunLoop();
		__DPRINTF("\tDONE\n");
	}

	return RS_OK;
}

#undef APP_DEBUG
