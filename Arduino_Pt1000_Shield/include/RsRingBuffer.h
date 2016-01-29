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
#ifndef RSRINGBUFFER_H_
#define RSRINGBUFFER_H_

#include "PlatformApi.h"


//===========================================================================
// RINGBUFFER
//===========================================================================
typedef struct __RS_RINGBUFFER_STATE
{
	UINT Size;
	UINT WriteIndex;
	UINT ReadIndex;
	U8 Data;
} RS_RINGBUFFER_STATE;

#define RS_RINGBUFFER_DECLARATION(Name, Size) RS_ALIGNED_BUFFER_DECLARATION(Name, (Size) + sizeof (RS_RINGBUFFER_STATE))


#define RS_RINGBUFFER_MT_STATE_(a)	__Buffer_State->a


UINT Rs_RingBuffer_GetLength(RS_RINGBUFFER_STATE* __Buffer_State);
UINT Rs_RingBuffer_GetSpace(RS_RINGBUFFER_STATE* __Buffer_State);
int Rs_RingBuffer_Write(RS_RINGBUFFER_STATE* __Buffer_State, void* Buffer, UINT Size);
int Rs_RingBuffer_Read(RS_RINGBUFFER_STATE* __Buffer_State, void* Buffer, UINT Size);
int Rs_RingBuffer_Peek(RS_RINGBUFFER_STATE* __Buffer_State, void* Buffer, UINT Size);
int Rs_RingBuffer_Drop(RS_RINGBUFFER_STATE* __Buffer_State, UINT Size);
void Rs_RingBuffer_Reset(RS_RINGBUFFER_STATE* __Buffer_State);
RS_RINGBUFFER_STATE *Rs_RingBuffer_Initialize(void *Buffer, UINT Size);


#endif /* RSRINGBUFFER_H_ */
