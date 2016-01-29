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
#ifndef PLATFORMAPI_H_
#define PLATFORMAPI_H_


#ifndef PLATFORM_USE_PROGMEM
#define PLATFORM_USE_PROGMEM 0
#endif

#if PLATFORM_USE_PROGMEM
// Place const types in flash (program memory)
#include <avr/pgmspace.h>

#define RS_PLATFORM_CONST PROGMEM const

// Remap Platform-API functions with const args to progmem variants

extern RS_ERROR Uart_Initialize_P(int Count, const UART_TYPE *Types, RS_RINGBUFFER_STATE **TxBuffers);
#undef Uart_Initialize
#define Uart_Initialize Uart_Initialize_P

#else

#define RS_PLATFORM_CONST const

#endif // PLATFORM_USE_PROGMEM




#if PLATFORM_USE_PROGMEM

#define Platform_ReadConstChar(p) ((char) pgm_read_byte((p)))
#define Platform_ReadConstI8(p) ((I8) pgm_read_byte((p)))
#define Platform_ReadConstU8(p) ((U8) pgm_read_byte((p)))

#define Platform_ReadConstShort(p) ((short) pgm_read_word((p)))
#define Platform_ReadConstI16(p) ((I16) pgm_read_word((p)))
#define Platform_ReadConstU16(p) ((U16) pgm_read_word((p)))

#define Platform_ReadConstInt(p) pgm_read_word((p))
#define Platform_ReadConstI32(p) ((I32) pgm_read_dword((p)))
#define Platform_ReadConstU32(p) ((U32) pgm_read_dword((p)))

#else // !PLATFORM_USE_PROGMEM


#define Platform_ReadConstChar(p) ((char) (*(p)))
#define Platform_ReadConstI8(p) ((I8) (*(p)))
#define Platform_ReadConstU8(p) ((U8) (*(p)))

#define Platform_ReadConstShort(p) ((short) (*(p)))
#define Platform_ReadConstI16(p) ((I16) (*(p)))
#define Platform_ReadConstU16(p) ((U16) (*(p)))

#define Platform_ReadConstInt(p) ((int) (*(p)))
#define Platform_ReadConstI32(p) ((I32) (*(p)))
#define Platform_ReadConstU32(p) ((U32) (*(p)))

#endif // PLATFORM_USE_PROGMEM


#ifndef countof
#define countof(x) (sizeof (x) / sizeof ((x) [0]))
#endif

typedef signed char I8;
typedef unsigned char U8;
typedef signed short I16;
typedef unsigned short U16;
typedef signed long I32;
typedef unsigned long U32;
typedef unsigned int UINT;
typedef signed int IPTR;

#include "RsRingBuffer.h"



/***************************************************************************
 * ERROR
 ***************************************************************************/
/**
 * RS_ERROR enumeration.
 *
 * @note Error codes must not be less than -32768 to ensure compatibility to 16-bit CPUs
 */
enum {
	RS_OK = 0,
	RS_WOULDBLOCK,
	RS_NOTREDUNDANT,

	RS_NOTFOUND = -1,

	// general purpose errors
	RS_ERROR_UNKNOWN = -1024,
	RS_ERROR_NULLPOINTER,
	RS_ERROR_OUTOFMEMORY,
	RS_ERROR_NOTIMPLEMENTED,
	RS_ERROR_INDEXOUTOFBOUNDS,
	RS_ERROR_THREADINGNOTSUPPORTED,
	RS_ERROR_INVALIDARGUMENT,
};

typedef int RS_ERROR;


/**
 * Disable the interrupts.
 */
void Rs_Hal_DisableInterrupts(void);

/**
 * Enable interrupts.
 */
void Rs_Hal_EnableInterrupts(void);


/**
 * Return the platform's ticks timer value.
 *
 * This value is an arbitrary free-running counter. This counter must conform to the following rules:
 *
 * - It's resolution is 20 ms, resulting in a time period greater than 2 years before an integer overflow occurs.
 * - It's steadily increasing. Care must be taken that carry-overs do not result in unsteady behaviour.
 */
