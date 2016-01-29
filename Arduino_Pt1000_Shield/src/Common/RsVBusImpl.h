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



#define _HardwareInitialize						RS_VBUS_INFO_(HardwareInitialize)
#define _PacketReceived							RS_VBUS_INFO_(PacketReceived)
#define _DatagramReceived						RS_VBUS_INFO_(DatagramReceived)
#define _SendByte								RS_VBUS_INFO_(SendByte)
#define _RxIndex								RS_VBUS_STATE_(RxIndex)
#define _RxFrameNr								RS_VBUS_STATE_(RxFrameNr)
#define _RxBuffer								RS_VBUS_STATE_(RxBuffer)


U8 Rs_VBus_CalcCrc(U8* Buffer, UINT Offset, UINT Length)
{
	U8 Crc;
	UINT i;

	Crc = 0xFF;
	for (i = 0; i < Length; i++) {
		Crc -= Buffer [Offset + i];
	}
	return (Crc & 0x7F);
}



void Rs_VBus_InjectSeptett(U8* Buffer, UINT Offset, UINT Length, U8 Septett)
{
	UINT i;

	for (i = 0; i < Length; i++) {
		if (Septett & (1 << i)) {
			Buffer [Offset + i] |= 0x80;
		}
	}
}



U8 Rs_VBus_ExtractSeptett(U8* Buffer, UINT Offset, UINT Length)
{
	U8 Septett;
	UINT i;

	Septett = 0;
	for (i = 0; i < Length; i++) {
		if (Buffer [Offset + 1] & 0x80) {
			Septett |= (1 << i);
			Buffer [Offset + i] &= 0x7F;
		}
	}
	return Septett;
}



static U8 Rs_VBus_StartTxCrc(RS_VBUS_ARGS)
{
	U8 Crc;

	Crc = 0xFF;

	return Crc;
}



static U8 Rs_VBus_CalcTxCrc(RS_VBUS_ARGS_ U8 Crc, U8 Data)
{
	Crc -= Data;

	return Crc;
}



static U8 Rs_VBus_EndTxCrc(RS_VBUS_ARGS_ U8 Crc)
{
	Crc = (Crc & 0x7F);

	return Crc;
}



static int Rs_VBus_CheckRxCrc(const U8 *Buffer, int Offset, int Length, U8 RxProtocolSelector)
{
	int Result;

	if (RxProtocolSelector == 0) {
		U8 Crc;
		int i;

		Crc = 0xFF;
		for (i = 0; i < Length; i++) {
			Crc -= Buffer [Offset + i];
		}
		Result = ((Crc & 0x7F) == 0x00);
	} else {
		Result = 0;
	}
	return Result;
}



void Rs_VBus_ByteReceived(RS_VBUS_ARGS_ U8 Data)
{
	I16 Dst, Src, Cmd;
	I8 FrameCount;
	U16 ValueId;
	U32 Value;

	if (Data == 0xAA) {
		_RxIndex = 0;
	} else if (Data & 0x80) {
		_RxIndex = -1;
	} else if (_RxIndex >= sizeof (_RxBuffer)) {
		_RxIndex = -1;
	}
	if (_RxIndex >= 0) {
		_RxBuffer [_RxIndex++] = Data;
		if ((_RxIndex == 10) && ((_RxBuffer [5] & 0xF0) == 0x10)) {
			if (Rs_VBus_CheckRxCrc(_RxBuffer, 1, 9, _RxBuffer [5] & 0x0F)) {
				_RxFrameNr = -1;
				Dst = ((U16) _RxBuffer [1]) | (((U16) _RxBuffer [2]) << 8);
				Src = ((U16) _RxBuffer [3]) | (((U16) _RxBuffer [4]) << 8);
				Cmd = ((U16) _RxBuffer [6]) | (((U16) _RxBuffer [7]) << 8);
				FrameCount = _RxBuffer [8];
				_PacketReceived(Dst, Src, Cmd, FrameCount, _RxFrameNr++, 0);
				if (FrameCount == _RxFrameNr) {
					_RxIndex = -1;
				}
			} else {
				_RxIndex = -1;
			}
		} else if ((_RxIndex == 16) && ((_RxBuffer [5] & 0xF0) == 0x10)) {
			if (Rs_VBus_CheckRxCrc(_RxBuffer, 10, 6, _RxBuffer [5] & 0x0F)) {
				Rs_VBus_InjectSeptett(_RxBuffer, 10, 4, _RxBuffer [14]);
				Dst = ((U16) _RxBuffer [1]) | (((U16) _RxBuffer [2]) << 8);
				Src = ((U16) _RxBuffer [3]) | (((U16) _RxBuffer [4]) << 8);
				Cmd = ((U16) _RxBuffer [6]) | (((U16) _RxBuffer [7]) << 8);
				FrameCount = _RxBuffer [8];
				_PacketReceived(Dst, Src, Cmd, FrameCount, _RxFrameNr++, _RxBuffer + 10);
				if (FrameCount == _RxFrameNr) {
					_RxIndex = -1;
				} else {
					_RxIndex = 10;
				}
			} else {
				_RxIndex = -1;
			}
		} else if ((_RxIndex == 16) && ((_RxBuffer [5] & 0xF0) == 0x20)) {
			if (Rs_VBus_CheckRxCrc(_RxBuffer, 1, 15, _RxBuffer [5] & 0x0F)) {
				Rs_VBus_InjectSeptett(_RxBuffer, 8, 6, _RxBuffer [14]);
				Dst = ((U16) _RxBuffer [1]) | (((U16) _RxBuffer [2]) << 8);
				Src = ((U16) _RxBuffer [3]) | (((U16) _RxBuffer [4]) << 8);
				Cmd = ((U16) _RxBuffer [6]) | (((U16) _RxBuffer [7]) << 8);
				ValueId = ((U16) _RxBuffer [8]) | (((U16) _RxBuffer [9]) << 8);
				Value = ((U32) _RxBuffer [10]) | (((U32) _RxBuffer [11]) << 8) | (((U32) _RxBuffer [12]) << 16) | (((U32) _RxBuffer [13]) << 24);
				_DatagramReceived(Dst, Src, Cmd, ValueId, Value);
				_RxIndex = -1;
			} else {
				_RxIndex = -1;
			}
		}
	}
}



