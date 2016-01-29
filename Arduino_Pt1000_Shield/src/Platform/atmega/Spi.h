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
#ifndef SPI_H_
#define SPI_H_

#include "Platform.h"

#ifndef SPI_HARDWARE_SPI
#define SPI_HARDWARE_SPI 1
#endif // SPI_HARDWARE_SPI


// software spi pin
#define SWSPI_MOSI_PORT (PORTB)
#define SWSPI_MOSI_PIN	(PB5)

#define SWSPI_MISO_PORT (PORTB)
#define SWSPI_MISO_PIN	(PB6)

#define SWSPI_CLK_PORT (PORTB)
#define SWSPI_CLK_PIN	(PB7)


#ifndef SPI_ADC_ENABLE
#define SPI_ADC_ENABLE() CLEAR_BIT(PORTB, 4)
#endif

#ifndef SPI_ADC_DISABLE
#define SPI_ADC_DISABLE() SET_BIT(PORTB, 4)
#endif

#ifndef SPI_ADC_READY
#define SPI_ADC_READY (PINB & (1 << SWSPI_MISO_PIN))
#endif



U8 Spi_Transceive(U8 Output);
void Spi_Initialize(void);

#endif /* SPI_H_ */
