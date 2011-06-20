/************************************************************************
	tlc5940.c

    AVR TLC5940 LED Driver Library
    Copyright (C) 2011 Simon Inns

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Email: simon.inns@gmail.com

************************************************************************/

// Note: This library configures the SPI module, the PWM module (timer1) and
// sets up an interrupt for dealing with the XLAT processing.
// (which requires both timer1 and timer2)

// This library is adapted from my PIC18F library available at
// http://www.waitingforfriday.com/index.php/USB_RGB_LED_VU_Meter

// Includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include "hardware.h"
#include "tlc5940.h"
#include <util/delay.h>

// Array for storing the gray-scale data packed into bytes
unsigned char packedGrayScaleDataBuffer1[24 * NUMBEROF5940];
unsigned char packedGrayScaleDataBuffer2[24 * NUMBEROF5940];

// Flags for the interrupt handling routine
unsigned char waitingForXLAT = 0;
unsigned char updatePending = 0;

// Set initial dot correction data
void setInitialDotCorrection(unsigned char *dotCorrectionValues)
{	
	// Set VPRG high (Dot correction mode)
	sbi(TLC5940_VPRG_PORT, TLC5940_VPRG_PIN);
	
	// We are passed an array of unsigned char values which are 8 bits each, however the dot 
	// correction is expecting 6 bit data for each channel (0-63) so only send the 6 least
	// significant bits of each entry in the array.  The values need to be sent MSB first.
	for (int ledChannel = 0; ledChannel < (16 * NUMBEROF5940); ledChannel++)
	{
		unsigned char bitMask = 0b00100000;
		
		for (int bitCounter = 5; bitCounter >= 0; bitCounter--)
		{
			// Set SIN to DC data bit
			if (((dotCorrectionValues[ledChannel] & bitMask) >> bitCounter) == 1) sbi(TLC5940_SIN_PORT, TLC5940_SIN_PIN);
			else cbi(TLC5940_SIN_PORT, TLC5940_SIN_PIN);
			
			// Pulse the serial clock
			_delay_us(20);
			sbi(TLC5940_SCLK_PORT, TLC5940_SCLK_PIN);
			_delay_us(20);
			cbi(TLC5940_SCLK_PORT, TLC5940_SCLK_PIN);
			
			// Move to the next bit in the mask
			bitMask >>= 1;
		}
	}	
	
	// Pulse XLAT
	_delay_us(20);
	sbi(TLC5940_XLAT_PORT, TLC5940_XLAT_PIN);
	_delay_us(20);
	cbi(TLC5940_XLAT_PORT, TLC5940_XLAT_PIN);
}

