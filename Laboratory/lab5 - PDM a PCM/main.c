/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "i2s.h"
#include "pdm2pcm.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fifo.h"
#include "cs43l22.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Audio In/Out parameters
#define AUDIO_SAMPLE_RATE		(48000UL)
#define AUDIO_DATA_SIZE			(16UL)
#define AUDIO_FRAME_SIZE		(16UL)
#define AUDIO_NUM_IN_CHANNELS	(1UL)
#define AUDIO_NUM_OUT_CHANNELS	(2UL)

// Audio PDM to PCM parameters
#define AUDIO_DECIMATION_FACTOR	(32UL)
#define AUDIO_NUM_OUT_SAMPLES	(AUDIO_SAMPLE_RATE/1000UL) // 1ms pcm data
#define AUDIO_PDM_BUFFER_SIZE	(AUDIO_NUM_OUT_SAMPLES*AUDIO_NUM_IN_CHANNELS*AUDIO_DECIMATION_FACTOR/16)
#define AUDIO_PCM_BUFFER_SIZE	(AUDIO_NUM_OUT_SAMPLES*AUDIO_NUM_OUT_CHANNELS*AUDIO_FRAME_SIZE/16)

// Audio DMA Buffers
#define AUDIO_RX_DMA_BUFSIZE	(2*AUDIO_PDM_BUFFER_SIZE)
#define AUDIO_TX_DMA_BUFSIZE	(2*AUDIO_PCM_BUFFER_SIZE)

// Internal Flags
#define AUDIO_RX_HALFCPLT_STATE	(1)
#define AUDIO_RX_FULLCPLT_STATE	(2)
#define AUDIO_TX_HALFCPLT_STATE (1)
#define AUDIO_TX_FULLCPLT_STATE	(2)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t uAudioRxDmaBuffer[AUDIO_RX_DMA_BUFSIZE];
int16_t uAudioTxDmaBuffer[AUDIO_TX_DMA_BUFSIZE];
int16_t uPcmBuffer[AUDIO_NUM_OUT_SAMPLES];
volatile uint8_t ucAudioRxDmaState = 0;	// Flag for RX DMA buffer full status
volatile uint8_t ucAudioTxDmaState = 0;	// Flag for TX DMA buffer full status
int16_t uPcmValue = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */
// To enable printf function on debugging
int _write(int file, char *ptr, int len)
{
  UNUSED(file);
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}

/*
 * This function is called when a new block of AUDIO_NUM_OUT_SAMPLES
 * PCM samples is received.
 */
