/*
Library:					STM32F4 Audio Codec - CS43L22
Written by:				Mohamed Yaqoob (MYaqoobEmbedded YouTube Channel)
Date Written:			29/01/2016
Last modified:			29/12/2018
Description:			This is an STM32 device driver library for the CS43L22 Audio Codec, using STM HAL libraries
References:
			1) Cirrus Logic CS43L22 datasheet
				 https://www.mouser.com/ds/2/76/CS43L22_F2-1142121.pdf
			2) ST opensource CS43L22 Audio Codec dsp drivers.

* Copyright (C) 2018 - M. Yaqoob
   This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
   of the GNU General Public Licenseversion 3 as published by the Free Software Foundation.

   This software library is shared with puplic for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
   or indirectly by this software, read more about this on the GNU General Public License.
*/

#ifndef __CS43L22_H_
#define __CS43L22_H_

/* Public Includes -------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "main.h"

/* Public define ------------------------------------------------------------*/

/* Public macro -------------------------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/
typedef enum
{
	CS43L22_MODE_I2S = 0,
	CS43L22_MODE_ANALOG
} cs43l22_mode_t;

typedef enum
{
	CS43L22_CHANNEL_MUTE = 0x00,
	CS43L22_CHANNEL_RIGHT,
	CS43L22_CHANNEL_LEFT,
	CS43L22_CHANNEL_RIGHT_LEFT
} cs43l22_channel_t;

typedef enum
{
	CS43L22_BPS_24_BITS = 0x00,
	CS43L22_BPS_20_BITS,
	CS43L22_BPS_18_BITS,
	CS43L22_BPS_16_BITS
} cs43l22_bps_t;


typedef enum
{
	CS43L22_OUTPUT_DISABLE=0x00,
	CS43L22_OUTPUT_SPEAKER,
	CS43L22_OUTPUT_HEADPHONE,
	CS43L22_OUTPUT_BOTH,
	CS43L22_OUTPUT_AUTO
} cs43l22_output_t;

/* Public function prototypes -----------------------------------------------*/
HAL_StatusTypeDef HAL_CS43L22_Init(I2C_HandleTypeDef *hi2cx, cs43l22_mode_t mode);
void HAL_CS43L22_Set_Channel(cs43l22_channel_t channel);
void HAL_CS43L22_Set_Volume(uint8_t volume);
void HAL_CS43L22_Set_BPS(cs43l22_bps_t bit_depth);
void HAL_CS43L22_Set_OutputDevice(cs43l22_output_t device);
void HAL_CS43L22_Start(void);
void HAL_CS43L22_Stop(void);

#endif /* __CS43L22_H_ */
