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
#include "AdcAD7790.h"

#undef DTAG
#define DTAG "<ADC>\t"

#ifndef ADCAD7790_DEBUG
#define ADCAD7790_DEBUG 0
#endif

#if ADCAD7790_DEBUG

#define __DPRINTF DPRINTF
#define __DPRINT DPRINT

#else

#define __DPRINTF(x, ...)
#define __DPRINT(x)

#endif




ADCAD7790_STATE __AdcAD7790_State;

#define _HardwareInitialize						ADCAD7790_INFO_(HardwareInitialize)
#define _Reset									ADCAD7790_INFO_(Reset)
#define _SetMuxChannel							ADCAD7790_INFO_(SetMuxChannel)
#define _Enable									ADCAD7790_INFO_(Enable)
#define _Transceive								ADCAD7790_INFO_(Transceive)
#define _MeasurementComplete					ADCAD7790_INFO_(MeasurementComplete)
#define _ChannelValues							ADCAD7790_INFO_(ChannelValues)
#define _ChannelCount							ADCAD7790_INFO_(ChannelCount)

#define _NextPriorityChannel					ADCAD7790_STATE_(NextPriorityChannel)
#define _PriorityChannel						ADCAD7790_STATE_(PriorityChannel)
#define _NextChannel							ADCAD7790_STATE_(NextChannel)
#define _CurrentChannel							ADCAD7790_STATE_(CurrentChannel)
#define _Phase									ADCAD7790_STATE_(Phase)
#define _Timeout								ADCAD7790_STATE_(Timeout)
#define _Delay									ADCAD7790_STATE_(Delay)
#define _LastTimer								ADCAD7790_STATE_(LastTimer)


void AdcAD7790_LoopHandler(void)
{
	U32 Timer;
	I32 TimerDiff;
	U32 Delay;
	I32 Value;

#if ADCAD7790_DEBUG
	static U32 MeasStartTime, MeasEndTime;
#endif // ADCAD7790_DEBUG

	Timer = Platform_GetPrecisionTimer();
	TimerDiff = (I32) (Timer - _LastTimer);
	_LastTimer = Timer;

	if (_Delay > 0) {
		if (TimerDiff < 0) {
			TimerDiff = 0;
		}
		if (TimerDiff < _Delay) {
			_Delay -= TimerDiff;
			return;
		}
	}

	Delay = 0;
	if (_Phase == 0) { /* Select Channel and set multiplexer */
		if (_PriorityChannel >= 0) {
			_CurrentChannel = _PriorityChannel;
		} else if (_NextPriorityChannel >= 0) {
			_CurrentChannel = _PriorityChannel = _NextPriorityChannel;
			_NextPriorityChannel = -1;
		} else {
			_CurrentChannel = _NextChannel;
		}

		// select channel and wait
		Delay = _SetMuxChannel(_CurrentChannel);
		_Phase = 1;

#if ADCAD7790_DEBUG
		MeasStartTime = Timer;
#endif

	} else if (_Phase == 1) { /* Trigger conversation */
		// trigger conversion
		_Enable(1);
		_Transceive(0x10, 0);		// Choose Mode Register Selection
		_Transceive(0x82, 0);		// Set Single Conversion Mode and Buffered Mode
		_Enable(0);
		_Phase = 2;
		_Timeout = 20;
	} else if (_Phase == 2) {
		_Enable(1);
		Value = _Transceive(0xFF, 1);
		_Enable(0);
		if (Value == 0) {
			// get conversion result and start next one
			_Enable(1);
			_Transceive(0x38, 0);	// Set Read from Data Register
			Value = (I32) _Transceive(0xFF, 0) << 8;	// Read Value
			Value |= (I32) _Transceive(0xFF, 0);		// Read Value
			_Enable(0);

#if ADCAD7790_DEBUG
			MeasEndTime = Timer;
			__DPRINTF(DTAG "[%d]: StartTime %ld, EndTime %ld, DiffTime %ld, Value %ld.\n", _CurrentChannel, MeasStartTime, MeasEndTime, (MeasEndTime - MeasStartTime), Value);
#endif

			// store result
			_ChannelValues [_CurrentChannel] = Value;

			// find next channel
			if (_CurrentChannel == _PriorityChannel) {
				_PriorityChannel = -1;
			} else if (++_NextChannel >= _ChannelCount) {
				_NextChannel = 0;
			}

			// report back complete measurement cycle
			_MeasurementComplete(_CurrentChannel);

			_Phase = 0;
		} else if (_Timeout > 0) {
			_Timeout--;
			Delay = 10000;
		} else {
			_Phase = 4;
		}
	} else if (_Phase == 4) {
		_Reset();

		_Phase = 0;
	} else {
		_Phase = 0;
	}

	if (Delay > 0) {
		_Delay = Platform_GetPrecisionInterval(Delay);
	}
}



void AdcAD7790_Initialize(void)
{
	_NextPriorityChannel = -1;
	_PriorityChannel = -1;
	_NextChannel = 0;
	_CurrentChannel = 0;
	_Phase = 4;
	_Timeout = 0;
	_Delay = 0;
	_LastTimer = Platform_GetPrecisionTimer();

	_HardwareInitialize();
}



#undef ADCAD7790_DEBUG
