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
#include "RESOLino_Pt1000.h"

static void _SendByte(uint8_t Output);
static void _DatagramReceived(int16_t  Dst, int16_t  Src, int16_t  Cmd, uint16_t ValueId, uint32_t Value);
static void Rs_VBus_SendDatagram(int16_t  Dst, int16_t  Src, int16_t  Cmd, uint16_t ValueId, uint32_t Value);
static void Rs_VBus_SendPacketFrame(uint8_t Data0, uint8_t Data1, uint8_t Data2, uint8_t Data3);
static void Rs_VBus_SendPacketHeader(int16_t  Dst, int16_t  Src, int16_t  Cmd, int8_t FrameCount);
static void Rs_VBus_SendByteWithCrcAndSeptet(uint8_t Data, uint8_t* Crc, uint8_t* SeptetValue, uint8_t* SeptetMask);
static void Rs_VBus_SendByteWithCrc(uint8_t Data, uint8_t* Crc);
static void Rs_VBus_ByteReceived(uint8_t Data);
static int Rs_VBus_CheckRxCrc(const uint8_t *Buffer, int Offset, int Length, uint8_t RxProtocolSelector);
static uint8_t Rs_VBus_EndTxCrc(uint8_t Crc);
static uint8_t Rs_VBus_CalcTxCrc(uint8_t Crc, uint8_t Data);
static uint8_t Rs_VBus_StartTxCrc();
static uint8_t Rs_VBus_ExtractSeptett(uint8_t* Buffer, uint16_t Offset, uint16_t Length);
static void Rs_VBus_InjectSeptett(uint8_t* Buffer, uint16_t Offset, uint16_t Length, uint8_t Septett);
static uint8_t Rs_VBus_CalcCrc(uint8_t* Buffer, uint16_t Offset, uint16_t Length);

static int _RxIndex;
static int8_t _RxFrameNr;
static uint8_t _RxBuffer[17];
static double Values[8];

static HardwareSerial *HWSerial;
#if defined SOFTWARESERIAL_AVAILABLE
	static SoftwareSerial *SWSerial;
#endif

static uint16_t ValueIDs[8] = {
	0x1001,
	0x1002,
	0x1003,
	0x1004,
	0x1005,
	0x1006,
	0x1007,
	0x1008
};

RESOLino_Pt1000::RESOLino_Pt1000(HardwareSerial *ser) {
	HWSerial = ser;
#if defined SOFTWARESERIAL_AVAILABLE
	SWSerial = NULL;
#endif
}

#if defined SOFTWARESERIAL_AVAILABLE
RESOLino_Pt1000::RESOLino_Pt1000(SoftwareSerial *ser) {
	HWSerial = NULL;
	SWSerial = ser;
}
#endif

void RESOLino_Pt1000::init(uint32_t Baudrate) {
	_RxIndex = -1;

	for(int i = 0; i < 8; i++) {
		Values[i] = -1;
	}
	setBaudrate(Baudrate);
}

void RESOLino_Pt1000::setADOperatingSpeed(AD_OPERAING_SPEED hz) {
	Rs_VBus_SendDatagram(SHIELD_ADDRESS, SOURCE_ADDRESS, 0x0200, ADSPEED_ID, hz);

	if (receive() < 0) {
		return;
	}
}

void RESOLino_Pt1000::setBaudrate(uint32_t Baudrate) {
	uint32_t Baudrates[4] = {9600, 19200, 38400, 57600};
	int i = 0;

	bool success = false;
	while (!success && i < 3) {
		Rs_VBus_SendDatagram(SHIELD_ADDRESS, SOURCE_ADDRESS, 0x0200, BAUD_ID, Baudrate);

		if(receive() > -1) {
			delay(20);
			begin(Baudrate);
			success = true;
		} else {
			begin(Baudrates[i]);
			i++;
		}
	}
	if (HWSerial) {
		int retry;
		double value;
		for (int i = 0; i < 8; i++) {
			value = -1.0;
			retry = 0;
			do {
				update(i+1, &value, Conversion_RawOhm, NULL);
			} while (value == -1.0 && retry++ < 50);
		}
	}
}