void AudioProcessCallback(int16_t *micData, uint16_t numSamples)
{
	for(int n = 0; n < numSamples; n++)
	{
		uPcmValue = micData[n];
	}
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_I2S2_Init();
  MX_CRC_Init();
  MX_PDM2PCM_Init();
  /* USER CODE BEGIN 2 */
  // Audio Codec CS43L22 init
  if(HAL_CS43L22_Init(&hi2c1, CS43L22_MODE_I2S) != HAL_OK)
  {
	  // If error, toggle RED led on STM32F407G-DISC1
	  while(1)
	  {
		  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		  HAL_Delay(500);
	  }
  }
  HAL_CS43L22_Set_BPS(CS43L22_BPS_16_BITS);
  HAL_CS43L22_Set_Channel(CS43L22_CHANNEL_RIGHT_LEFT);
  HAL_CS43L22_Set_OutputDevice(CS43L22_OUTPUT_HEADPHONE);
  HAL_CS43L22_Set_Volume(50);
  HAL_CS43L22_Start();

  // PCM Fifo init
  int16_t PcmFifoBuffer[AUDIO_TX_DMA_BUFSIZE*2]; // At least twice the TX DMA buffer size
  Fifo_t mPcmFifo;
  FIFO_Init(&mPcmFifo, PcmFifoBuffer, AUDIO_TX_DMA_BUFSIZE*2);

  // Start I2S audio transmission(DAC)/reception(Mic) DMA service
  HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*) uAudioTxDmaBuffer, AUDIO_TX_DMA_BUFSIZE / (AUDIO_FRAME_SIZE/16UL));
  HAL_I2S_Receive_DMA(&hi2s2,  uAudioRxDmaBuffer, AUDIO_RX_DMA_BUFSIZE / (AUDIO_FRAME_SIZE/16UL));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // IMP34DT05 PDM Microphone --------------------------------------------------
	  if(ucAudioRxDmaState == AUDIO_RX_HALFCPLT_STATE)
	  {
		  // Clear the flag
		  ucAudioRxDmaState = 0;

		  // Convert the first half PDM DMA buffer to PCM
		  PDM_Filter(&uAudioRxDmaBuffer[0], &uPcmBuffer[0], &PDM1_filter_handler);

		  /* Data can be processed here */
		  AudioProcessCallback(uPcmBuffer, AUDIO_NUM_OUT_SAMPLES);
		  /* -------------------------- */

		  // Store into a FIFO for transmission
		  FIFO_WriteBlock(&mPcmFifo, uPcmBuffer, AUDIO_NUM_OUT_SAMPLES);
	  }
	  if(ucAudioRxDmaState == AUDIO_RX_FULLCPLT_STATE)
	  {
		  // Clear the flag
		  ucAudioRxDmaState = 0;

		  // Convert the second half PDM DMA buffer to PCM
		  PDM_Filter(&uAudioRxDmaBuffer[AUDIO_PDM_BUFFER_SIZE], &uPcmBuffer[0], &PDM1_filter_handler);

		  /* Data can be processed here -------------------------------*/
		  AudioProcessCallback(uPcmBuffer, AUDIO_NUM_OUT_SAMPLES);
		  /* --------------------------------------------------------- */

		  // Store into a FIFO for transmission
		  FIFO_WriteBlock(&mPcmFifo, uPcmBuffer, AUDIO_NUM_OUT_SAMPLES);
	  }

	  // CS43L22 DAC Codec --------------------------------------------
	  if(ucAudioTxDmaState == AUDIO_TX_HALFCPLT_STATE)
	  {
		  // Clear the flag
		  ucAudioTxDmaState = 0;

		  // Check if there is at least a full PCM output buffer
		  if(mPcmFifo.numel >= AUDIO_PCM_BUFFER_SIZE)
		  {
			  for (int i = 0; i < AUDIO_PCM_BUFFER_SIZE; i=i+(2*AUDIO_FRAME_SIZE/16))
			  {
				// Read data from the FIFO
				int16_t value = 0;
				FIFO_Read(&mPcmFifo, &value);

				// Duplicate output channel for stereo sound
				uAudioTxDmaBuffer[i] = value;							// Right channel
				uAudioTxDmaBuffer[i + (AUDIO_FRAME_SIZE/16)] = value;	// Left channel
			 }
		  }
	  }
	  if(ucAudioTxDmaState == AUDIO_TX_FULLCPLT_STATE)
	  {
		  // Clear the flag
		  ucAudioTxDmaState = 0;

		  // Check if there is at least a full PCM output buffer
		  if(mPcmFifo.numel >= AUDIO_PCM_BUFFER_SIZE)
		  {
			  for (int i=AUDIO_PCM_BUFFER_SIZE; i < 2*AUDIO_PCM_BUFFER_SIZE; i=i+(2*AUDIO_FRAME_SIZE/16))
			  {
				  // Read data from the FIFO
				  int16_t value = 0;
				  FIFO_Read(&mPcmFifo, &value);

				  // Duplicate output channel for stereo sound
				  uAudioTxDmaBuffer[i] = value;							// Right channel
				  uAudioTxDmaBuffer[i + (AUDIO_FRAME_SIZE/16)] = value;	// Left channel
			  }
		  }
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// IMP34DT05 DMA Interrupts ==============================================
void HAL_I2S_RxHalfCpltCallback (I2S_HandleTypeDef *hi2s)
{
	// The first half DMA buffer was filled and is ready to process
	// The second half DMA buffer is currently filling
	ucAudioRxDmaState = AUDIO_RX_HALFCPLT_STATE;
}

void HAL_I2S_RxCpltCallback (I2S_HandleTypeDef *hi2s)
{
	// The second half dma buffer was filled and is ready to process
	// The first half DMA buffer is currently filling
	ucAudioRxDmaState = AUDIO_RX_FULLCPLT_STATE;
}

// CS43L22 DMA Interrupts ==============================================
void HAL_I2S_TxHalfCpltCallback (I2S_HandleTypeDef *hi2s)
{
	// The first half DMA buffer is available for writing
	// The second half DMA buffer is currently transmitting
	ucAudioTxDmaState = AUDIO_TX_HALFCPLT_STATE;
}

void HAL_I2S_TxCpltCallback (I2S_HandleTypeDef *hi2s)
{
	// The second half DMA buffer is available for writing
	// The first half DMA buffer is currently transmitting
	ucAudioTxDmaState = AUDIO_TX_FULLCPLT_STATE;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
