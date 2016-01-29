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
#define DTAG "[COMM]\t"

#ifndef COMM_DEBUG
#define COMM_DEBUG 0
#endif


#if COMM_DEBUG

#define __DPRINTF DPRINTF
#define __DPRINT DPRINT

#else

#define __DPRINTF(x, ...)
#define __DPRINT(x)

#endif

//---------------------------------------------------------------------------
// VBUS => GENERAL PURPOSE (GP)
//---------------------------------------------------------------------------

#ifdef RS_VBUS_MULTITON
static RS_VBUS_STATE Communication_GP_VBusState;
#endif // RS_VBUS_MULTITON

static volatile I8 __GP_Phase;
static volatile I8 __GP_PhaseTimer;
static volatile I8 __GP_TriggerTimer;

static RS_RINGBUFFER_STATE* __GP_TxBufferState = 0;
#define KOMMUNIKATION_GP_BUFFERSIZE 64

static RS_RINGBUFFER_DECLARATION(__GP_RxBuffer, KOMMUNIKATION_GP_BUFFERSIZE);

static RS_RINGBUFFER_STATE *__GP_RxBufferState;



void VBus_HardwareInitialize(void)
{
	Uart_Reset(UART_INDEX_VBUS_GP, 0);
}



void VBus_PacketReceived(I16 Dst, I16 Src, I16 Cmd, I8 FrameCount, I8 FrameNr, const U8* Data)
{
	if (Dst == VBusAddress) {
		if (FrameNr == -1) {
			// header, data pointer is null
		} else if (FrameNr == 0) {
			// each frame is 4 byte long (Data[4])
		}
	}
}



void VBus_DatagramReceived(I16 Dst, I16 Src, I16 Cmd, U16 ValueId, U32 Value)
{
	U8 ValueInfo;
	RS_VALUE RefValue;

	if (Dst == VBusAddress) {
		__DPRINTF(DTAG "Datagramm RX: 0x%04X => 0x%04X, Cmd 0x%04X, ValueId 0x%04X, Value 0x%04X\n", Src, Dst, Cmd, ValueId, Value);
		if ((Cmd == 0x0200) || (Cmd == 0x0400)) {
			__GP_TriggerTimer = 10;

			ValueInfo = Value_IsValueAvailable(ValueId);
			if (ValueInfo != 0) {
				Value_SetValueById(ValueId, Value);
				RefValue = Value_GetValueById(ValueId);
				Uart_Reset(UART_INDEX_VBUS_GP, 1);
				Rs_VBus_SendDatagram(Src, Dst, 0x0100, ValueId, RefValue);
				Uart_FlushTxBuffer(UART_INDEX_VBUS_GP, 0);

				if (ValueId == VALUEID_OUTPUT_BAUDRATE) {
					Uart_SetBaud(0, Value);
				}
			}
		} else if (Cmd == 0x0300) {
			__GP_TriggerTimer = 10;
			ValueInfo = Value_IsValueAvailable(ValueId);
			if (ValueInfo != 0) {
				RefValue = Value_GetValueById(ValueId);
				Uart_Reset(UART_INDEX_VBUS_GP, 1);
				Rs_VBus_SendDatagram(Src, Dst, 0x0100, ValueId, RefValue);
				Uart_FlushTxBuffer(UART_INDEX_VBUS_GP, 0);
			}
		} else if (Cmd == 0x0600) {
			__GP_TriggerTimer = 0;
		}
	}
}



void VBus_SendByte(U8 Output)
{
	if (__GP_TxBufferState) {
		Rs_RingBuffer_Write(__GP_TxBufferState, &Output, 1);
	}
}




RS_ERROR inline Communication_GP_LoopHandler(void)
{
	U8 Byte;
	I8 Phase;

	while (__GP_RxBufferState && Rs_RingBuffer_GetLength(__GP_RxBufferState)) {
		Rs_RingBuffer_Read(__GP_RxBufferState, &Byte, 1);
		Rs_VBus_ByteReceived(Byte);
	}


	if ((__GP_PhaseTimer > 0) || (__GP_Phase < 0)) {
		return RS_OK;
	}

	Phase = 0;
#if 0 // do not send 'free bus' command
	if (__GP_Phase == Phase++) {
		Rs_VBus_SendDatagram(0x0000, VBusAddress, 0x0500, 0, 0);
		__GP_PhaseTimer = 20;
	} else
#endif
	{
		__GP_Phase = -1;
	}

//	DPRINTF(DTAG "Communication_GP_Phase: %d, ", Communication_GP_Phase);
//	DPRINTF("Communication_GP_TriggerTimer: %d.\n", Communication_GP_TriggerTimer);

	Uart_FlushTxBuffer(UART_INDEX_VBUS_GP, 0);

	if (__GP_Phase >= 0) {
		__GP_Phase = Phase;
	}

	return RS_OK;
}



RS_ERROR Communication_GP_Timer20msHandler(void)
{
	if (__GP_PhaseTimer > 0) {
		__GP_PhaseTimer--;
	}

	return RS_OK;
}



RS_ERROR inline Communication_GP_StartCycle(void)
{
	if (__GP_TriggerTimer > 0) {
		__GP_TriggerTimer--;
	} else if (__GP_Phase < 0) {
		__GP_Phase = 0;
	}

	return RS_OK;
}



RS_ERROR Communication_GP_Initialize(void)
{

	__GP_Phase = -1;
	__GP_PhaseTimer = 0;
	__GP_TriggerTimer = 0;

	__GP_RxBufferState = Rs_RingBuffer_Initialize(__GP_RxBuffer, KOMMUNIKATION_GP_BUFFERSIZE);

	__GP_TxBufferState = Uart_GetTxBuffer(UART_INDEX_VBUS_GP);

	Uart_SetEnable(UART_INDEX_VBUS_GP, 1);

	return RS_OK;
}



//---------------------------------------------------------------------------
// COMMUNICATION
//---------------------------------------------------------------------------
RS_ERROR Communication_LoopHandler(void)
{
	Communication_GP_LoopHandler();

	return RS_OK;
}



RS_ERROR inline Communication_Timer20msHandler(void)
{
	Communication_GP_Timer20msHandler();

	return RS_OK;
}



RS_ERROR inline Communication_DidReceive(int Index, int Received)
{
	U8 Byte;

	Byte = Received & 255;

	if (Index == UART_INDEX_VBUS_GP) {
		if (__GP_RxBufferState && Rs_RingBuffer_GetSpace(__GP_RxBufferState)) {
			Rs_RingBuffer_Write(__GP_RxBufferState, &Byte, 1);
		}
	}
	return RS_OK;
}



RS_ERROR Communication_StartCycle(void)
{
	Communication_GP_StartCycle();

	return RS_OK;
}



RS_ERROR Communication_Initialize(void)
{
	Communication_GP_Initialize();

	return RS_OK;
}

#undef COMM_DEBUG