void RESOLino_Pt1000::setPriorityTerminal(int32_t  Terminal) {
	Rs_VBus_SendDatagram(SHIELD_ADDRESS, SOURCE_ADDRESS, 0x0200, PRIO_ID, Terminal - 1);
	receive();
}

void RESOLino_Pt1000::update(int SensorNr, double *Value, uint8_t Conversion, double (*CustomConversionFunction)(double RawOhm)) {
	Rs_VBus_SendDatagram(SHIELD_ADDRESS, SOURCE_ADDRESS, 0x0300, ValueIDs[SensorNr - 1], 0x0);
	if (receive() < 0) {
		*Value = -1;
		return;
	}
	double raw = Values[SensorNr - 1];
	if (raw == -1.0) {
		*Value = -1.0;
		return;
	}
	double radicand = 0;
	if (Conversion == Conversion_RawOhm) {
		*Value = raw;
	} else if (Conversion == Conversion_Pt100_To_DegreeCentigrade) {
		radicand = 1 - 0.15123 * (raw / 100 - 1);
		radicand = radicand < 0 ? 0 : radicand;

		*Value = 100.0 * 3.3838 * (1 - sqrt(1 - radicand));
	} else if (Conversion == Conversion_Pt500_To_DegreeCentigrade) {
		radicand = 1 - 0.15123 * (raw / 500 - 1);
		radicand = radicand < 0 ? 0 : radicand;

		*Value = 500.0 * 3.3838 * (1 - sqrt(radicand));
	} else if (Conversion == Conversion_Pt1000_To_DegreeCentigrade) {
		radicand = 1 - 0.15123 * (raw / 1000 - 1);
		radicand = radicand < 0 ? 0 : radicand;

		*Value = 1000.0 * 3.3838 * (1 - sqrt(radicand));
	} else if (Conversion == Conversion_Pt100_To_DegreeFahrenheit) {
		radicand = 1 - 0.15123 * (raw / 100 - 1);
		radicand = radicand < 0 ? 0 : radicand;

		*Value = 100.0 * 3.3838 * (1 - sqrt(1 - radicand)) * 1.8 + 32;
	} else if (Conversion == Conversion_Pt500_To_DegreeFahrenheit) {
		radicand = 1 - 0.15123 * (raw / 500 - 1);
		radicand = radicand < 0 ? 0 : radicand;

		*Value = 500.0 * 3.3838 * (1 - sqrt(radicand)) * 1.8 + 32;
	} else if (Conversion == Conversion_Pt1000_To_DegreeFahrenheit) {
		radicand = 1 - 0.15123 * (raw / 1000 - 1);
		radicand = radicand < 0 ? 0 : radicand;

		*Value = 1000.0 * 3.3838 * (1 - sqrt(radicand)) * 1.8 + 32;
	} else if (Conversion == Conversion_CustomFunction) {
		*Value = CustomConversionFunction(raw);
	}
}

int RESOLino_Pt1000::receive(void) {
	long timeout = millis();
	while (!available()) {
		if (millis() - timeout > TIMEOUT) {
			return -1;
		}
	}
	while (available()) {
		delay(1);
		Rs_VBus_ByteReceived(read());
	}
	return 0;
}

void RESOLino_Pt1000::begin(uint32_t baud) {
#if defined SOFTWARESERIAL_AVAILABLE
	if(SWSerial) {
		SWSerial->begin(baud);
	} else {
		HWSerial->begin(baud);
	}
#else
	HWSerial->begin(baud);
#endif
}

int RESOLino_Pt1000::available() {
#if defined SOFTWARESERIAL_AVAILABLE
	if(SWSerial) {
		return SWSerial->available();
	} else {
		return HWSerial->available();
	}
#else
	HWSerial->available();
#endif
}

int RESOLino_Pt1000::read() {
#if defined SOFTWARESERIAL_AVAILABLE
	if(SWSerial) {
		return SWSerial->read();
	} else {
		return HWSerial->read();
	}
#else
	HWSerial->read();
#endif
}

static void _SendByte(uint8_t Output) {
#if defined SOFTWARESERIAL_AVAILABLE
	if(SWSerial) {
		SWSerial->write(Output);
	} else {
		HWSerial->write(Output);
	}
#else
	HWSerial->write(Output);
#endif
}

