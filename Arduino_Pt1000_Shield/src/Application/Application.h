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
#ifndef __ARDUINO_PT1000_SHIELD_APPLICATION_H_
#define __ARDUINO_PT1000_SHIELD_APPLICATION_H_


#include "PlatformApi.h"

#include "Console.h"
#include "Values.h"
#include "RsRingBuffer.h"

//---------------------------------------------------------------------------
// APPLICATION
//---------------------------------------------------------------------------

struct __APLLICATION_FLAGS {
	U8 Timer20msFlag : 1;
	U8 Timer1sFlag : 1;
} Application_Flags;

extern RS_PLATFORM_CONST INPUT_OUTPUT_TYPE kApplication_InputTypes [];
extern RS_PLATFORM_CONST int kApplication_InputTypesCount;

extern RS_PLATFORM_CONST INPUT_OUTPUT_TYPE kApplication_OutputTypes [];
extern RS_PLATFORM_CONST int kApplication_OutputTypesCount;

void Application_Timer1sHandler(void);
void Application_SetValueById(RS_VALUEID ValueId, RS_VALUE Value);


//---------------------------------------------------------------------------
// COMMUNICATION
//---------------------------------------------------------------------------

#define VBUS_ADDRESS_BROADCAST 0x7770

extern I16 VBusAddress;

enum {
	UART_INDEX_VBUS_GP = 0,
};

RS_ERROR Communication_LoopHandler(void);
RS_ERROR inline Communication_Timer20msHandler(void);
RS_ERROR inline Communication_DidReceive(int Index, int Received);
RS_ERROR Communication_StartCycle(void);
RS_ERROR Communication_Initialize(void);



//---------------------------------------------------------------------------
// VALUE
//---------------------------------------------------------------------------

// MenuSystem_GetValueInfoById
U8 Value_IsValueAvailable(RS_VALUEID ValueId);

// MenuSystem_GetValueById
RS_VALUE Value_GetValueById(RS_VALUEID ValueId);

// MenuSystem_SetValueById
void Value_SetValueById(RS_VALUEID ValueId, RS_VALUE Value);

#endif /* __ARDUINO_PT1000_SHIELD_APPLICATION_H_ */