// Since the TLC5940 requires an 'extra' SCLK pulse the first time the gray-scales
// are cycled we have to do one input cycle manually, after this we can use the SPI
// module for extra serial interface speed (this is due to the fact that there is
// no way to tell the SPI module to generate the extra clock pulse).
void setInitialGrayScaleValues()
{
	// Set VPRG = Low (Gray-scale mode)
	cbi(TLC5940_VPRG_PORT, TLC5940_VPRG_PIN);
	
	// Reset GSCLK_Counter = 0
	int GSCLKcounter = 0;
	
	// Reset Data_Counter = 0
	int dataCounter = 0;
	
	// Set BLANK = High (Turn LED's Off)
	sbi(TLC5940_BLANK_PORT, TLC5940_BLANK_PIN);
	
	for (GSCLKcounter = 0; GSCLKcounter < 4096; GSCLKcounter++)
	{
		if (dataCounter > (NUMBEROF5940 * 192) )
		{
			// Pulse GSCLK
			_delay_us(20);
			sbi(TLC5940_GSCLK_PORT, TLC5940_GSCLK_PIN);
			_delay_us(20);
			cbi(TLC5940_GSCLK_PORT, TLC5940_GSCLK_PIN);
		}
		else
		{
			// Set SIN to the gray-scale data bit
			cbi(TLC5940_SIN_PORT, TLC5940_SIN_PIN); // We just output zero for everything during initialisation
			
			// Pulse SCLK
			_delay_us(20);
			sbi(TLC5940_SCLK_PORT, TLC5940_SCLK_PIN);
			_delay_us(20);
			cbi(TLC5940_SCLK_PORT, TLC5940_SCLK_PIN);
			
			// Increment Data_Counter
			dataCounter++;
			
			// Pulse GSCLK
			_delay_us(20);
			sbi(TLC5940_GSCLK_PORT, TLC5940_GSCLK_PIN);
			_delay_us(20);
			cbi(TLC5940_GSCLK_PORT, TLC5940_GSCLK_PIN);
		}	
	}
	
	// Pulse XLAT to latch in GS data
	_delay_us(20);
	sbi(TLC5940_XLAT_PORT, TLC5940_XLAT_PIN);
	_delay_us(20);
	cbi(TLC5940_XLAT_PORT, TLC5940_XLAT_PIN);

	// Set BLANK = Low (Turn LED's on)
	_delay_us(20);
	cbi(TLC5940_BLANK_PORT, TLC5940_BLANK_PIN);
	
	// Send an extra SCLK pulse since this is the first gray-scale cycle
	// after the dot correction data has been set
		
	// Pulse SCLK
	_delay_us(20);
	sbi(TLC5940_SCLK_PORT, TLC5940_SCLK_PIN);
	_delay_us(20);
	cbi(TLC5940_SCLK_PORT, TLC5940_SCLK_PIN);
}			

// Initialise the TLC5940 devices
void initialiseTlc5940()
{
	// Power up delay
	_delay_us(100);

	// Initialise device pins
	cbi(TLC5940_GSCLK_PORT, TLC5940_GSCLK_PIN);
	cbi(TLC5940_SCLK_PORT, TLC5940_SCLK_PIN);
	sbi(TLC5940_VPRG_PORT, TLC5940_VPRG_PIN);
	cbi(TLC5940_XLAT_PORT, TLC5940_XLAT_PIN);
	sbi(TLC5940_BLANK_PORT, TLC5940_BLANK_PIN);
	
	// Set up an array of dot correction values (0-63)
	unsigned char dotCorrectionValues[16 * NUMBEROF5940];
	for (unsigned char ledChannel = 0; ledChannel < (16 * NUMBEROF5940); ledChannel++)
		dotCorrectionValues[ledChannel] = 63;
	
	// Set the initial dot correction values
	setInitialDotCorrection(dotCorrectionValues);
	
	// Clear the LED channel data
	for (int bytePointer = 0; bytePointer < 24 * NUMBEROF5940; bytePointer++)
	{
		packedGrayScaleDataBuffer1[bytePointer] = 0;
		packedGrayScaleDataBuffer1[bytePointer] = 0;
	}
	
	// Set the initial gray-scale values
	setInitialGrayScaleValues();

	// Set up SPI for communicating with the TLC5940
	// Enable SPI as master
	//SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << SPR1); // Fosc/128
	SPCR = (1<<SPE) | (1<<MSTR); // Fosc/2
	SPSR = (1<<SPI2X);

	// Timer1 is used to generate the GSCLK clock signal
	//
	// We want an overall PWM period of around 60Hz
	// There are 4096 PWM steps per period so we need to pulse
	// around 60Hz * 4096 steps = 245,760Hz, so we round up to
	// the next even value of 250,000 Hz
	TCCR1A = 0x63;	// 01100011 - Fast PWM - TOP = OCR1A
	TCCR1B = 0x11;	// 00010001 = Fosc (16,000,000) no pre-scale = 250,000 Hz
	OCR1A = 16; // Duty change at 16 counts
	ICR1 = 32; // Maximum count is 32

	// Timer2 should interrupt every 4096 pulses of the PWM1
	// PWM1 period is 250,000 Hz which is one pulse every 4 uS
	// We need to wait for 4096 pulses before calling the interrupt
	// so 4 uS * 4096 = 16384 uS

	// We need an interrupt every 16384 uS
	// Fosc is 16,000,000 meaning we have 16M ticks per second
	// With a /1024 pre-scaler we get 15,625 ticks per second
	// 15,625 / 1M (uS per second) = 0.015625 ticks per uS
	// 0.015625 * 16,384 = 256 ticks
	//
	// 16 bit counter so 65535 - 32,768 = 32,767
	TCCR2A = 0x00;	// Set timer2 to normal operation (mode0)
	TCCR2B = 0x07;	// 00000111 Set pre-scaler to /1024
	TIMSK2 = 0x01;	// Enable the timer2 overflow interrupt;
	TCNT2 = 0x00;	// Reset the 8 bit timer register
}