static void _DatagramReceived(int16_t  Dst, int16_t  Src, int16_t  Cmd, uint16_t ValueId, uint32_t Value) {
	int idx = -1;
	for(int i = 0; i < 8; i++) {
		if (ValueId == ValueIDs[i]) {
			idx = i;
			break;
		}
	}
	if(idx > -1) {
		// Value = Value > MAXRAW ? MAXRAW : Value;
		Values[idx] = ((double)Value) / 1000.0;
	}
}

static void Rs_VBus_InjectSeptett(uint8_t* Buffer, uint16_t Offset, uint16_t Length, uint8_t Septett) {
	uint16_t i;
	for (i = 0; i < Length; i++) {
		if (Septett & (1 << i)) {
			Buffer [Offset + i] |= 0x80;
		}
	}
}

static uint8_t Rs_VBus_StartTxCrc() {
	uint8_t Crc;
	Crc = 0xFF;
	return Crc;
}

static uint8_t Rs_VBus_CalcTxCrc(uint8_t Crc, uint8_t Data) {
	Crc -= Data;
	return Crc;
}


static uint8_t Rs_VBus_EndTxCrc(uint8_t Crc) {
	Crc = (Crc & 0x7F);
	return Crc;
}

static int Rs_VBus_CheckRxCrc(const uint8_t *Buffer, int Offset, int Length, uint8_t RxProtocolSelector) {
	int Result;
	if (RxProtocolSelector == 0) {
		uint8_t Crc;
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

static void Rs_VBus_ByteReceived(uint8_t Data) {
	int16_t  Dst, Src, Cmd;
	int8_t FrameCount;
	uint16_t ValueId;
	uint32_t Value;
	if (Data == 0xAA) {
		_RxIndex = 0;
	} else if (Data & 0x80) {
		_RxIndex = -1;
	} else if (_RxIndex >= (int)sizeof (_RxBuffer)) {
		_RxIndex = -1;
	}
	if (_RxIndex >= 0) {
		_RxBuffer [_RxIndex++] = Data;
		if ((_RxIndex == 10) && ((_RxBuffer [5] & 0xF0) == 0x10)) {
			if (Rs_VBus_CheckRxCrc(_RxBuffer, 1, 9, _RxBuffer [5] & 0x0F)) {
				_RxFrameNr = -1;
				Dst = ((uint16_t) _RxBuffer [1]) | (((uint16_t) _RxBuffer [2]) << 8);
				Src = ((uint16_t) _RxBuffer [3]) | (((uint16_t) _RxBuffer [4]) << 8);
				Cmd = ((uint16_t) _RxBuffer [6]) | (((uint16_t) _RxBuffer [7]) << 8);
				FrameCount = _RxBuffer [8];
				//_PacketReceived(Dst, Src, Cmd, FrameCount, _RxFrameNr++, 0);
				if (FrameCount == _RxFrameNr) {
					_RxIndex = -1;
				}
			} else {
				_RxIndex = -1;
			}
		} else if ((_RxIndex == 16) && ((_RxBuffer [5] & 0xF0) == 0x10)) {
			if (Rs_VBus_CheckRxCrc(_RxBuffer, 10, 6, _RxBuffer [5] & 0x0F)) {
				Rs_VBus_InjectSeptett(_RxBuffer, 10, 4, _RxBuffer [14]);
				Dst = ((uint16_t) _RxBuffer [1]) | (((uint16_t) _RxBuffer [2]) << 8);
				Src = ((uint16_t) _RxBuffer [3]) | (((uint16_t) _RxBuffer [4]) << 8);
				Cmd = ((uint16_t) _RxBuffer [6]) | (((uint16_t) _RxBuffer [7]) << 8);
				FrameCount = _RxBuffer [8];
				//_PacketReceived(Dst, Src, Cmd, FrameCount, _RxFrameNr++, _RxBuffer + 10);
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
				Dst = ((uint16_t) _RxBuffer [1]) | (((uint16_t) _RxBuffer [2]) << 8);
				Src = ((uint16_t) _RxBuffer [3]) | (((uint16_t) _RxBuffer [4]) << 8);
				Cmd = ((uint16_t) _RxBuffer [6]) | (((uint16_t) _RxBuffer [7]) << 8);
				ValueId = ((uint16_t) _RxBuffer [8]) | (((uint16_t) _RxBuffer [9]) << 8);
				Value = ((uint32_t) _RxBuffer [10]) | (((uint32_t) _RxBuffer [11]) << 8) | (((uint32_t) _RxBuffer [12]) << 16) | (((uint32_t) _RxBuffer [13]) << 24);
				_DatagramReceived(Dst, Src, Cmd, ValueId, Value);
				_RxIndex = -1;
			} else {
				_RxIndex = -1;
			}
		}
	}
}

static void Rs_VBus_SendByteWithCrc(uint8_t Data, uint8_t* Crc) {
	*Crc = Rs_VBus_CalcTxCrc(*Crc, Data);
	_SendByte(Data);
}

static void Rs_VBus_SendByteWithCrcAndSeptet(uint8_t Data, uint8_t* Crc, uint8_t* SeptetValue, uint8_t* SeptetMask) {
	if (Data & 0x80) {
		*SeptetValue |= *SeptetMask;
	}
	*SeptetMask <<= 1;

	Rs_VBus_SendByteWithCrc(Data & 0x7F, Crc);
}

static void Rs_VBus_SendPacketHeader(int16_t  Dst, int16_t  Src, int16_t  Cmd, int8_t FrameCount) {
	uint8_t Crc;

	Crc = Rs_VBus_StartTxCrc();
	_SendByte(0xAA);
	Rs_VBus_SendByteWithCrc(Dst, &Crc);
	Rs_VBus_SendByteWithCrc(Dst >> 8, &Crc);
	Rs_VBus_SendByteWithCrc(Src, &Crc);
	Rs_VBus_SendByteWithCrc(Src >> 8, &Crc);
	Rs_VBus_SendByteWithCrc(0x10, &Crc);
	Rs_VBus_SendByteWithCrc(Cmd, &Crc);
	Rs_VBus_SendByteWithCrc(Cmd >> 8, &Crc);
	Rs_VBus_SendByteWithCrc(FrameCount, &Crc);
	_SendByte(Rs_VBus_EndTxCrc(Crc));
}

static void Rs_VBus_SendPacketFrame(uint8_t Data0, uint8_t Data1, uint8_t Data2, uint8_t Data3) {
	uint8_t Crc, Septet, Mask;

	Crc = Rs_VBus_StartTxCrc();
	Septet = 0;
	Mask = 1;
	Rs_VBus_SendByteWithCrcAndSeptet(Data0, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(Data1, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(Data2, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(Data3, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrc(Septet, &Crc);
	_SendByte(Rs_VBus_EndTxCrc(Crc));
}

static void Rs_VBus_SendDatagram(int16_t  Dst, int16_t  Src, int16_t  Cmd, uint16_t ValueId, uint32_t Value) {
	uint8_t Crc, Septet, Mask;

	Crc = Rs_VBus_StartTxCrc();
	Septet = 0;
	Mask = 1;

	_SendByte(0xAA);
	Rs_VBus_SendByteWithCrc(Dst, &Crc);
	Rs_VBus_SendByteWithCrc(Dst >> 8, &Crc);
	Rs_VBus_SendByteWithCrc(Src, &Crc);
	Rs_VBus_SendByteWithCrc(Src >> 8, &Crc);
	Rs_VBus_SendByteWithCrc(0x20, &Crc);
	Rs_VBus_SendByteWithCrc(Cmd, &Crc);
	Rs_VBus_SendByteWithCrc(Cmd >> 8, &Crc);
	Rs_VBus_SendByteWithCrcAndSeptet(ValueId, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(ValueId >> 8, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(Value, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(Value >> 8, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(Value >> 16, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrcAndSeptet(Value >> 24, &Crc, &Septet, &Mask);
	Rs_VBus_SendByteWithCrc(Septet, &Crc);
	_SendByte(Rs_VBus_EndTxCrc(Crc));
}