/*HWDEV*/ U32 Platform_GetTicksTimer(void);

/**
 * Returns the platform's precision timer value.
 *
 * This value is an arbitrary free-running counter. This counter must conform to the following rules:
 *
 * - It's maximum resolution is 10 ns, resulting in a time period greater than 40s until an integer overflow occurs.
 * - It's steadily increasing. Care must be taken that carry-overs do not result in unsteady behaviour.
 *
 * @returns Precision timer value.
 *
 * @see Platform_GetPrecisionInterval
 */
/*HWDEV*/ U32 Platform_GetPrecisionTimer(void);

/**
 * Returns the platform's precision timer interval value for the specified amount of microseconds.
 *
 * Implementation must guarantee that the execution of the following code always takes longer than the specified amount of microseconds:
 *
 * @code
 *  void SleepWithPlatformPrecisionTimer(U32 Microseconds)
 *  {
 *  	U32 CurrentTimer, TimeoutTimer;
 *  	I32 TimerDiff;
 *
 * 		CurrentTimer = Platform_GetPrecisionTimer();
 * 		TimeoutTimer = CurrentTimer + Platform_GetPrecisionInterval(Microseconds);
 * 		do {
 * 			CurrentTimer = Platform_GetPrecisionTimer();
 * 			TimerDiff = (I32) (TimeoutTimer - CurrentTimer();
 * 		} while (TimerDiff > 0);
 *  }
 * @endcode
 *
 * @param Microseconds Amount of microseconds to get interval value for. Must not exceed 40,000,000.
 * @returns Precision timer interval value for the requested amount of microseconds.
 */
/*HWDEV*/ U32 Platform_GetPrecisionInterval(U32 Microseconds);

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
/*HWDEV*/ RS_ERROR Platform_Initialize(void);

/**
 * Starts the platform's loop.
 *
 * The implementation is only necessary, if the application requested not to "UseThreading" while calling @ref Platform_Initialize().
 * This method should enter an infinite loop calling @ref Application_LoopHandler() on a regular basis.
 *
 * @note Implemented by Hardware Developer, called by Application Developer.
 */
/*HWDEV*/ RS_ERROR Platform_RunLoop(void);

/**
 * Starts the platform's threading scheduler.
 *
 * The implementation is only necessary if the application requested to "UseThreading" while calling @ref Platform_Initialize().
 * The implementation should call @ref RS_Threading_RunScheduler().
 *
 * @note Implemented by Hardware Developer, called by Application Developer.
 */
/*HWDEV*/ RS_ERROR Platform_RunScheduler(void);

/**
 * Enables the platform's watchdog timer. Once enabled it cannot be disabled until the next reset occurs.
 *
 * @note Implemented by Hardware Developer, called by Application Developer.
 */
/*HWDEV*/ RS_ERROR Platform_EnableWatchdog(void);

/**
 * Resets the platform's watchdog timer.
 *
 * Maximum delay between two calls: 100ms
 *
 * @note Implemented by Hardware Developer, called by Application Developer.
 */
/*HWDEV*/ RS_ERROR Platform_ResetWatchdog(void);



/****************************************************************************
 * APPLICATION
 ***************************************************************************/
/**
 * Initializes the application.
 *
 * @returns RS_OK if initialization was successful, otherwise RS_ERROR code
 *
 * @note Implemented by Application Developer, called by @ref Platform_Initialize().
 */
/*APPDEV*/ RS_ERROR Application_Initialize(void);

/**
 * Interrupts are disabled during this funcation call.
 *
 * @note Implemented by Application Developer, called by Hardware Developer.
 */
/*APPDEV*/ RS_ERROR Application_Timer5msInterrupt(void);

/**
 * Interrupts are disabled during this funcation call.
 *
 * @note Implemented by Application Developer, called by Hardware Developer.
 */
/*APPDEV*/ RS_ERROR Application_Timer20msInterrupt(void);

/**
 * Interrupts are disabled during this function call.
 *
 * @note Implemented by Application Developer, called by Hardware Developer.
 */
/*APPDEV*/ RS_ERROR Application_Timer1sInterrupt(void);


