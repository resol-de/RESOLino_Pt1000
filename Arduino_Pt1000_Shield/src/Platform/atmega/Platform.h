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
#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdint.h>
#include <avr/common.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <string.h>
#include <avr/interrupt.h>

#include "PlatformApi.h"

#include "Values.h"
#include "Uart.h"
#include "Spi.h"
#include "Console.h"
#include "AdcAD7790.h"
#include "Input.h"


#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#define PLATFORM_FREQ						F_CPU

#define PLATFORM_TIMER1_FREQ				(PLATFORM_FREQ / 8UL)
#define PLATFORM_TIMER1_COMPA_FREQ			1000UL /* 1ms (1/1000)s*/
#define PLATFORM_TIMER1_COMPA_COUNTER		(PLATFORM_TIMER1_FREQ / PLATFORM_TIMER1_COMPA_FREQ)
#define PLATFORM_TIMER1_COMPB_FREQ			50UL /* 20ms (1/50)s*/
#define PLATFORM_TIMER1_COMPB_COUNTER		(PLATFORM_TIMER1_FREQ / PLATFORM_TIMER1_COMPB_FREQ)

#define PLATFORM_UART_VBUS_BAUDRATE			9600UL
#define PLATFORM_UART_VBUS_COUNTER			(PLATFORM_FREQ / 16UL / PLATFORM_UART_VBUS_BAUDRATE - 1)
#define PLATFORM_UART_DEBUG_BAUDRATE		115200UL
#define PLATFORM_UART_DEBUG_COUNTER			(PLATFORM_FREQ / 16UL / PLATFORM_UART_DEBUG_BAUDRATE - 1)

#define SET_BIT(PORT, BITNUM)    ((PORT) |=  (1 << (BITNUM)))
#define CLEAR_BIT(PORT, BITNUM)  ((PORT) &= ~(1 << (BITNUM)))
#define TOGGLE_BIT(PORT, BITNUM) ((PORT) ^=  (1 << (BITNUM)))



#endif /* PLATFORM_H_ */
