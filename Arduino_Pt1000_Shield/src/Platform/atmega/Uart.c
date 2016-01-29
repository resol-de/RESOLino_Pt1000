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
#include "Uart.h"

#undef DTAG
#define DTAG "<UART>\t"

#ifndef UART_DEBUG
#define UART_DEBUG 0
#endif

#if UART_DEBUG

#define __DPRINTF DPRINTF
#define __DPRINT DPRINT

#else

#define __DPRINTF(x, ...)
#define __DPRINT(x)

#endif

//------------------
// Uart0: Arduino
//--------------------

static RS_RINGBUFFER_STATE* __Uart_TxBuffer = 0;
static U8 __Uart_IsTransmitting = 0;



RS_ERROR Uart0_Initialize(RS_RINGBUFFER_STATE *TxBuffer, U32 Baudrate)
{
	U16 BaudrateReg;
	U8 DoubleSpeed = 0;

	UCSRB = 0;

	/*Set baud rate */
	if (Baudrate >= 57600UL) {
		// at FOSC = 8MHz, the baudrate error is to high at baudrate >= 57600 in normal speed mode
		DoubleSpeed = 1;
	}
	if (DoubleSpeed) {
		BaudrateReg = (PLATFORM_FREQ / 8UL / Baudrate - 1);
	} else {
		BaudrateReg = (PLATFORM_FREQ / 16UL / Baudrate - 1);
	}
	UBRRH = (unsigned char) (BaudrateReg >> 8) & 0xFF;
	UBRRL = (unsigned char) BaudrateReg & 0xFF;

	if (DoubleSpeed) {
		UCSRA = 2; // enable double speed mode
	} else {
		UCSRA = 0; // disable double speed mode (normal mode)
	}

	/* Set frame format: 8data, 1stop bit, No parity */
	UCSRC = (1 << URSEL) | (3 << UCSZ0);

	/*Enable receiver and transmitter and RX complete Interrupt*/
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);

	__Uart_TxBuffer = TxBuffer;

	__DPRINTF(DTAG "Init. Baud %lu, UBBR %d.\n", Baudrate, BaudrateReg);


	return RS_OK;
}



ISR(USART_RXC_vect)
{
	uint8_t data, status;

	status = SREG;	// save status register
	CLEAR_BIT(PORTD, 7); // Rx LED on

	data = UDR;

	Application_Uart_DidReceive(0, data);

	SREG = status;   // restore status register
	SET_BIT(PORTD, 7); // Rx LED off
}



void Uart0_Putc(uint8_t c)
{
	CLEAR_BIT(PORTD, 6); // Tx LED on

	U16 timeout = 10000;

    // wait for UDR ready
    while (!(UCSRA & (1 << UDRE))) {
    	if (--timeout == 0) {
    		return;
    	}
    }

    // write UDR starts the transm.
    UDR = c;
    SET_BIT(PORTD, 6); // Tx LED off
}



static RS_ERROR __Uart0_FlushTxBuffer(int Block)
{
	U8 data = 0;

//	__DPRINTF(DTAG "1: FlushBuffer. Length %d...", length);

	if (__Uart_TxBuffer != 0) {
		while (Rs_RingBuffer_Read(__Uart_TxBuffer, &data, 1) != 0) {
			__Uart_IsTransmitting = 1;
			Uart0_Putc(data);
		}
	}

	__Uart_IsTransmitting = 0;

//	__DPRINTF(" Done. Timeouts: %d, ", __U1Timeouts);
//	__DPRINTF("TimeoutCounter: %d.\n", __U1TimeoutCounter);

	return RS_OK;
}




//---------------------
// Platform API
//----------------------

RS_ERROR Uart_SetBaud(int Index, U32 Baud)
{
	if (Index == 0) {
		if (Baud < 2400 || Baud > 115200) {
			return RS_ERROR_INVALIDARGUMENT;
		}
		Rs_Hal_DisableInterrupts();
		Uart_Reset(0, 1);
		Uart0_Initialize(__Uart_TxBuffer, Baud);
		Rs_Hal_EnableInterrupts();

		return RS_OK;
	} else {
		return RS_ERROR_INDEXOUTOFBOUNDS;
	}
}



RS_ERROR Uart_Reset(int Index, int Reset)
{
	RS_ERROR Result = RS_OK;

	if (Index == 0) {
		Rs_RingBuffer_Reset(__Uart_TxBuffer);
	} else {
		Result = RS_ERROR_INDEXOUTOFBOUNDS;
	}

	return Result;
}



RS_ERROR Uart_SetEnable(int Index, int Enable)
{
	//TODO:
	return RS_OK;
}



RS_RINGBUFFER_STATE* Uart_GetTxBuffer(int Index)
{
	if (Index == 0) {
		return __Uart_TxBuffer;
	} else {
		return (void *) 0;
	}
}



RS_ERROR Uart_FlushTxBuffer(int Index, int Block)
{

	if (Index == 0) {
		return __Uart0_FlushTxBuffer(Block);
	} else {
		return RS_ERROR_INDEXOUTOFBOUNDS;
	}
}




RS_ERROR Uart_Initialize(int Count, const UART_TYPE *Types, RS_RINGBUFFER_STATE **TxBuffers)
{
	RS_ERROR Error = RS_OK;

	__DPRINTF(DTAG "Init: Count %d, Type[0]: 0x%X\n", Count, Types[0]);

	if (Types[0] == UART_TYPE_VBUS) {
		Error = Uart0_Initialize(TxBuffers[0], Types[0] & 0xFFFFF);
	} else {
		Error = RS_ERROR_INVALIDARGUMENT;
	}

	__DPRINT(DTAG "Init done.\n");

	return Error;
}


#undef UART_DEBUG
