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

/*
 * RESOLino_Pt1000.h
 *
 *  Created on: 11.06.2014
 *      Author: andrek
 *
 *	This library enables you to read resistance values from temperature dependent
 *  resistors just like Pt100 - Pt1000 or KTY which are attached to a Temperature
 *  Measurement Shield from RESOL - Elektronische Regelungen GmbH.
 *
 *  Shield connection and initialization:
 *
 *  The serial shield communication can be done over hardware or software serial.
 *  You simply provide the desired serial object to the constructor while instantiating
 *  a library object:
 *
 *  RESOLino_Pt1000 Pt1000(&Serial) or RESOLino_Pt1000 Pt1000(&SoftSerial)
 *
 *  !!When using hardware serial, to avoid data confusions you should not use the
 *  same serial line for other operations!!
 *
 *	Please call the init-function from your shield object once within your setup routine.
 *
 *
 *  Read and convert values:
 *
 *  To read sensor values call the update-function with the number of the terminal your
 *  sensor is connected to, a pointer to a variable where the read value will be stored
 *  in and the desired value conversion, depending from the sensor type.
 *
 *  If you choose custom conversion, then you have to additionally provide a pointer
 *  to your own conversion function which needs to have the following form:
 *
 *	double YourFunc(double raw)
 *
 *  Otherwise write NULL instead of the pointer.
 *
 *
 *  Set custom baudrate:
 *
 *  The default baudrate is 9600 Bd. You can set rates between 2400 and 115200 Bd, while
 *  the recommended value is 38400 Bd. With this rate, reading one value takes around 6 ms
 *  on an Arduino Uno.
 *
 *  Rates above 38400 seemed to produce too many errors.
 *
 *  !!Keep in mind, that changing the baudrate with the shield connected to hardware serial
 *  may affect other parts of your program!!
 *
 *  Following restrictions to the use of the serial interfaces are known:
 *
 *  Using Arduino Uno with HardwareSerial a maximum Baudrate of 57600 gives correct values.
 *  Using Arduino Uno with SoftwareSerial a maximum Baudrate of 19200 gives correct values.
 *  Using Arduino Mega with HardwareSerial a maximum Baudrate of 57600 gives correct values.
 *  Using Arduino Mega with SoftwareSerial seems to do not work correctly.
 *  Using Arduino Yun with HardwareSerial is not working because the pins 0 and 1 are used to communicate with the Linux system.
 *  Using Arduino Yun with SoftwareSerial a maximum Baudrate of 19200 gives correct values.
 *
 *
 *
 *  Set priority terminal:
 *
 *  By default all terminal values get refreshed one after another which leads to a refresh
 *  rate of about 1 Hz for each terminal. If this is too slow for a certain purpose,
 *  a priority terminal between 1 and 8 can be set.
 *
 *  The priority terminal gets then refreshed every second time, which gives a refresh rate
 *  of about 4 Hz for this particular terminal, but slows down the rate of the other ones.
 *
 *  Setting a priority terminal of -1 disables the function.
 */

#ifndef _RESOLino_PT1000_h
#define _RESOLino_PT1000_h

#define SOFTWARESERIAL_AVAILABLE

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#if defined SOFTWARESERIAL_AVAILABLE
	#include "SoftwareSerial.h"
#endif

#define RS_VBUS_BAUD_ID 0x0001
#define RS_VBUS_AD_SPEED_ID 0x0002
#define RS_VBUS_PRIORITY_ID 0x100A
#define RS_VBUS_SHIELD_ADDRESS 0x7770
#define RS_VBUS_SOURCE_ADDRESS 0x0020

#define RECEIVE_TIMEOUT_MS 100 //ms
#define MAX_RAW_VALUE 10000000

class RESOLino_Pt1000 {
public: // public attributes
	typedef enum AD_OPERAING_SPEED_t {HZ16 = 0x04, HZ20 = 0x03, HZ33 = 0x02, HZ100 = 0x01, HZ120 = 0x00} AD_OPERAING_SPEED;

	static const uint8_t Conversion_RawOhm = 0;
	static const uint8_t Conversion_Pt100_To_DegreeCentigrade = 1;
	static const uint8_t Conversion_Pt500_To_DegreeCentigrade = 2;
	static const uint8_t Conversion_Pt1000_To_DegreeCentigrade = 3;
	static const uint8_t Conversion_Pt100_To_DegreeFahrenheit = 4;
	static const uint8_t Conversion_Pt500_To_DegreeFahrenheit = 5;
	static const uint8_t Conversion_Pt1000_To_DegreeFahrenheit = 6;
	static const uint8_t Conversion_CustomFunction = 7;

public: // public methods
#if defined SOFTWARESERIAL_AVAILABLE
	RESOLino_Pt1000(SoftwareSerial *serial);
#endif
	RESOLino_Pt1000(HardwareSerial *serial);

	void init(uint32_t baudrate = 9600);
	void setPriorityTerminal(int32_t terminal);
	void setADOperatingSpeed(AD_OPERAING_SPEED hz);
	void update(int sensorNr, double *value, uint8_t conversion, double (*customConversionFunction)(double rawOhm));

private: // private attributes
	HardwareSerial *hwSerial;
#if defined SOFTWARESERIAL_AVAILABLE
	SoftwareSerial *swSerial;
#endif
	uint8_t rxIndex;
	int8_t rxFrameNr;
	uint8_t rxBuffer[17];
	double values[8];

	const uint16_t valueIDs[8] = {
		0x1001,
		0x1002,
		0x1003,
		0x1004,
		0x1005,
		0x1006,
		0x1007,
		0x1008
	};

private: // private methods
	void begin(uint32_t baud);
	void setBaudrate(uint32_t baud);
	
	int available();
	int readByte();
	int receiveData();
	
	void sendByte(uint8_t output);

	void datagramReceived(int16_t dst, int16_t src, int16_t cmd, uint16_t valueId, uint32_t value);
	void injectSeptett(uint8_t* buffer, uint16_t offset, uint16_t length, uint8_t septett);

	uint8_t startTxCrc();
	uint8_t calcTxCrc(uint8_t crc, uint8_t data);
	uint8_t endTxCrc(uint8_t crc);
	int checkRxCrc(const uint8_t *buffer, int offset, int length, uint8_t rxProtocolSelector);

	void byteReceived(uint8_t data);

	void sendByteWithCrc(uint8_t data, uint8_t* crc);
	void sendByteWithCrcAndSeptet(uint8_t data, uint8_t* crc, uint8_t* septetValue, uint8_t* septetMask);
	void sendDatagram(int16_t cmd, uint16_t valueId, uint32_t value);
};

extern RESOLino_Pt1000 RESOLino_PT1000;

#endif