// Set the gray-scale value of a LED channel
//
// Note: The TLC5940 expects 12 bit values for each channel, however we store
// the values in an 8 bit array (since that is what we need for sending the 
// data over the SPI).  This function places our 12 bit value in the correct
// place.
void setGrayScaleValue(unsigned char channel, int grayScale)
{
	// Range check the grayscale data
	if (grayScale > 4095) grayScale = 4095;
	if (grayScale < 0) grayScale = 0;
	
	// Now we pack the 12 bit channel data into our 8 bit array
	unsigned char eightBitIndex = (NUMBEROF5940 * 16 - 1) - channel;
	unsigned char *twelveBitIndex = packedGrayScaleDataBuffer1 + ((eightBitIndex * 3) >> 1);
	
	if (eightBitIndex & 1)
	{
		// Value starts in the middle of the byte
		// Set only the top 4 bits
		*twelveBitIndex = (*twelveBitIndex & 0xF0) | (grayScale >> 8);
		
		// Now set the lower 4 bits of the next byte
		*(++twelveBitIndex) = grayScale & 0xFF;
	}
	else
	{
		// Value starts at the start of the byte
		*(twelveBitIndex++) = grayScale >> 4;
		
		// Now set the 4 lower bits of the next byte leaving the top 4 bits alone
		*twelveBitIndex = ((int)(grayScale << 4)) | (*twelveBitIndex & 0xF);
	}		
}		

// Update the TLC5940 send buffer
int updateTlc5940(void)
{
	// If an update is already pending, return with status 0;
	if (updatePending == 1) return 0;
	
	// Copy over our packed data buffer to the send data buffer
	// Note: We are using double-buffering to prevent a partial
	// update from occurring (since an XLAT interrupt could occur
	// whilst we are still updating the data)

	for (int byteCounter = 0; byteCounter < (24 * NUMBEROF5940); byteCounter++)
	packedGrayScaleDataBuffer2[byteCounter] = packedGrayScaleDataBuffer1[byteCounter];
	
	// Set the update pending flag
	updatePending = 1;
	
	// Update OK, return with status 0
	return 0;
}

