/* Private Includes --------------------------*/
#include <Arduino.h>
#include "AudioKitHAL.h"

/* Private Defines ---------------------------*/
#define DMA_BUFFER_SIZE (32)	/*<! The number of samples to read/write */
#define SAMPLE_RATE		(8000)	/*<! The sample rate (It must be defined according to the AudioKit) */
#define FIR_NUM_TAPS 	(11)	/*<! The number of coefficients of the filter */

/* Private Funcions ---------------------------*/
int FIR_LowPass_Calc(float wc, int M,float *h)
{
	// Only allows for even filter taps
	if( (M % 2) == 0) return 0;
	
	for(int k = 0; k < M; k++)
	{
		if(k == ((M-1)/2))
		{
			h[k] = wc/M_PI;
		}
		else
		{
			h[k] = wc/M_PI * sin(wc*(k - (M - 1)/2))/(wc*(k - (M - 1)/2));
		}
	}
	
	return 1;
}

int FIR_HighPass_Calc(float wc, int M, float *h)
{
	// Only allows for even filter taps
	if( (M % 2) == 0) return 0;
	
	return 1;
}

int FIR_BandPass_Calc(float w1, float w2, int M, float *h)
{
	// Only allows for even filter taps
	if( (M % 2) == 0) return 0;
	
	return 1;
}

int FIR_StopBand_Calc(float w1, float w2, int M, float *h)
{
	// Only allows for even filter taps
	if( (M % 2) == 0) return 0;
	
	return 1;
}

/* Global variables ---------------------------*/
int16_t xprev[FIR_NUM_TAPS-1];	// h[n-1], h[n-2], ..., h[n-(TAPS-1)]
float hlp[FIR_NUM_TAPS];		// Low-pass filter impulse response buffer

/* Setup --------------------------------------*/
void setup()
{
	// Init serial
	Serial.begin(115200);
	
	// Fir filter begin
	float wc = 2*M_PI*500/SAMPLE_RATE;	// Fc = 500Hz
	if( !FIR_LowPass_Calc(wc, FIR_NUM_TAPS, hlp) ) 
	{
		Serial.println("Invalid Number of taps");
		while(1)
		{
			
		}
	}
	
	// I2S Config
	auto cfg = kit.defaultConfig(KitInputOutput);
	cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE2;			// MICROPHONE/AUXIN audio input
	cfg.dac_output = AUDIO_HAL_DAC_OUTPUT_ALL;			// SPEAKER/HEADPHONE audio output
	cfg.sample_rate = AUDIO_HAL_08K_SAMPLES;			// Sampling frequency (Fs)
	cfg.bits_per_sample = AUDIO_HAL_BIT_LENGTH_16BITS;	// Number of bits per sample
	cfg.buffer_size = 32;								// DMA buffer size (Each entry stores a variable of bits_per_sample)
	kit.begin(cfg);										// Initialize ES8388 audio codec
	kit.setVolume(100);									// Set audio codec volume to 100
}

/* Main loop ----------------------------------*/
void loop()
{
	/* Signal Sampling (ADC Conversion) */
	// The "AudioBuffer" variable have both L and R samples alternated, i.e.
	// x_left(0),x_right(0),x_left(1),x_right(1),...,x_left(N-1),x_right(N-1)
	size_t bytesRead = kit.read((uint8_t *)AudioBuffer, sizeof(AudioBuffer));

	/* DSP processing goes here */
	// "AudioBuffer" variable has both Left and Right audio samples
	// n+=2 ensures we take only the Left audio samples
	for(int n = 0; n < bytesRead/2; n+=2) {
		// y[n] = x[n]*h[0] + x[n-1]*h[1] + .... + x[n - M + 1]*h[M-1]
		float y = h[0]*AudioBuffer[n];
		for(k = 0; k < 10; k++)
		{
			y += xprev[k]*h[k+1];
		}
		
		// Update previous input buffer
		for(k = 10; k > 0; k--)
		{
			xprev[k] = xprev[k-1];
		}
		xprev[0] = AudioBuffer[n];
		
		// Set current sample (Both L&R samples)
		AudioBuffer[n] = (int16_t)y;
		AudioBuffer[n+1] = (int16_t)y;
	}
	// DSP END

	/* Signal Reconstruction (write to DAC) */
	kit.write((uint8_t *)AudioBuffer, bytesRead);
}
