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
#include "Platform.h"

#undef DTAG
#define DTAG "<PLATFORM>\t"

volatile static U32 __Timer20ms = 0;
volatile static U32 __TicksTimer = 0;
volatile static U32 __PrecisionTimer;

static struct {
	U8 Timer1ms : 1;
	U8 Timer20ms : 1;
	U8 Timer1s : 1;
} __Flags;



ISR(TIMER1_COMPA_vect)
{
	OCR1A += PLATFORM_TIMER1_COMPA_COUNTER;
	__Flags.Timer1ms = 1;
	__PrecisionTimer += PLATFORM_TIMER1_COMPA_COUNTER;
	__TicksTimer++;
}



ISR(TIMER1_COMPB_vect)
{
	OCR1B += PLATFORM_TIMER1_COMPB_COUNTER;
	__Flags.Timer20ms = 1;
	__Timer20ms++;
	Application_Timer20msInterrupt();

	if ((__Timer20ms % 50) == 0) {
		__Flags.Timer1s = 1;
		Application_Timer1sInterrupt();
	}
}



U32 Platform_GetPrecisionTimer(void)
{
	U32 Base1, Base2;
	U16 Timer;

	do {
		Base1 = __PrecisionTimer;
		Timer = TCNT1;
		Base2 = __PrecisionTimer;
	} while (Base1 != Base2);
	Timer -= (U16) Base1;
	return (Base1 + Timer);
}



U32 Platform_GetPrecisionInterval(U32 Microseconds)
{
	return (Microseconds); // =>  1 uS / 1 MHz = 1
}



static void Platform_LoopHandler(void)
{

	if (__Flags.Timer1ms) {
		__Flags.Timer1ms = 0;
		// handle 1ms tasks
	}
	if (__Flags.Timer20ms) {
		__Flags.Timer20ms = 0;
		// handle 20ms tasks
		Input_Timer20msHandler();
	}
	if (__Flags.Timer1s) {
		__Flags.Timer1s = 0;
		// handle 1s tasks
	}
}



/**
 * Starts the platform's loop.
 *
 * The implementation is only necessary, if the application requested not to "UseThreading" while calling @ref Platform_Initialize().
 * This method should enter an infinite loop calling @ref Application_LoopHandler() on a regular basis.
 *
 * @note Implemented by Hardware Developer, called by Application Developer.
 */
RS_ERROR Platform_RunLoop(void)
{
	DPRINT(DTAG "RunLoop...\n");

	while (1) {
		Platform_LoopHandler();
		Input_LoopHandler();

		Application_LoopHandler();
	}

	return RS_OK;
}



/**
 * Initializes the platform.
 *
 * This method should do the following steps:
 *
 * - Disable interrupts
 * - Perform low-level initialization (CPU clocks, timers, port pins, etc.)
 * - Initialize the heap
 * - Initialize threading if applicable
 * - Call "Application_Initialize()"
 * - Enable interrupts
 *
 * @note Implemented by Hardware Developer, called by Application Developer asap.
 */
RS_ERROR Platform_Initialize(void)
{
	Rs_Hal_DisableInterrupts();

	//PORT A
	// Reserved

	//PORT B (ExtSpi, MUX)
	DDRB = 0xBF; //PB6 input
	PORTB = 0;

	//PORT C (VBus, SW-UART)
	DDRC = 0xFE; //PC0 input
	PORTC = 0;

	//PORT D (UART, DIP-IN, LED)
	DDRD = 0xC2; //PD1,PD6:7 out (uart, led)
	PORTD = 0xFC; // leds off, enable pull-ups for dip input

	TIMSK = 0; // clear all timer ints.

	//------ Timer 1 init (1 ms/ 20 ms Ticks)
	TCNT1 = 0;
	TCCR1A = 0;
	TCCR1B = 0;
	OCR1A = (U16) PLATFORM_TIMER1_COMPA_COUNTER;
	OCR1B = (U16) PLATFORM_TIMER1_COMPB_COUNTER;
	TIMSK |= ((1 << OCIE1A) | (1 << OCIE1B)); //enable timer compare match int.
	TCCR1B = (1 << CS11); // Prescaler 8 => 1 MHz

	Spi_Initialize();

	Application_Initialize();

	Rs_Hal_EnableInterrupts();

	DPRINT("Platform init.\n");

	//***** init ende ********

	return RS_OK;
}



int main(void)
{
	RS_ERROR Error;

	Error = Application_RunMain();

	return (Error == RS_OK) ? 0 : 1;
}