// Timer2 interrupt procedure for XLAT processing
ISR(TIMER2_OVF_vect)
{	
		
	// Get ready for the next interrupt -----------------------------------
	TCNT2 = 0x00;	// Reset the 16 bit timer register
	
	// Process the XLAT interrupt --------------------------------------------------
	
	// Turn off the LEDs
	sbi(TLC5940_BLANK_PORT, TLC5940_BLANK_PIN);
	
	// Are we waiting for an XLAT pulse to latch new data?
	if (waitingForXLAT == 1)
	{
		// Pulse the XLAT signal
		sbi(TLC5940_XLAT_PORT, TLC5940_XLAT_PIN);
		cbi(TLC5940_XLAT_PORT, TLC5940_XLAT_PIN);
		
		// Clear the flag
		waitingForXLAT = 0;
	}
	
	// Turn on the LEDs
	cbi(TLC5940_BLANK_PORT, TLC5940_BLANK_PIN);
	
	// Process the automatic LED fading --------------------------------------------
	
#ifdef TLC_FADE_CONTROL

	unsigned char updateCheck = 0;

	// Process the LEDs
	for (int ledNumber = 0; ledNumber < 16 * NUMBEROF5940; ledNumber++)
	{
		if (led[ledNumber].targetBrightness != led[ledNumber].actualBrightness)
		{
			// Fade the colour up or down
			if (led[ledNumber].targetBrightness >= led[ledNumber].actualBrightness)
			{
				led[ledNumber].actualBrightness += fadeOnSpeed;
				
				// Range check
				if (led[ledNumber].actualBrightness > led[ledNumber].targetBrightness)
					led[ledNumber].actualBrightness = led[ledNumber].targetBrightness;
				
				updateCheck = 1;
			}				
			else
			{
				led[ledNumber].actualBrightness -= fadeOffSpeed;
				
				// Range check
				if (led[ledNumber].actualBrightness < led[ledNumber].targetBrightness)
					led[ledNumber].actualBrightness = led[ledNumber].targetBrightness;
				
				updateCheck = 1;
			}				
				
			// Range check the fade
			if (led[ledNumber].actualBrightness > 4095)
				led[ledNumber].actualBrightness = 4095;

							
			if (led[ledNumber].actualBrightness < 0)
				led[ledNumber].actualBrightness = 0;
			
			// Set the correct LED channels for the RGB LED
			setGrayScaleValue(ledNumber, led[ledNumber].actualBrightness);
		}
	
		// Update the TLC5940s
		if (updateCheck == 1)
		{
			updateTlc5940();
			updateCheck == 0;
		}			
	}		

#endif
	
	// Note: Once we have reset the 5940's PWM counter by toggling the BLANK pin we can
	// shift in the serial data (since the PWM pulse for GSCLK continues to run
	// in the background).  The shifting of the data must happen before the next
	// XLAT interrupt is due which means we have about 16,000 uS to do this.
	// As you add more and more TLC5940s this shifting will take longer and longer
	// until you simply can't shift the data in time, then it's time to buy another
	// AVR if you want to support more LED channels...
	
	// Do we have an update to the data pending?
	if (updatePending == 1)
	{
		// We have an update pending, write the serial information to the device
		for (int byteCounter = 0; byteCounter < (24 * NUMBEROF5940); byteCounter++)
		{
			// Start transmission
			SPDR = packedGrayScaleDataBuffer2[byteCounter];
			
			// Wait for transmission complete
			while (!(SPSR & (1 << SPIF)));
		}
		
		// Serial data is now updated, clear the flag
		updatePending = 0;
		
		// Set the waiting for XLAT flag to indicate there is data waiting
		// to be latched
		waitingForXLAT = 1;
	}
}

// The following functions are for the automatic fade control, see tlc5940.h for details
#ifdef TLC_FADE_CONTROL

	// Initialise the LED states
	void initialiseFadingLeds(void)
	{
		for (int ledNumber = 0; ledNumber < 16 * NUMBEROF5940; ledNumber++)
		{
			led[ledNumber].targetBrightness = 0;
			led[ledNumber].actualBrightness = 0;
		}
	
		fadeOnSpeed = 30;
		fadeOffSpeed = 100;
	}
	
	void setLedBrightness(int ledNumber, int brightness)
	{
		led[ledNumber].targetBrightness = brightness;
	}

	// Set the fade on and off speed for an LED
	// Note: The fade speed is the amount of fade in one PWM period i.e. if PWM period is 60Hz
	// 8 would be 'fade 8 units 60 times a second' - units are the brightness of 0-4095
	void setLedFadeSpeed(int fadeOn, int fadeOff)
	{
		fadeOnSpeed = fadeOn;
		fadeOffSpeed = fadeOff;
	}
	
#endif
