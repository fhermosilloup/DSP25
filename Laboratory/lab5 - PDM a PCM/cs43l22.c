/*
Library:				STM32F4 Audio Codec - CS43L22
Written by:				Fernando Hermosillo Reynoso
Date Written:			29/01/2016
Last modified:			29/12/2018
Description:			This is an STM32 device driver library for the CS43L22 Audio Codec, using STM HAL libraries
References:
			1) Cirrus Logic CS43L22 datasheet
				 https://www.mouser.com/ds/2/76/CS43L22_F2-1142121.pdf

* Copyright (C) 2018 - Fernando Hermosillo
   This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
   of the GNU General Public Licenseversion 3 as published by the Free Software Foundation.

   This software library is shared with puplic for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
   or indirectly by this software, read more about this on the GNU General Public License.
*/

#include "cs43l22.h"

/* Private define ------------------------------------------------------------*/
#define CHIP_ID_REG							0x01
#define POWER_CONTROL1						0x02
#define POWER_CONTROL2						0x04
#define CLOCKING_CONTROL 	  				0x05
#define INTERFACE_CONTROL1					0x06
#define INTERFACE_CONTROL2					0x07
#define PASSTHROUGH_A						0x08
#define PASSTHROUGH_B						0x09
#define MISCELLANEOUS_CONTRLS				0x0E
#define PLAYBACK_CONTROL					0x0F
#define PASSTHROUGH_VOLUME_A				0x14
#define PASSTHROUGH_VOLUME_B				0x15
#define PCM_VOLUME_A						0x1A
#define PCM_VOLUME_B						0x1B
#define CONFIG_00							0x00
#define CONFIG_47							0x47
#define CONFIG_32							0x32

#define CS43L22_REG_MASTER_A_VOL        	0x20
#define CS43L22_REG_MASTER_B_VOL        	0x21

#define CS43L22_I2C_ADDR 					0x94

/* Private macro -------------------------------------------------------------*/
#define VOLUME_CONVERT_A(Volume)    (((Volume) > 100)? 255:((uint8_t)(((Volume) * 255) / 100)))
#define VOLUME_CONVERT_D(Volume)    (((Volume) > 100)? 24:((uint8_t)((((Volume) * 48) / 100) - 24)))

/* Private variables ---------------------------------------------------------*/
static uint8_t ucI2CBuffer[2];
static I2C_HandleTypeDef *hi2c = NULL;

/* Private function reference ---------------------------------------------------------*/
// Write to register
static HAL_StatusTypeDef CS43L22_IO_Write(uint8_t reg, uint8_t value)
{
	ucI2CBuffer[0] = reg;
	ucI2CBuffer[1] = value;

	return HAL_I2C_Master_Transmit(hi2c, CS43L22_I2C_ADDR, ucI2CBuffer, 2, 100);
}
// Read from register
static HAL_StatusTypeDef CS43L22_IO_Read(uint8_t reg, uint8_t *dout)
{
	ucI2CBuffer[0] = reg;
	HAL_I2C_Master_Transmit(hi2c, CS43L22_I2C_ADDR, ucI2CBuffer, 1, 100);
	return HAL_I2C_Master_Receive(hi2c,  CS43L22_I2C_ADDR,  dout, 1, 100);
}