/**
 * The implementation is only necessary, if the application requested not to "UseThreading" while calling @ref Platform_Initialize().
 *
 * @note Implemented by Application Developer, called by Hardware Developer.
 */
/*APPDEV*/ RS_ERROR Application_LoopHandler(void);

/**
 *
 */
/*APPDEV*/ RS_ERROR Application_RunMain(void);



/****************************************************************************
 * INPUT
 ***************************************************************************/
typedef enum __INPUT_OUTPUT_TYPE {
	/**
	 * I/O type "Raw": Returns the raw value of the corresponding I/O channel.
	 */
	INPUT_OUTPUT_TYPE_RAW = 1,

	/**
	 * I/O type "Digital": can be used for switches, GPIO etc.
	 */
	INPUT_OUTPUT_TYPE_DIGITAL = 2,

	/**
	 * I/O type "Voltage": Resolution: 1e-6 V
	 */
	INPUT_OUTPUT_TYPE_VOLTAGE = 4,

	/**
	 * I/O type "Current": Resoltion 1e-6 A
	 */
	INPUT_OUTPUT_TYPE_CURRENT = 16,

	/**
	 * I/O type "Resistor": Resolution 1e-3 Ohm
	 */
	INPUT_OUTPUT_TYPE_RESISTOR = 32,

	/**
	 * I/O type "Counter"
	 */
	INPUT_OUTPUT_TYPE_COUNTER = 64,

	/**
	 * I/O type "Frequency": Resolution 1e-3 Hz
	 */
	INPUT_OUTPUT_TYPE_FREQUENCY = 128,

	/**
	 * I/O type "Duty": Resolution 1e-3 %
	 */
	INPUT_OUTPUT_TYPE_DUTYCYCLE = 256,

	/**
	 * I/O type "Temperature": Resolution 1e-3 �C
	 */
	INPUT_OUTPUT_TYPE_TEMPERATURE = 512,

	/**
	 * I/O type "Irradiation": Resolution 1e-3 W/m^2
	 */
	INPUT_OUTPUT_TYPE_IRRADIATION = 1024,

	/**
	 * I/O type "Pressure": Resolution 1e-6 bar
	*/
	INPUT_OUTPUT_TYPE_PRESSURE = 2048,

	/**
	* I/O type "Flow": Resolution 1e-3 l/min
	*/
	INPUT_OUTPUT_TYPE_FLOW = 4096,

	/**
	* I/O type "Periodic time": Resolution 1e-3 s
	*/
	INPUT_OUTPUT_TYPE_PERIODICTIME = 8192,
} INPUT_OUTPUT_TYPE;



/**
 * Initializes the inputs.
 *
 * @note Implemented by Hardware Developer, called by @ref Application_Initialize().
 */
/*HWDEV*/ RS_ERROR Input_Initialize(int Count, const INPUT_OUTPUT_TYPE* Types);

/**
 * Get the input's current value.
 *
 * @note Implemented by Hardware Developer, called by Application Developer.
 */
/*HWDEV*/ RS_ERROR Input_GetInputValueForType(int Index, INPUT_OUTPUT_TYPE Type, I32* pValue);

/*HWDEV*/ RS_ERROR Input_GetControlValue(int Index, int Control, I32* pValue);

/*HWDEV*/ RS_ERROR Input_SetControlValue(int Index, int Control, I32 Value);



/****************************************************************************
 * UART
 ***************************************************************************/
#define UART_TYPE_1200BAUD						((UART_TYPE) 1200)
#define UART_TYPE_9600BAUD						((UART_TYPE) 9600)
#define UART_TYPE_19200BAUD						((UART_TYPE) 19200)
#define UART_TYPE_38400BAUD						((UART_TYPE) 38400)
#define UART_TYPE_115200BAUD					((UART_TYPE) 115200)
#define UART_TYPE_7DATA							((UART_TYPE) 0x00100000)
#define UART_TYPE_8DATA							((UART_TYPE) 0x00000000)
#define UART_TYPE_1STOP							((UART_TYPE) 0x00000000)
#define UART_TYPE_2STOP							((UART_TYPE) 0x00200000)
#define UART_TYPE_NOPARITY						((UART_TYPE) 0x00000000)
#define UART_TYPE_EVENPARITY					((UART_TYPE) 0x01000000)
#define UART_TYPE_ODDPARITY						((UART_TYPE) 0x02000000)

