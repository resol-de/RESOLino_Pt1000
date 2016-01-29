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
#ifndef VALUES_H_
#define VALUES_H_

#include "PlatformApi.h"

/////////////////////
// Inputs
////////////////////
#define VALUEID_INPUT_BASE					0x1000

#define VALUEID_INPUT_SENSOR1				(VALUEID_INPUT_BASE + 1)
#define VALUEID_INPUT_SENSOR2				(VALUEID_INPUT_BASE + 2)
#define VALUEID_INPUT_SENSOR3				(VALUEID_INPUT_BASE + 3)
#define VALUEID_INPUT_SENSOR4				(VALUEID_INPUT_BASE + 4)
#define VALUEID_INPUT_SENSOR5				(VALUEID_INPUT_BASE + 5)
#define VALUEID_INPUT_SENSOR6				(VALUEID_INPUT_BASE + 6)
#define VALUEID_INPUT_SENSOR7				(VALUEID_INPUT_BASE + 7)
#define VALUEID_INPUT_SENSOR8				(VALUEID_INPUT_BASE + 8)
#define VALUEID_INPUT_DIPSWITCH				(VALUEID_INPUT_BASE + 9)
#define VALUEID_INPUT_PRIORITY_CHANNEL		(VALUEID_INPUT_BASE + 10)

extern RS_PLATFORM_CONST RS_VALUEID kInputValueIds [];
extern RS_PLATFORM_CONST int kInputValueIdsCount;

/////////////////////
// Outputs
////////////////////
#define VALUEID_OUTPUT_BASE				0x0000

#define VALUEID_OUTPUT_BAUDRATE			(VALUEID_OUTPUT_BASE + 1)


extern RS_PLATFORM_CONST RS_VALUEID kOutputValueIds [];
extern RS_PLATFORM_CONST int kOutputValueIdsCount;


typedef struct __MENUSYSTEM_VALUES {
	RS_VALUE Input_Sensor1;
	RS_VALUE Input_Sensor2;
	RS_VALUE Input_Sensor3;
	RS_VALUE Input_Sensor4;
	RS_VALUE Input_Sensor5;
	RS_VALUE Input_Sensor6;
	RS_VALUE Input_Sensor7;
	RS_VALUE Input_Sensor8;
	RS_VALUE Input_DipSwitch;
	RS_VALUE Input_PriorityChannel;
	RS_VALUE Output_Baudrate;
} RS_VALUES;

extern RS_VALUES Values;

typedef struct __VALUES_VALUE_INFO {
	RS_VALUEID ValueId;
	RS_VALUE *pValue;
} VALUES_VALUE_INFO;

extern VALUES_VALUE_INFO InputValues [];
extern RS_PLATFORM_CONST int kInputValuesCount;

extern VALUES_VALUE_INFO OutputValues [];
extern RS_PLATFORM_CONST int kOutputValuesCount;

#define _Input_Sensor1 Values.Input_Sensor1
#define _Input_Sensor2 Values.Input_Sensor2
#define _Input_Sensor3 Values.Input_Sensor3
#define _Input_Sensor4 Values.Input_Sensor4
#define _Input_Sensor5 Values.Input_Sensor5
#define _Input_Sensor6 Values.Input_Sensor6
#define _Input_Sensor7 Values.Input_Sensor7
#define _Input_Sensor8 Values.Input_Sensor8
#define _Input_DipSwitch Values.Input_DipSwitch
#define _Input_PriorityChannel Values.Input_PriorityChannel

#define _Output_Baudrate Values.Output_Baudrate

#endif /* VALUES_H_ */
