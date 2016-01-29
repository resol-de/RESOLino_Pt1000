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
#ifndef ADCAD7790_H_
#define ADCAD7790_H_

#include "PlatformApi.h"
#include "Platform.h"

//===========================================================================
// ADCAD7790
//===========================================================================
/**
 * @defgroup AdcAD7790
 * @ingroup
 * @{
 * @author RESOL
 *
 */

typedef struct __ADCAD7790_INFO
{
	void (*HardwareInitialize)(void);
	void (*Reset)(void);
	U32 (*SetMuxChannel)(int Channel);
	void (*Enable)(int Enable);
	int (*Transceive)(U8 Output, int CheckOnly);
	void (*MeasurementComplete)(void);
	U32* ChannelValues;
	UINT ChannelCount;
} ADCAD7790_INFO;

typedef struct __ADCAD7790_STATE
{
	int NextPriorityChannel;
	int PriorityChannel;
	int NextChannel;
	int CurrentChannel;
	int Phase;
	int Timeout;
	U32 Delay;
	U32 LastTimer;
} ADCAD7790_STATE;



extern void AdcAD7790_HardwareInitialize(void);
extern void AdcAD7790_Reset(void);
extern U32 AdcAD7790_SetMuxChannel(int Channel);
extern void AdcAD7790_Enable(int Enable);
extern int AdcAD7790_Transceive(U8 Output, int CheckOnly);
extern void AdcAD7790_MeasurementComplete(int Channel);
extern I32 AdcAD7790_ChannelValues [];
extern const UINT AdcAD7790_ChannelCount;

extern ADCAD7790_STATE __AdcAD7790_State;

#define ADCAD7790_FUNC(a)				AdcAD7790_##a
#define ADCAD7790_INFO_(a)				AdcAD7790_##a
#define ADCAD7790_STATE_(a)				__AdcAD7790_State.a
#define ADCAD7790_ARGS
#define ADCAD7790_ARGS_
#define ADCAD7790_PARAMS
#define ADCAD7790_PARAMS_


#define AdcAD7790_MeasurementLoop			ADCAD7790_FUNC(MeasurementLoop)
#define AdcAD7790_LoopHandler				ADCAD7790_FUNC(LoopHandler)
#define AdcAD7790_Initialize				ADCAD7790_FUNC(Initialize)

void AdcAD7790_LoopHandler(void);
void AdcAD7790_Initialize(void);


/**
 * @}
 */

#endif /* ADCAD7790_H_ */