#define UART_TYPE_VBUS							(UART_TYPE_9600BAUD | UART_TYPE_8DATA | UART_TYPE_1STOP | UART_TYPE_NOPARITY)
#define UART_TYPE_CONSOLE						(UART_TYPE_115200BAUD | UART_TYPE_8DATA | UART_TYPE_1STOP | UART_TYPE_NOPARITY)

typedef U32 UART_TYPE;


/*HWDEV*/ RS_ERROR Uart_Initialize(int Count, const UART_TYPE *Types, RS_RINGBUFFER_STATE **TxBuffers);


/*HWDEV*/ RS_RINGBUFFER_STATE* Uart_GetTxBuffer(int Index);
/*HWDEV*/ RS_ERROR Uart_FlushTxBuffer(int Index, int Block);
/*HWDEV*/ RS_ERROR Uart_SetEnable(int Index, int Enable);
/*HWDEV*/ RS_ERROR Uart_SetBaud(int Index, U32 Baud);
/*HWDEV*/ RS_ERROR Uart_Reset(int Index, int Reset);
/*APPDEV*/ RS_ERROR Application_Uart_DidReceive(int Index, int Received);



//===========================================================================
// VBUS
//===========================================================================
/**
 * @defgroup VBus
 * @ingroup Network
 * @{
 * @author RESOL
 *
 */


typedef struct __RS_VBUS_INFO
{
	void (*HardwareInitialize)(void);
	void (*PacketReceived)(I16 Dst, I16 Src, I16 Cmd, I8 FrameCount, I8 FrameNr, const U8* Data);
	void (*DatagramReceived)(I16 Dst, I16 Src, I16 Cmd, U16 ValueId, U32 Value);
	void (*SendByte)(U8 Output);
} RS_VBUS_INFO;

typedef struct __RS_VBUS_STATE
{
	int RxIndex;
	I8 RxFrameNr;
	U8 RxBuffer [17];
} RS_VBUS_STATE;


#define RS_VBUS_MT_FUNC(a)					Rs_VBusMT_##a
#define RS_VBUS_MT_INFO_(a)					__VBus_Info->a
#define RS_VBUS_MT_STATE_(a)				__VBus_State->a
#define RS_VBUS_MT_ARGS						const RS_VBUS_INFO* __VBus_Info, RS_VBUS_STATE* __VBus_State
#define RS_VBUS_MT_ARGS_					const RS_VBUS_INFO* __VBus_Info, RS_VBUS_STATE* __VBus_State,
#define RS_VBUS_MT_PARAMS					__VBus_Info, __VBus_State
#define RS_VBUS_MT_PARAMS_					__VBus_Info, __VBus_State,


#define RS_VBUS_ST_FUNC(a)					Rs_VBusST_##a
#define RS_VBUS_ST_INFO_(a)					VBus_##a
#define RS_VBUS_ST_STATE_(a)				__Rs_VBus_State.a
#define RS_VBUS_ST_ARGS
#define RS_VBUS_ST_ARGS_
#define RS_VBUS_ST_PARAMS
#define RS_VBUS_ST_PARAMS_

extern void VBus_HardwareInitialize(void);
extern void VBus_PacketReceived(I16 Dst, I16 Src, I16 Cmd, I8 FrameCount, I8 FrameNr, const U8* Data);
extern void VBus_DatagramReceived(I16 Dst, I16 Src, I16 Cmd, U16 ValueId, U32 Value);
extern void VBus_SendByte(U8 Output);

extern RS_VBUS_STATE __Rs_VBus_State;