/* Public function reference ---------------------------------------------------------*/
HAL_StatusTypeDef HAL_CS43L22_Init(I2C_HandleTypeDef *hi2cx, cs43l22_mode_t mode)
{
	uint8_t ucData = 0x00;

	if(!hi2cx) return HAL_ERROR;

	// Get handlers
	hi2c = hi2cx;

	// Enable I2S
	//__HAL_UNLOCK(hi2s);     // THIS IS EXTREMELY IMPORTANT FOR I2S3 TO WORK!!
	//__HAL_I2S_ENABLE(hi2s); // THIS IS EXTREMELY IMPORTANT FOR I2S3 TO WORK!!

	// Reset CS43L22 driver
	HAL_GPIO_WritePin(Audio_RST_GPIO_Port, Audio_RST_Pin, GPIO_PIN_SET);

	// Check connection
	if (HAL_I2C_IsDeviceReady(hi2c, CS43L22_I2C_ADDR, 1, 10) != HAL_OK) return HAL_ERROR;

	// Power down
	CS43L22_IO_Write(POWER_CONTROL1, 0x01);

	// Read chip ID
	CS43L22_IO_Read(CHIP_ID_REG, &ucData);

	// Enable Right and Left headphones
	ucData =  (2 << 6);  // PDN_HPB[0:1]  = 10 (HP-B always on)
	ucData |= (2 << 4);  // PDN_HPA[0:1]  = 10 (HP-A always on)
	ucData |= (3 << 2);  // PDN_SPKB[0:1] = 11 (Speaker B always off)
	ucData |= (3 << 0);  // PDN_SPKA[0:1] = 11 (Speaker A always off)
	CS43L22_IO_Write(POWER_CONTROL2, ucData);

	// Automatic clock detection
	CS43L22_IO_Write(CLOCKING_CONTROL, 0x80);

	// Interface control 1
	CS43L22_IO_Read(INTERFACE_CONTROL1, &ucData);
	ucData &= (1 << 5); 	// Clear all bits except bit 5 which is reserved
	ucData &= ~(1 << 7);  	// Slave mode
	ucData &= ~(1 << 6);  	// Clock polarity: Not inverted
	ucData &= ~(1 << 4);  	// No DSP mode
	ucData &= ~(0x03 << 2); // Left justified, up to 24 bit (default)
	ucData |=  (1 << 2);	// I2S, up to 24-bit data
	ucData &=  ~(3 << 0);   // 24-bit audio word length for I2S interface
	ucData |=  (3 << 0);	// 16-bit audio word length for I2S interface
	CS43L22_IO_Write(INTERFACE_CONTROL1, ucData);

	// Passthrough A settings
	CS43L22_IO_Read(PASSTHROUGH_A, &ucData);
	ucData &= 0xF0;      // Bits [4-7] are reserved
	ucData |=  (1 << 0); // Use AIN1A as source for passthrough
	CS43L22_IO_Write(PASSTHROUGH_A, ucData);

	// Passthrough B settings
	CS43L22_IO_Read(PASSTHROUGH_B, &ucData);
	ucData &= 0xF0;      // Bits [4-7] are reserved
	ucData |=  (1 << 0); // Use AIN1B as source for passthrough
	CS43L22_IO_Write(PASSTHROUGH_B, ucData);

	// Miscellaneous register settings
	CS43L22_IO_Read(MISCELLANEOUS_CONTRLS, &ucData);
	if(mode == CS43L22_MODE_ANALOG)
	{
		ucData |=  (1 << 7);   // Enable passthrough for AIN-A
		ucData |=  (1 << 6);   // Enable passthrough for AIN-B
		ucData &= ~(1 << 5);   // Unmute passthrough on AIN-A
		ucData &= ~(1 << 4);   // Unmute passthrough on AIN-B
		ucData &= ~(1 << 3);   // Changed settings take effect immediately
	}
	else if(mode == CS43L22_MODE_I2S)
	{
		ucData = 0x02;
	}
	CS43L22_IO_Write(MISCELLANEOUS_CONTRLS, ucData);

	// Unmute headphone and speaker
	//CS43L22_IO_Read(PLAYBACK_CONTROL, &ucData);
	CS43L22_IO_Write(PLAYBACK_CONTROL, 0x00);

	// Set volume to default (0dB)
	CS43L22_IO_Write(PASSTHROUGH_VOLUME_A, 0x00);
	CS43L22_IO_Write(PASSTHROUGH_VOLUME_B, 0x00);
	CS43L22_IO_Write(PCM_VOLUME_A, 0x00);
	CS43L22_IO_Write(PCM_VOLUME_B, 0x00);

	return HAL_OK;
}

