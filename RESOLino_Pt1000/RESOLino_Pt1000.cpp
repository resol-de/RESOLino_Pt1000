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

#if defined SOFTWARESERIAL_AVAILABLE
RESOLino_Pt1000::RESOLino_Pt1000(SoftwareSerial *serial) {
	hwSerial = NULL;
	swSerial = serial;
}
#endif
RESOLino_Pt1000::RESOLino_Pt1000(HardwareSerial *serial) {
	hwSerial = serial;
#if defined SOFTWARESERIAL_AVAILABLE
	swSerial = NULL;
#endif
}

void RESOLino_Pt1000::init(uint32_t baudrate) {
	rxIndex = -1;
	for(int i = 0; i < 8; i++) {
		values[i] = -1;
	}
	setBaudrate(baudrate);
}

void RESOLino_Pt1000::setPriorityTerminal(int32_t terminal) {
	sendDatagram(0x0200, RS_VBUS_PRIORITY_ID, terminal - 1);
	receiveData();
}

void RESOLino_Pt1000::setADOperatingSpeed(AD_OPERAING_SPEED hz) {
	sendDatagram(0x0200, RS_VBUS_AD_SPEED_ID, hz);
	if (receiveData() < 0) {
		return;
	}
}

void RESOLino_Pt1000::update(int sensorNr, double *value, uint8_t conversion, double (*customConversionFunction)(double rawOhm)) {
#if defined SOFTWARESERIAL_AVAILABLE
	if(swSerial) { // if more than one SoftwareSerial instances are used, you need to select the one currently listening to
		swSerial->listen();
	}
#endif
	sendDatagram(0x0300, valueIDs[sensorNr - 1], 0x0);
	if (receiveData() < 0) {
		*value = -1;
		return;
	}
	double raw = values[sensorNr - 1];
	if (raw == -1.0) {
		*value = -1.0;
		return;
	}
	*value = raw;
	double radicand = 0;
	uint8_t isFarenheit = 0;
	switch (conversion) {
		case Conversion_RawOhm:
			break;
		case Conversion_Pt100_To_DegreeCentigrade:
			radicand = 100;
			break;
		case Conversion_Pt500_To_DegreeCentigrade:
			radicand = 500;
			break;
		case Conversion_Pt1000_To_DegreeCentigrade:
			radicand = 1000;
			break;
		case Conversion_Pt100_To_DegreeFahrenheit:
			radicand = 100;
			isFarenheit = 1;
			break;
		case Conversion_Pt500_To_DegreeFahrenheit:
			radicand = 500;
			isFarenheit = 1;
			break;
		case Conversion_Pt1000_To_DegreeFahrenheit:
			radicand = 1000;
			isFarenheit = 1;
			break;
		case Conversion_CustomFunction:
			*value = customConversionFunction(raw);
			break;
	}
	if (radicand) {
		*value = (3383.81f - 1315.9f * sqrt(7.61247f - *value / radicand));
	}
	if (isFarenheit) {
		*value = *value * 1.8 + 32;
	}
}

void RESOLino_Pt1000::begin(uint32_t baud) {
#if defined SOFTWARESERIAL_AVAILABLE
	if(swSerial) {
		swSerial->begin(baud);
	} else {
		hwSerial->begin(baud);
	}
#else
	hwSerial->begin(baud);
#endif
}

