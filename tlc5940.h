/************************************************************************
	tlc5940.h

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

#ifndef TLC5940_H_
#define TLC5940_H_

// TLC5940 Hardware mapping definitions

// Note: SIN, SCLK and GSCLK are tied to module functions on the AVR
// and you can't change these without changing the appropriate library
// functions.
#define TLC5940_SIN_PORT	PORTB
#define TLC5940_SIN_PIN		3
#define TLC5940_SCLK_PORT	PORTB
#define TLC5940_SCLK_PIN	5
#define TLC5940_GSCLK_PORT	PORTB
#define TLC5940_GSCLK_PIN	1

// These you can assign to other pins if required
#define TLC5940_XLAT_PORT	PORTD
#define TLC5940_XLAT_PIN	6
#define TLC5940_VPRG_PORT	PORTD
#define TLC5940_VPRG_PIN	7
#define TLC5940_BLANK_PORT	PORTD
#define TLC5940_BLANK_PIN	5

// The number of cascaded TLC5940s
#define NUMBEROF5940	7

// If you don't want the built in LED fade on and off control comment out 
// the following line:
#define TLC_FADE_CONTROL

#ifdef TLC_FADE_CONTROL

	// Structures for storing LED fading information
	struct ledState
	{
		int targetBrightness;
		int actualBrightness;
	};

	struct ledState led[NUMBEROF5940 * 16];
	
	// Globals for the LED fading speeds
	int fadeOnSpeed;
	int fadeOffSpeed;

#endif

// Function prototypes
void setInitialDotCorrection(unsigned char *dotCorrectionValues);
void setInitialGrayScaleValues(void);
void initialiseTlc5940(void);
void setGrayScaleValue(unsigned char channel, int grayScale);
int updateTlc5940(void);

#ifdef TLC_FADE_CONTROL
	void initialiseFadingLeds(void);
	void setLedBrightness(int ledNumber, int brightness);
	void setLedFadeSpeed(int fadeOn, int fadeOff);
#endif

#endif /* TLC5940_H_ */