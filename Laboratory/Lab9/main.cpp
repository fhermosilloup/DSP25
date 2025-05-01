#include <Arduino.h>
#include <Wire.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "AudioKitHAL.h"
#include <WiFi.h>

/* Private Defines ---------------------------*/
#define DMA_BUFFER_SIZE 32

#define LED_4  GPIO_NUM_22
#define LED_5  GPIO_NUM_21
#define KEY_1  GPIO_NUM_36
#define KEY_2  GPIO_NUM_13
#define KEY_3  GPIO_NUM_19
#define KEY_4  GPIO_NUM_23
#define KEY_5  GPIO_NUM_18
#define KEY_6  GPIO_NUM_5

#define GPIO_LOW    0
#define GPIO_HIGH   1

/* Private Macros ------------------------*/
#define TOFLOAT(X)  ((float)(X)/32767.0F)
#define TOINT16(X)  ((X)*32767.0F)
#define MAX(X,Y)    ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y)    ((X) < (Y) ? (X) : (Y))

/* Private Structures ------------------------*/

/* Private Constants ------------------------*/
const float fir_filt_coefs[11] = {0.0113,
  0.0266,
  0.0686,
  0.1248,
  0.1728,
  0.1916,
  0.1728,
  0.1248,
  0.0686,
  0.0266,
  0.0113};
float xprev[10];

/* Private Variables -------------------------*/
int16_t AudioBuffer[DMA_BUFFER_SIZE];	//!< Buffer that stores the data to process
AudioKit kit;

/* Private functions --------------------------*/
void audiokit_gpio_init(void);

/* Interrupts ---------------------------------*/


/* Setup --------------------------------------*/
void setup()
{
  // I2S Config
  AudioKitConfig cfg = kit.defaultConfig(KitInputOutput);
  cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE2;	// MICROPHONE/AUXIN audio input
  cfg.dac_output = AUDIO_HAL_DAC_OUTPUT_ALL;	// SPEAKER/HEADPHONE audio output
  cfg.sample_rate = AUDIO_HAL_48K_SAMPLES;		// Sampling frequency (Fs)
  cfg.bits_per_sample = AUDIO_HAL_BIT_LENGTH_16BITS;	// Number of bits per sample
  cfg.buffer_size=DMA_BUFFER_SIZE;	// DMA buffer size (Each entry stores a variable of bits_per_sample)
  cfg.buffer_count = 2; // Number of buffers used for DMA (Def 6)(Total memory footprint is buffer_count*buffer_size*bits_per_sample/8)
  kit.begin(cfg);		// Initialize ES8388 audio codec
  kit.setVolume(100);	// Set audio codec volume to 100
  kit.setSpeakerActive(false);
  // BSP audiokit gpio initialization
  audiokit_gpio_init();
}

/* Main loop ----------------------------------*/
void loop()
{
  // Signal Sampling
  // Suspend main thread until the number of bytes determined by the 
  // buffer size is read (this task is usually supended. An interrupt
  // is generated when when the number of bytes are read, and after that
  // the RTOS yield the CPU usage to this task.
  //
  // The "AudioBuffer" variable have both L and R samples alternated, i.e.
  // x_left(0),x_right(0),x_left(1),x_right(1),...,x_left(N-1),x_right(N-1)
  size_t bytesRead = kit.read((uint8_t *)AudioBuffer, sizeof(AudioBuffer));


  // DSP processing goes here
  // "AudioBuffer" variable has both Left and Right audio samples
  // n+=2 ensures we take only the Left audio samples
  /*for(int n = 0; n < bytesRead/2; n+=2)
  {
    // Get current sample: int16 to float
    float x_L = TOFLOAT(AudioBuffer[n]);
	  float x_R = TOFLOAT(AudioBuffer[n+1]);
	
    // Process sample by sample
    // BEGIN
    float y = f1.filter(x_L);
    // END
    
    // Audio clipping Keeps audio between [-1,1]
    y = MIN(MAX(y,-1.0F),1.0F);
    y = MIN(MAX(y,-1.0F),1.0F);
  
    // float to int16
    AudioBuffer[n] = TOINT16(y);
    AudioBuffer[n] = TOINT16(y);
  }*/
  
  // Signal Interpolation
  // Suspend main thread until buffer size is read (yield from interrupt)
  // 1. It is better to have a reader and a writter task separately to 
  // avoid suspending audio sampling when sample data is reconstructed
  // 2. A queue can be used to communicate each task that new data has arrived
  // 3. Also consider to use double buffering.
  kit.write((uint8_t *)AudioBuffer, bytesRead);
}








/* Reference functions -----------------------------------------*/
void audiokit_gpio_init(void)
{
  // Set GPIO to reset state
  gpio_reset_pin(LED_4);
  gpio_reset_pin(LED_5);
  gpio_reset_pin(KEY_1);
  gpio_reset_pin(KEY_2);
  gpio_reset_pin(KEY_3);
  gpio_reset_pin(KEY_4);
  gpio_reset_pin(KEY_5);
  gpio_reset_pin(KEY_6);

  // Set gpio direction
  gpio_set_direction(LED_4, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_5, GPIO_MODE_OUTPUT);
  gpio_set_direction(KEY_1, GPIO_MODE_INPUT);
  gpio_set_direction(KEY_2, GPIO_MODE_INPUT);
  gpio_set_direction(KEY_3, GPIO_MODE_INPUT);
  gpio_set_direction(KEY_4, GPIO_MODE_INPUT);
  gpio_set_direction(KEY_5, GPIO_MODE_INPUT);
  gpio_set_direction(KEY_6, GPIO_MODE_INPUT);
  
  // Set gpio output level
  gpio_set_level(LED_4, GPIO_HIGH);  // 1: LOW
  gpio_set_level(LED_5, GPIO_HIGH);  // 1: LOW

  gpio_reset_pin(GPIO_NUM_12);
  gpio_set_direction(GPIO_NUM_12, GPIO_MODE_INPUT);
  gpio_pullup_en(GPIO_NUM_12);
  gpio_set_pull_mode(GPIO_NUM_12, GPIO_PULLUP_ONLY);
}