void RESOLino_Pt1000::setBaudrate(uint32_t baud) {
	uint32_t baudrates[4] = {9600, 19200, 38400, 57600};
	int i = 0;
	bool success = false;
	while (!success && i < 3) {
		sendDatagram(0x0200, RS_VBUS_BAUD_ID, baud);
		if(receiveData() > -1) {
			delay(20);
			begin(baud);
			success = true;
		} else {
			begin(baudrates[i]);
			i++;
		}
	}
	if (hwSerial) {
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

int RESOLino_Pt1000::available() {
#if defined SOFTWARESERIAL_AVAILABLE
	if(swSerial) {
		return swSerial->available();
	} else {
		return hwSerial->available();
	}
#else
	hwSerial->available();
#endif
}

int RESOLino_Pt1000::readByte() {
#if defined SOFTWARESERIAL_AVAILABLE
	if(swSerial) {
		return swSerial->read();
	} else {
		return hwSerial->read();
	}
#else
	hwSerial->read();
#endif
}

int RESOLino_Pt1000::receiveData() {
	long timeout = millis();
	while (!available()) {
		if (millis() - timeout > RECEIVE_TIMEOUT_MS) {
			return -1;
		}
	}
	while (available()) {
		delay(1);
		byteReceived(readByte());
	}
	return 0;
}

void RESOLino_Pt1000::sendByte(uint8_t output) {
#if defined SOFTWARESERIAL_AVAILABLE
	if(swSerial) {
		swSerial->write(output);
	} else {
		hwSerial->write(output);
	}
#else
	hwSerial->write(output);
#endif
}

void RESOLino_Pt1000::datagramReceived(int16_t dst, int16_t src, int16_t cmd, uint16_t valueId, uint32_t value) {
	int idx = -1;
	for(int i = 0; i < 8; i++) {
		if (valueId == valueIDs[i]) {
			idx = i;
			break;
		}
	}
	if(idx > -1) {
		values[idx] = ((double)value) / 1000.0;
	}
}

void RESOLino_Pt1000::injectSeptett(uint8_t* buffer, uint16_t offset, uint16_t length, uint8_t septett) {
	uint16_t i;
	for (i = 0; i < length; i++) {
		if (septett & (1 << i)) {
			buffer[offset + i] |= 0x80;
		}
	}
}

uint8_t RESOLino_Pt1000::startTxCrc() {
	uint8_t crc;
	crc = 0xFF;
	return crc;
}

uint8_t RESOLino_Pt1000::calcTxCrc(uint8_t crc, uint8_t data) {
	crc -= data;
	return crc;
}

uint8_t RESOLino_Pt1000::endTxCrc(uint8_t crc) {
	crc = (crc & 0x7F);
	return crc;
}

int RESOLino_Pt1000::checkRxCrc(const uint8_t *buffer, int offset, int length, uint8_t rxProtocolSelector) {
	int result;
	if (rxProtocolSelector == 0) {
		uint8_t crc;
		int i;
		crc = 0xFF;
		for (i = 0; i < length; i++) {
			crc -= buffer[offset + i];
		}
		result = ((crc & 0x7F) == 0x00);
	} else {
		result = 0;
	}
	return result;
}

void RESOLino_Pt1000::byteReceived(uint8_t data) {
	int16_t  dst, src, cmd;
	uint16_t valueId;
	uint32_t value;
	if (data == 0xAA) {
		rxIndex = 0;
	} else if (data & 0x80) {
		rxIndex = -1;
	} else if (rxIndex >= (int)sizeof (rxBuffer)) {
		rxIndex = -1;
	}
	if (rxIndex >= 0) {
		rxBuffer [rxIndex++] = data;
		if ((rxIndex == 16) && ((rxBuffer [5] & 0xF0) == 0x20)) {
			if (checkRxCrc(rxBuffer, 1, 15, rxBuffer [5] & 0x0F)) {
				injectSeptett(rxBuffer, 8, 6, rxBuffer [14]);
				dst = ((uint16_t) rxBuffer [1]) | (((uint16_t) rxBuffer [2]) << 8);
				src = ((uint16_t) rxBuffer [3]) | (((uint16_t) rxBuffer [4]) << 8);
				cmd = ((uint16_t) rxBuffer [6]) | (((uint16_t) rxBuffer [7]) << 8);
				valueId = ((uint16_t) rxBuffer [8]) | (((uint16_t) rxBuffer [9]) << 8);
				value = ((uint32_t) rxBuffer [10]) | (((uint32_t) rxBuffer [11]) << 8) | (((uint32_t) rxBuffer [12]) << 16) | (((uint32_t) rxBuffer [13]) << 24);
				datagramReceived(dst, src, cmd, valueId, value);
				rxIndex = -1;
			} else {
				rxIndex = -1;
			}
		}
	}
}

void RESOLino_Pt1000::sendByteWithCrc(uint8_t data, uint8_t* crc) {
	*crc = calcTxCrc(*crc, data);
	sendByte(data);
}

void RESOLino_Pt1000::sendByteWithCrcAndSeptet(uint8_t data, uint8_t* crc, uint8_t* septetValue, uint8_t* septetMask) {
	if (data & 0x80) {
		*septetValue |= *septetMask;
	}
	*septetMask <<= 1;
	sendByteWithCrc(data & 0x7F, crc);
}


void RESOLino_Pt1000::sendDatagram(int16_t cmd, uint16_t valueId, uint32_t value) {
	uint8_t crc, septet, mask;
	crc = startTxCrc();
	septet = 0;
	mask = 1;
	sendByte(0xAA);
	sendByteWithCrc(RS_VBUS_SHIELD_ADDRESS, &crc);
	sendByteWithCrc(RS_VBUS_SHIELD_ADDRESS >> 8, &crc);
	sendByteWithCrc(RS_VBUS_SOURCE_ADDRESS, &crc);
	sendByteWithCrc(RS_VBUS_SOURCE_ADDRESS >> 8, &crc);
	sendByteWithCrc(0x20, &crc);
	sendByteWithCrc(cmd, &crc);
	sendByteWithCrc(cmd >> 8, &crc);
	sendByteWithCrcAndSeptet(valueId, &crc, &septet, &mask);
	sendByteWithCrcAndSeptet(valueId >> 8, &crc, &septet, &mask);
	sendByteWithCrcAndSeptet(value, &crc, &septet, &mask);
	sendByteWithCrcAndSeptet(value >> 8, &crc, &septet, &mask);
	sendByteWithCrcAndSeptet(value >> 16, &crc, &septet, &mask);
	sendByteWithCrcAndSeptet(value >> 24, &crc, &septet, &mask);
	sendByteWithCrc(septet, &crc);
	sendByte(endTxCrc(crc));
}