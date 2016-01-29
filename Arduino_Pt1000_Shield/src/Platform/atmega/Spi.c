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
#include "Spi.h"

#ifndef SPI_DISABLE_TIMEOUT
#define SPI_DISABLE_TIMEOUT 0
#endif


#if !SPI_HARDWARE_SPI

// software spi
unsigned char Spi_Transmit(unsigned char Output)
{
	unsigned char Mask, Input;

	Input = 0;

	for (Mask = 0x80; Mask > 0; Mask >>= 1) {
		// Data out
		if (Output & Mask) {
			SET_BIT(SWSPI_MOSI_PORT, SWSPI_MOSI_PIN);
		} else {
			CLEAR_BIT(SWSPI_MOSI_PORT, SWSPI_MOSI_PIN);
		}

		// Clock
		SET_BIT(SWSPI_CLK_PORT, SWSPI_CLK_PIN);

		// Data in
		if (SWSPI_MISO_PIN) {
			Input |= Mask;
		}

		// Clock
		CLEAR_BIT(SWSPI_CLK_PORT, SWSPI_CLK_PIN);
	}

	return Input;
}

#else

static volatile U8 __Input, __TxDone;



// spi transmit complete interrupt
ISR(SPI_STC_vect)
{
	__Input = SPDR;
	__TxDone = 1;
}



U8 Spi_Transceive(U8 Output)
{
	__Input = 0xFF;
	__TxDone = 0;

	SPDR = Output;

#if SPI_DISABLE_TIMEOUT
	// wait for transmit
	while (__TxDone == 0) {
		asm("nop");
	}

#else
	U16 CurrentTimer = TCNT1;

	// wait for transmit
	while ((I16) (TCNT1 - CurrentTimer) < 10000) {
		if (__TxDone != 0) {
			break;
		}
	}
#endif

#ifdef DEBUG
	if (__TxDone == 0) {
		DPRINTF("\n\n\n<SPI>\t!!!! ERROR: Tx Timeout (Out 0x%X, In 0x%X). !!!!\n\n\n", Output, __Input);
	}
#endif // DEBUG

	return __Input;
}

#endif // SPI_HARDWARE_SPI


void Spi_Initialize(void)
{

#if SPI_DISABLE_TIMEOUT
	DPRINT("<SPI>\tTimout disabled!\n");
#endif

	//SPI
#if SPI_HARDWARE_SPI
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA) | (1 << SPIE); //Enable SPI, Master, clock rate fck/4, CPOL 1, CPHA 1 (SPI Mode 3)
#endif
}
