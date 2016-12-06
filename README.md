# RESOLino_Pt1000

An Arduino library and shield firmware to measure Pt1000 temperature sensors using RESOL's shield.


## Features

- Shield measures resistors of up to 8 inputs
- Transfers measurement values to Arduino over serial port
- Library allows conversion to Pt1000 temperature values



## Using the Library

### Serial Connection

SoftwareSerial works directly on Arduino Uno and Yun and Mega. For Arduino Due and Genuino ZERO there is currently no SoftwareSerial library available. So HardwareSerial must be used on these boards and in RESOLino_PT1000.h the line #define SOFTWARESERIAL_AVAILABLE must be deleted or commented out.


### Example

[RESOLino_Pt1000/Example/Example.ino](RESOLino_Pt1000/Example/Example.ino)



## Building your own firmware

1. Make sure the avr-gcc toolchain is reachable

		$ avr-gcc -v
		Using built-in specs.
		COLLECT_GCC=avr-gcc
		COLLECT_LTO_WRAPPER=/Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/../libexec/gcc/avr/4.8.1/lto-wrapper
		Target: avr
		Configured with: ../gcc-4.8.1/configure --enable-fixed-point --enable-languages=c,c++ --prefix=/Users/jenkins/jenkins/workspace/toolchain-avr-mac32/objdir --enable-long-long --disable-nls --disable-checking --disable-libssp --disable-libada --disable-shared --enable-lto --with-avrlibc=yes --with-dwarf2 --disable-doc --target=avr
		Thread model: single
		gcc version 4.8.1 (GCC)


2. Clone the repository

		$ cd <working dir>
		$ git clone https://github.com/resol-de/RESOLino_Pt1000.git


3. Build the firmware

		$ cd <working dir>/RESOLino_Pt1000/Arduino_Pt1000_Shield
		$ make


4. Flash the firmware

	TBD: - change AVRDUDE_CONF_FILE if not on OS X

		$ cd <working dir>/RESOLino_Pt1000/Arduino_Pt1000_Shield
		$ make flash


## Changes

### 0.1-SNAPSHOT

- Initial project release



## Contributors

- The RESOL VBus Development Team <vbus@resol.de>



## Links

- RESOLino_Pt1000 Shield: [Buy at amazon](https://www.amazon.de/RESOL-73041-RESOLino-Pt1000-Shield/dp/B01IVSCTS8)
- GitHub Repository: [https://github.com/resol-de/RESOLino_Pt1000](https://github.com/resol-de/RESOLino_Pt1000)



## License

Copyright (C) 2014-2016, RESOL - Elektronische Regelungen GmbH.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