#ifdef RS_VBUS_MULTITON
	#define RS_VBUS_FUNC(a)					RS_VBUS_MT_FUNC(a)
	#define RS_VBUS_INFO_(a)				RS_VBUS_MT_INFO_(a)
	#define RS_VBUS_STATE_(a)				RS_VBUS_MT_STATE_(a)
	#define RS_VBUS_ARGS					RS_VBUS_MT_ARGS
	#define RS_VBUS_ARGS_					RS_VBUS_MT_ARGS_
	#define RS_VBUS_PARAMS					RS_VBUS_MT_PARAMS
	#define RS_VBUS_PARAMS_					RS_VBUS_MT_PARAMS_
#else
	#define RS_VBUS_FUNC(a)					RS_VBUS_ST_FUNC(a)
	#define RS_VBUS_INFO_(a)				RS_VBUS_ST_INFO_(a)
	#define RS_VBUS_STATE_(a)				RS_VBUS_ST_STATE_(a)
	#define RS_VBUS_ARGS					RS_VBUS_ST_ARGS
	#define RS_VBUS_ARGS_					RS_VBUS_ST_ARGS_
	#define RS_VBUS_PARAMS					RS_VBUS_ST_PARAMS
	#define RS_VBUS_PARAMS_					RS_VBUS_ST_PARAMS_
#endif

#define Rs_VBus_CalcCrc						RS_VBUS_FUNC(CalcCrc)
#define Rs_VBus_InjectSeptett				RS_VBUS_FUNC(InjectSeptett)
#define Rs_VBus_ExtractSeptett				RS_VBUS_FUNC(ExtractSeptett)
#define Rs_VBus_SetTxProtocolSelector		RS_VBUS_FUNC(SetTxProtocolSelector)
#define Rs_VBus_ByteReceived				RS_VBUS_FUNC(ByteReceived)
#define Rs_VBus_SendByteWithCrc				RS_VBUS_FUNC(SendByteWithCrc)
#define Rs_VBus_SendByteWithCrcAndSeptet	RS_VBUS_FUNC(SendByteWithCrcAndSeptet)
#define Rs_VBus_SendPacketHeader			RS_VBUS_FUNC(SendPacketHeader)
#define Rs_VBus_SendPacketFrame				RS_VBUS_FUNC(SendPacketFrame)
#define Rs_VBus_SendDatagram				RS_VBUS_FUNC(SendDatagram)
#define Rs_VBus_Initialize					RS_VBUS_FUNC(Initialize)

U8 Rs_VBus_CalcCrc(U8* Buffer, UINT Offset, UINT Length);
void Rs_VBus_InjectSeptett(U8* Buffer, UINT Offset, UINT Length, U8 Septett);
U8 Rs_VBus_ExtractSeptett(U8* Buffer, UINT Offset, UINT Length);
void Rs_VBus_ByteReceived(RS_VBUS_ARGS_ U8 Data);
void Rs_VBus_SendByteWithCrc(RS_VBUS_ARGS_ U8 Data, U8* Crc);
void Rs_VBus_SendByteWithCrcAndSeptet(RS_VBUS_ARGS_ U8 Data, U8* Crc, U8* SeptetValue, U8* SeptetMask);
void Rs_VBus_SendPacketHeader(RS_VBUS_ARGS_ I16 Dst, I16 Src, I16 Cmd, I8 FrameCount);
void Rs_VBus_SendPacketFrame(RS_VBUS_ARGS_ U8 Data0, U8 Data1, U8 Data2, U8 Data3);
void Rs_VBus_SendDatagram(RS_VBUS_ARGS_ I16 Dst, I16 Src, I16 Cmd, U16 ValueId, U32 Value);
void Rs_VBus_Initialize(RS_VBUS_ARGS);

/**
 * @}
 */



#define RS_ALIGN_POINTER_DOWN(Pointer) ((void *) ((IPTR) (Pointer) & (~(sizeof (void *) - 1))))
#define RS_ALIGN_POINTER_UP(Pointer) RS_ALIGN_POINTER_DOWN((IPTR) (Pointer) + sizeof (void *) - 1)

#define RS_ALIGNED_BUFFER_DECLARATION(Name, Size) IPTR Name [((Size) + sizeof (IPTR) - 1) / sizeof (IPTR)]

typedef U16 RS_VALUEID;
typedef I32 RS_VALUE;



#endif /* PLATFORMAPI_H_ */
