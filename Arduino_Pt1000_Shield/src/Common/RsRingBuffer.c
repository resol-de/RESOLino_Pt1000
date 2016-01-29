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
#include "PlatformApi.h"
#include "RsRingBuffer.h"


#define _Size									RS_RINGBUFFER_MT_STATE_(Size)
#define _WriteIndex								RS_RINGBUFFER_MT_STATE_(WriteIndex)
#define _ReadIndex								RS_RINGBUFFER_MT_STATE_(ReadIndex)
#define _Data									RS_RINGBUFFER_MT_STATE_(Data)


UINT Rs_RingBuffer_GetLength(RS_RINGBUFFER_STATE* __Buffer_State)
{
	UINT Length;

	if (_WriteIndex >= _ReadIndex) {
		Length = _WriteIndex - _ReadIndex;
	} else {
		Length = _Size + _WriteIndex - _ReadIndex;
	}

	return Length;
}



UINT Rs_RingBuffer_GetSpace(RS_RINGBUFFER_STATE* __Buffer_State)
{
	UINT Space;

	if (_ReadIndex > _WriteIndex) {
		Space = _ReadIndex - _WriteIndex - 1;
	} else {
		Space = _Size + _ReadIndex - _WriteIndex - 1;
	}

	return Space;
}



int Rs_RingBuffer_Write(RS_RINGBUFFER_STATE* __Buffer_State, void* Buffer, UINT Size)
{
	int Result;
	UINT i;

	Result = (Rs_RingBuffer_GetSpace(__Buffer_State) >= Size);
	if (Result) {
		for (i = 0; i < Size; i++) {
			(&_Data) [_WriteIndex++] = ((U8*) Buffer) [i];
			if (_WriteIndex >= _Size) {
				_WriteIndex = 0;
			}
		}
	}

	return Result;
}



int Rs_RingBuffer_Read(RS_RINGBUFFER_STATE* __Buffer_State, void* Buffer, UINT Size)
{
	int Result;
	UINT i;

	Result = (Rs_RingBuffer_GetLength(__Buffer_State) >= Size);
	if (Result) {
		for (i = 0; i < Size; i++) {
			((U8*) Buffer) [i] = (&_Data) [_ReadIndex++];
			if (_ReadIndex >= _Size) {
				_ReadIndex = 0;
			}
		}
	}

	return Result;
}



void Rs_RingBuffer_Reset(RS_RINGBUFFER_STATE* __Buffer_State)
{
	_WriteIndex = 0;
	_ReadIndex = 0;
}



RS_RINGBUFFER_STATE *Rs_RingBuffer_Initialize(void *Buffer, UINT Size)
{
	RS_RINGBUFFER_STATE* State;

	State = RS_ALIGN_POINTER_UP(Buffer);
	State->Size = (IPTR) Buffer + Size - (IPTR) (&State->Data);
	Rs_RingBuffer_Reset(State);

	return State;
}
