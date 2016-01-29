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
#include "Values.h"

#undef DATG
#define DTAG "[VALUE]\t"

#ifndef VALUE_DEBUG
#define VALUE_DEBUG 0
#endif


#if VALUE_DEBUG

#define __DPRINTF DPRINTF
#define __DPRINT DPRINT

#else

#define __DPRINTF(x, ...)
#define __DPRINT(x)

#endif


//---------------------------------------------------------------------------
// Value
//---------------------------------------------------------------------------

RS_VALUES Values = {0};

VALUES_VALUE_INFO InputValues [] = {
		{VALUEID_INPUT_SENSOR1, &Values.Input_Sensor1},
		{VALUEID_INPUT_SENSOR2, &Values.Input_Sensor2},
		{VALUEID_INPUT_SENSOR3, &Values.Input_Sensor3},
		{VALUEID_INPUT_SENSOR4, &Values.Input_Sensor4},
		{VALUEID_INPUT_SENSOR5, &Values.Input_Sensor5},
		{VALUEID_INPUT_SENSOR6, &Values.Input_Sensor6},
		{VALUEID_INPUT_SENSOR7, &Values.Input_Sensor7},
		{VALUEID_INPUT_SENSOR8, &Values.Input_Sensor8},
		{VALUEID_INPUT_DIPSWITCH, &Values.Input_DipSwitch},
		{VALUEID_INPUT_PRIORITY_CHANNEL, &Values.Input_PriorityChannel},
};

RS_PLATFORM_CONST int kInputValuesCount = countof(InputValues);

VALUES_VALUE_INFO OutputValues [] = {
		{VALUEID_OUTPUT_BAUDRATE, &Values.Output_Baudrate},
};

RS_PLATFORM_CONST int kOutputValuesCount = countof(OutputValues);

RS_PLATFORM_CONST RS_VALUEID kInputValueIds [] = {
	VALUEID_INPUT_SENSOR1,
	VALUEID_INPUT_SENSOR2,
	VALUEID_INPUT_SENSOR3,
	VALUEID_INPUT_SENSOR4,
	VALUEID_INPUT_SENSOR5,
	VALUEID_INPUT_SENSOR6,
	VALUEID_INPUT_SENSOR7,
	VALUEID_INPUT_SENSOR8,
	VALUEID_INPUT_DIPSWITCH,
	VALUEID_INPUT_PRIORITY_CHANNEL,
};
RS_PLATFORM_CONST int kInputValueIdsCount = countof(kInputValueIds);

RS_PLATFORM_CONST RS_VALUEID kOutputValueIds [] = {
		VALUEID_OUTPUT_BAUDRATE,
};
RS_PLATFORM_CONST int kOutputValueIdsCount = countof(kOutputValueIds);

// MenuSystem_GetValueInfoById
VALUES_VALUE_INFO* Value_GetValueInfoById(RS_VALUEID ValueId)
{
	VALUES_VALUE_INFO* result = 0;
	int i, count = 0;
	VALUES_VALUE_INFO *table = 0;
	RS_VALUEID tableValueId = 0;

	if (ValueId < VALUEID_INPUT_BASE) {
		count = Platform_ReadConstInt(&kOutputValuesCount);
		table = OutputValues;
	} else if (ValueId > VALUEID_INPUT_BASE) {
		count = Platform_ReadConstInt(&kInputValuesCount);
		table = InputValues;
	}

	if (table != 0) {
		for (i = 0; i < count; i++) {
			tableValueId = table[i].ValueId;
			if (tableValueId == ValueId) {
				result = &table[i];
				break;
			}
		}
	}
	return result;
}



U8 Value_IsValueAvailable(RS_VALUEID ValueId)
{
	U8 result = 0;
	VALUES_VALUE_INFO *info = 0;

	info = Value_GetValueInfoById(ValueId);
	result = (info != 0);

	__DPRINTF(DTAG "IsValueAvailable(0x%04X) -> %d.\n", ValueId, result);
	return result;
}



RS_VALUE Value_GetValueById(RS_VALUEID ValueId)
{
	RS_VALUE Value = 0;
	VALUES_VALUE_INFO *info = 0;

	info = Value_GetValueInfoById(ValueId);

	if (info != 0) {
		Value = *info->pValue;
	}

	return Value;
}



void Value_SetValueById(RS_VALUEID ValueId, RS_VALUE Value)
{
	VALUES_VALUE_INFO *info = 0;

	__DPRINTF(DTAG "SetValueById(0x%04X, %ld", ValueId, Value);
	__DPRINTF(" (0x%lX)).\n", Value);

	info = Value_GetValueInfoById(ValueId);

	if (info != 0) {
		if (info->ValueId == ValueId) {
			*info->pValue = Value;
			Application_SetValueById(ValueId, Value);
		}
	}
}

#undef VALUE_DEBUG