// Function(2): Enable Right and Left headphones
void HAL_CS43L22_Set_Channel(cs43l22_channel_t channel)
{
	uint8_t ucData = 0x00;
	switch (channel)
	{
		case CS43L22_CHANNEL_MUTE:
			ucData =  (3 << 6);  // PDN_HPB[0:1]  = 10 (HP-B always on)
			ucData |= (3 << 4);  // PDN_HPA[0:1]  = 10 (HP-A always on)
		break;

		case CS43L22_CHANNEL_RIGHT:
			ucData =  (2 << 6);  // PDN_HPB[0:1]  = 10 (HP-B always on)
			ucData |= (3 << 4);  // PDN_HPA[0:1]  = 10 (HP-A always on)
		break;

		case CS43L22_CHANNEL_LEFT:
			ucData =  (3 << 6);  // PDN_HPB[0:1]  = 10 (HP-B always on)
			ucData |= (2 << 4);  // PDN_HPA[0:1]  = 10 (HP-A always on)
		break;

		case CS43L22_CHANNEL_RIGHT_LEFT:
			ucData =  (2 << 6);  // PDN_HPB[0:1]  = 10 (HP-B always on)
			ucData |= (2 << 4);  // PDN_HPA[0:1]  = 10 (HP-A always on)
		break;

		default:
		break;
	}
	ucData |= (3 << 2);  // PDN_SPKB[0:1] = 11 (Speaker B always off)
	ucData |= (3 << 0);  // PDN_SPKA[0:1] = 11 (Speaker A always off)
	CS43L22_IO_Write(POWER_CONTROL2, ucData);
}

// Function(3): Set Volume Level
void HAL_CS43L22_Set_Volume(uint8_t volume)
{
	// Convert volume to register
	int8_t tempVol = volume - 50;
	tempVol = tempVol*(127/50);
	uint8_t ucData =  (uint8_t )tempVol;


	// Write to volume registers
	CS43L22_IO_Write(PASSTHROUGH_VOLUME_A, ucData);
	CS43L22_IO_Write(PASSTHROUGH_VOLUME_B, ucData);

	// Set the Master volume
	ucData = VOLUME_CONVERT_D(volume);
	CS43L22_IO_Write(CS43L22_REG_MASTER_A_VOL, ucData);
	CS43L22_IO_Write(CS43L22_REG_MASTER_B_VOL, ucData);
}

// Set bits per sample
void HAL_CS43L22_Set_BPS(cs43l22_bps_t bit_depth)
{
	uint8_t ucData = 0x00;

	CS43L22_IO_Read(INTERFACE_CONTROL1, &ucData);
	ucData &= ~ 0x03; // 24-bits
	ucData |= (uint8_t)bit_depth;
	CS43L22_IO_Write(CS43L22_REG_MASTER_A_VOL, ucData);
}

void HAL_CS43L22_Set_OutputDevice(cs43l22_output_t device)
{
	switch (device)
	{
		case CS43L22_OUTPUT_SPEAKER:
			CS43L22_IO_Write(POWER_CONTROL2, 0xFA); /* SPK always ON & HP always OFF */
		break;

		case CS43L22_OUTPUT_HEADPHONE:
			CS43L22_IO_Write(POWER_CONTROL2, 0xAF); /* SPK always OFF & HP always ON */
		break;

		case CS43L22_OUTPUT_BOTH:
			CS43L22_IO_Write(POWER_CONTROL2, 0xAA); /* SPK always ON & HP always ON */
		break;

		case CS43L22_OUTPUT_AUTO:
			CS43L22_IO_Write(POWER_CONTROL2, 0x05); /* Detect the HP or the SPK automatically */
		break;

		default:
			CS43L22_IO_Write(POWER_CONTROL2, 0xFF); /* Disable */
		break;
	}
}

// Function(4): Start the Audio DAC
void HAL_CS43L22_Start(void)
{
	uint8_t ucData = 0x00;

	// Write 0x99 to register 0x00.
	CS43L22_IO_Write(CONFIG_00, 0x99);

	// Write 0x80 to register 0x47.
	CS43L22_IO_Write(CONFIG_47, 0x80);

	// Write 1 to bit 7 in register 0x32.
	CS43L22_IO_Read(CONFIG_32, &ucData);
	ucData |= 0x80;
	CS43L22_IO_Write(CONFIG_32, ucData);

	// Write 0 to bit 7 in register 0x32.
	CS43L22_IO_Read(CONFIG_32, &ucData);
	ucData &= ~(0x80);
	CS43L22_IO_Write(CONFIG_32, ucData);

	// Write 0x00 to register 0x00.
	CS43L22_IO_Write(CONFIG_00, 0x00);

	//Set the "Power Ctl 1" register (0x02) to 0x9E
	CS43L22_IO_Write(POWER_CONTROL1, 0x9E);
}

void HAL_CS43L22_Stop(void)
{
	CS43L22_IO_Write(POWER_CONTROL1, 0x01);
}