void Rs_VBus_SendByteWithCrc(RS_VBUS_ARGS_ U8 Data, U8* Crc)
{
	*Crc = Rs_VBus_CalcTxCrc(RS_VBUS_PARAMS_ *Crc, Data);
	_SendByte(Data);
}



void Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_ARGS_ U8 Data, U8* Crc, U8* SeptetValue, U8* SeptetMask)
{
	if (Data & 0x80) {
		*SeptetValue |= *SeptetMask;
	}
	*SeptetMask <<= 1;

	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Data & 0x7F, Crc);
}



void Rs_VBus_SendPacketHeader(RS_VBUS_ARGS_ I16 Dst, I16 Src, I16 Cmd, I8 FrameCount)
{
	U8 Crc;

	Crc = Rs_VBus_StartTxCrc(RS_VBUS_PARAMS);
	_SendByte(0xAA);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Dst, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Dst >> 8, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Src, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Src >> 8, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ 0x10, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Cmd, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Cmd >> 8, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ FrameCount, &Crc);
	_SendByte(Rs_VBus_EndTxCrc(RS_VBUS_PARAMS_ Crc));
}



void Rs_VBus_SendPacketFrame(RS_VBUS_ARGS_ U8 Data0, U8 Data1, U8 Data2, U8 Data3)
{
	U8 Crc, Septet, Mask;

	Crc = Rs_VBus_StartTxCrc(RS_VBUS_PARAMS);
	Septet = 0;
	Mask = 1;
	Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_PARAMS_ Data0, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_PARAMS_ Data1, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_PARAMS_ Data2, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_PARAMS_ Data3, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Septet, &Crc);
	_SendByte(Rs_VBus_EndTxCrc(RS_VBUS_PARAMS_ Crc));
}



void Rs_VBus_SendDatagram(RS_VBUS_ARGS_ I16 Dst, I16 Src, I16 Cmd, U16 ValueId, U32 Value)
{
	U8 Crc, Septet, Mask;

	Crc = Rs_VBus_StartTxCrc(RS_VBUS_PARAMS);
	Septet = 0;
	Mask = 1;

	_SendByte(0xAA);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Dst, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Dst >> 8, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Src, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Src >> 8, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ 0x20, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Cmd, &Crc);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Cmd >> 8, &Crc);
	Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_PARAMS_ ValueId, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_PARAMS_ ValueId >> 8, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_PARAMS_ Value, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_PARAMS_ Value >> 8, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_PARAMS_ Value >> 16, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_PARAMS_ Value >> 24, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrc(RS_VBUS_PARAMS_ Septet, &Crc);
	_SendByte(Rs_VBus_EndTxCrc(RS_VBUS_PARAMS_ Crc));
}



void Rs_VBus_Initialize(RS_VBUS_ARGS)
{
	_HardwareInitialize();
	_RxIndex = -1;
}
